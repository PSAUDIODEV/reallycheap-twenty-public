#include "MacroController.h"
#include "Params.h"

namespace ReallyCheap
{

void MacroController::prepare(double sampleRate, int samplesPerBlock) noexcept
{
    juce::ignoreUnused(samplesPerBlock);
    sampleRate_ = sampleRate;
    
    // Setup moderate smoothing for macro (20ms time constant - faster for automation)
    const double smoothingTimeMs = 20.0;
    macroSmoothingCoeff_ = static_cast<float>(std::exp(-1.0 / (smoothingTimeMs * 0.001 * sampleRate)));
    
    reset();
}

void MacroController::reset() noexcept
{
    smoothedMacro_ = ParameterDefaults::macroReallyCheap;
    updateScalingFactors();
}

void MacroController::tick(const juce::AudioProcessorValueTreeState& apvts) noexcept
{
    // Get current macro value and apply smoothing
    auto macroParam = apvts.getRawParameterValue(ParameterIDs::macroReallyCheap);
    if (macroParam != nullptr)
    {
        const float targetMacro = *macroParam;
        smoothedMacro_ = smoothedMacro_ * macroSmoothingCoeff_ + targetMacro * (1.0f - macroSmoothingCoeff_);
        updateScalingFactors();
        
        // Debug output
        static int debugCount = 0;
        if (debugCount < 10 || (debugCount % 1000 == 0))
        {
            DBG("Macro - raw: " << targetMacro << ", smoothed: " << smoothedMacro_ << ", wobbleGain: " << wobbleDepthGain());
            debugCount++;
        }
    }
}

void MacroController::updateScalingFactors() noexcept
{
    const float m = saturate(smoothedMacro_);
    
    // Zone-based behavior:
    // Zone A (0.0–0.3): subtle sweetening
    // Zone B (0.3–0.7): tasteful degradation + light space  
    // Zone C (0.7–1.0): the "nasty" zone (still bounded)
    
    //========================================================================
    // WOBBLE (Primary response - most sensitive)
    //========================================================================
    
    // Depth gain: subtle at first, then ramps up significantly
    // gw = lerp(1.0, 2.0, ease2(saturate((m-0.15)/0.85)))
    wobbleDepthGain_ = lerp(1.0f, 2.0f, ease2(saturate((m - 0.15f) / 0.85f)));
    
    // Flutter gain: starts later, builds to 2.5x
    // gf = lerp(1.0, 2.5, ease(saturate((m-0.35)/0.65)))
    wobbleFlutterGain_ = lerp(1.0f, 2.5f, ease(saturate((m - 0.35f) / 0.65f)));
    
    //========================================================================
    // MAGNETIC (Primary response - tape character)
    //========================================================================
    
    // Compression gain: starts at 25% macro, builds to 2x
    // gc = lerp(1.0, 2.0, ease(saturate((m-0.25)/0.75)))
    magneticCompGain_ = lerp(1.0f, 2.0f, ease(saturate((m - 0.25f) / 0.75f)));
    
    // Saturation gain: similar timing, more controlled (1.8x max)
    // gs = lerp(1.0, 1.8, ease2(saturate((m-0.25)/0.75)))
    magneticSatGain_ = lerp(1.0f, 1.8f, ease2(saturate((m - 0.25f) / 0.75f)));
    
    //========================================================================
    // DISTORT (Secondary response - kicks in later)
    //========================================================================
    
    // Extra drive: 0 dB until 40% macro, then ramps to +12 dB max
    // ddB = 12.0 * ease(saturate((m-0.4)/0.6))
    if (m <= 0.4f)
    {
        distortDriveAddDb_ = 0.0f;
    }
    else
    {
        distortDriveAddDb_ = 12.0f * ease(saturate((m - 0.4f) / 0.6f));
    }
    
    //========================================================================
    // DIGITAL (Secondary response - degradation in upper range)
    //========================================================================
    
    // Bits floor: starts reducing at 60% macro, min 6 bits (guardrail)
    // floorBits = 16 - 10 * ease(saturate((m-0.6)/0.4)) then clamp ≥ 6
    if (m <= 0.6f)
    {
        digitalBitsFloor_ = 16.0f;
    }
    else
    {
        float reduction = 10.0f * ease(saturate((m - 0.6f) / 0.4f));
        digitalBitsFloor_ = juce::jmax(6.0f, 16.0f - reduction);
    }
    
    // Sample rate floor: starts at 50% macro, min 8000 Hz (guardrail)
    // floorSRHz = max(8000, 44100 - 28100 * ease(saturate((m-0.5)/0.5)))
    if (m <= 0.5f)
    {
        digitalSRFloorHz_ = 44100.0f;
    }
    else
    {
        float reduction = 28100.0f * ease(saturate((m - 0.5f) / 0.5f));
        digitalSRFloorHz_ = juce::jmax(8000.0f, 44100.0f - reduction);
    }
    
    //========================================================================
    // SPACE (Secondary response - controlled reverb)
    //========================================================================
    
    // Mix cap: subtle reverb build, never exceeds 0.25 (guardrail)
    // cap = lerp(0.10, 0.25, ease(saturate((m-0.35)/0.65)))
    if (m <= 0.35f)
    {
        spaceMixCap_ = 0.10f;
    }
    else
    {
        spaceMixCap_ = lerp(0.10f, 0.25f, ease(saturate((m - 0.35f) / 0.65f)));
    }
    
    //========================================================================
    // NOISE (Secondary response - texture in upper range)
    //========================================================================
    
    // Level add: +0 dB at m=0 → +6 dB at m=1
    noiseLevelAddDb_ = 6.0f * ease(m);
    
    // Age gain: starts at 50%, makes noise older/darker
    // ga = lerp(1.0, 1.3, ease(saturate((m-0.5)/0.5)))
    if (m <= 0.5f)
    {
        noiseAgeGain_ = 1.0f;
    }
    else
    {
        noiseAgeGain_ = lerp(1.0f, 1.3f, ease(saturate((m - 0.5f) / 0.5f)));
    }
    
    // Debug output of key scaling factors
    static int scalingDebugCount = 0;
    if (scalingDebugCount < 20 || (scalingDebugCount % 500 == 0))
    {
        DBG("MacroController scalars - m: " << m << ", wobbleDepth: " << wobbleDepthGain_ << ", magneticComp: " << magneticCompGain_ << ", distortAdd: " << distortDriveAddDb_);
        scalingDebugCount++;
    }
}

//============================================================================
// Musical easing functions
//============================================================================

float MacroController::ease(float x) noexcept
{
    // Smoothstep: x*x*(3 - 2*x)
    x = saturate(x);
    return x * x * (3.0f - 2.0f * x);
}

float MacroController::ease2(float x) noexcept
{
    // Double smoothstep for stronger easing
    return ease(ease(x));
}

float MacroController::saturate(float x) noexcept
{
    return juce::jlimit(0.0f, 1.0f, x);
}

float MacroController::lerp(float a, float b, float t) noexcept
{
    return a + (b - a) * saturate(t);
}

}