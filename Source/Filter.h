#pragma once

#include <JuceHeader.h>

class FirFilter : AudioProcessorListener
{
public:
    FirFilter (AudioProcessor& p);
    ~FirFilter ();

    using SampleType = float;
    using Context = dsp::ProcessContextReplacing<SampleType>;
    using Block = dsp::AudioBlock<SampleType>;
    using Filter = dsp::FIR::Filter<SampleType>;
    using Coefficients = dsp::FIR::Coefficients<SampleType>;
    using Spec = dsp::ProcessSpec;
    using FilterDesign = dsp::FilterDesign<SampleType>;

    void prepare (const Spec& spec);
    void process (Context context);
    void audioProcessorParameterChanged (AudioProcessor*, int, float) override;
    void audioProcessorChanged (AudioProcessor* processor, const ChangeDetails& details) override { /* unused */ };

private:
    AudioProcessor& processor;
    HashMap<String, RangedAudioParameter*> parameters;
    
    dsp::ProcessorDuplicator<Filter, Coefficients> filter;
    dsp::DelayLine<SampleType> delayLine{ 8192 };

    dsp::ProcessSpec specs;
    
    SpinLock swapLock;
    
    ThreadPool threadPool;

    Coefficients::Ptr newCoefficients;
    Coefficients::Ptr oldCoefficients;

    Atomic<bool> needsUpdate { false };

    template<typename T>
    T getDenormalisedValue (const String& id, T defaultValue = T())
    {
        if (auto* param = parameters[id])
            return static_cast<T> (param->convertFrom0to1 (param->getValue()));
        else
            return defaultValue;
    }

    void updateFilter ();
};