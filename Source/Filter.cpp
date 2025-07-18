#include "Filter.h"

namespace IDs
{
    static inline String FunctionId{ "Function" };
    static inline String OrderId{ "Order" };
    static inline String FrequencyId{ "Frequency" };
    static inline String WindowTypeId{ "WindowType" };
    static inline String TransitionWidthId{ "TransitionWidth" };
    static inline String StopBandWeightId{ "StopBandWeight" };
    static inline String AmplitudeId{ "Amplitude" };
    static inline String SplineId{ "Spline" };
}

StringArray createFunctionChoices ()
{
    return {
        "LowpassWindowMethod",
        "LowpassKaiserMethod",
        "LowpassTransitionMethod",
        "LowpassLeastSquaresMethod",
        "LowpassHalfBandEquirippleMethod",
        "Custom"
    };
};

StringArray createWindowTypeChoices ()
{
    return {
        "rectangular",
        "triangular",
        "hann",
        "hamming",
        "blackman",
        "blackmanHarris",
        "flatTop",
        "kaiser"
    };
};

FirFilter::FirFilter(AudioProcessor &p)
    : processor(p)
{
    processor.addListener (this);

    parameters.set (IDs::FunctionId, new AudioParameterChoice({IDs::FunctionId, 1}, IDs::FunctionId, createFunctionChoices(), createFunctionChoices().indexOf("LowpassWindowMethod")));
    parameters.set (IDs::OrderId, new AudioParameterInt({IDs::OrderId, 1}, IDs::OrderId, 1, 5000, 21));
    parameters.set (IDs::FrequencyId, new AudioParameterFloat({IDs::FrequencyId, 1}, IDs::FrequencyId, { 20.f, 96000.f, 0.01f, 1.5f }, 1000.f));
    parameters.set (IDs::WindowTypeId, new AudioParameterChoice({IDs::WindowTypeId, 1}, IDs::WindowTypeId, createWindowTypeChoices(), createWindowTypeChoices().indexOf("hamming")));
    parameters.set (IDs::StopBandWeightId, new AudioParameterFloat({IDs::StopBandWeightId, 1}, IDs::StopBandWeightId, 1.f, 100.f, 1.f));
    parameters.set (IDs::TransitionWidthId, new AudioParameterFloat({IDs::TransitionWidthId, 1}, IDs::TransitionWidthId, 0.0001f, 0.5f, 0.5f));
    parameters.set (IDs::AmplitudeId, new AudioParameterFloat({IDs::AmplitudeId, 1}, IDs::AmplitudeId, -100.f, 0.f, -100.f));
    parameters.set (IDs::SplineId, new AudioParameterFloat({IDs::SplineId, 1}, IDs::SplineId, 1.f, 4.f, 1.f));

    for (auto param : parameters)
        processor.addParameter (param);
}

FirFilter::~FirFilter()
{
    processor.removeListener (this);
}

void FirFilter::prepare(const Spec &spec)
{
    specs = spec;
    filter.prepare (specs);
    delayLine.prepare (specs);
    updateFilter ();
}

void FirFilter::process(Context context)
{
    Coefficients::Ptr coeff;

    if (SpinLock::ScopedTryLockType sl{ swapLock }; sl.isLocked ())
        std::swap (coeff, newCoefficients);

    if (coeff)
    {
        auto state = filter.state;

        // auto needsReset = coeff->coefficients.size () != state->coefficients.size ();
        *state = *coeff;
        filter.reset ();
    }
    
    filter.process (context);
    // delayLine.process (context);
}

void FirFilter::audioProcessorParameterChanged(AudioProcessor *, int, float)
{
    updateFilter ();
}

void FirFilter::updateFilter()
{
    const auto sr = specs.sampleRate;
    
    const auto nyquist = sr / 2.0;
    const auto freq = jlimit (0.0f, (float)nyquist, getDenormalisedValue<float> (IDs::FrequencyId, 1000.f));
    const auto order = getDenormalisedValue<int> (IDs::OrderId, 21);
    const auto transitionWidth = getDenormalisedValue<float> (IDs::TransitionWidthId, 0.f);
    const auto amplitude = getDenormalisedValue<float> (IDs::AmplitudeId, -100.f);
    const auto spline = getDenormalisedValue<float> (IDs::SplineId, 0.f);
    const auto type = static_cast<dsp::WindowingFunction<float>::WindowingMethod> (getDenormalisedValue<int> (IDs::WindowTypeId, 0));
    const auto function = getDenormalisedValue<int> (IDs::FunctionId, 0);
    const auto stopBandWeight = getDenormalisedValue<float> (IDs::StopBandWeightId, 1.f);

    SpinLock::ScopedLockType lock (swapLock);

    switch (function)
    {
        case 0:
            newCoefficients = FilterDesign::designFIRLowpassWindowMethod (freq, sr, order, type);
            break;
        case 1:
            newCoefficients = FilterDesign::designFIRLowpassKaiserMethod (freq, sr, transitionWidth, amplitude);
            break;
        case 2:
            newCoefficients = FilterDesign::designFIRLowpassTransitionMethod (freq, sr, order, transitionWidth, spline);
            break;
        case 3:
            newCoefficients = FilterDesign::designFIRLowpassLeastSquaresMethod (freq, sr, order, transitionWidth, stopBandWeight);
            break;
        case 4:
            newCoefficients = FilterDesign::designFIRLowpassHalfBandEquirippleMethod (jlimit (0.f, 0.5f, transitionWidth), jlimit (-300.f, -10.f, amplitude));
            break;
        case 5:
        {

            Array<float> coeff{ 0.3f, 0.2f, 0.1f, 0.2f, 0.1f, 0.2f, 0.3f };
            newCoefficients = new Coefficients (coeff.getRawDataPointer (), coeff.size () );
            delayLine.setDelay (coeff.size () / 2);

            break;
        }
        default:
            jassertfalse; // Invalid function
            break;
    }

    auto isSymmetric = [&](){
        if (! newCoefficients)
            return false;

        auto& coeffs = newCoefficients->coefficients;
        
        if (coeffs.size() % 2 == 0)
            return false; // Even length coefficients cannot be symmetric

        for (size_t i = 0; i < coeffs.size() / 2; ++i)
        {
            if (coeffs[i] != coeffs[coeffs.size() - 1 - i])
                return false;
        }
        
        return true;
    };

    auto isAntiSymmetric = [&](){
        if (! newCoefficients)
            return false;

        auto& coeffs = newCoefficients->coefficients;
        
        if (coeffs.size() % 2 == 0)
            return false; // Even length coefficients cannot be anti-symmetric

        for (size_t i = 0; i < coeffs.size() / 2; ++i)
        {
            if (coeffs[i] != -coeffs[coeffs.size() - 1 - i])
                return false;
        }
        
        return true;
    };
    
    DBG ("Symmetric: " << (int)isSymmetric());
    DBG ("Anti-Symmetric: " << (int)isAntiSymmetric());
}
