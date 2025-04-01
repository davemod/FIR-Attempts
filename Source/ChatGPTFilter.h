#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

class FIRLinearPhaseFilter
{
public:
    FIRLinearPhaseFilter() = default;

    std::vector<std::vector<float>> ringBuffer;
    int ringBufferSize = 0;
    int ringBufferPos = 0;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        blockSize = spec.maximumBlockSize;

        int numChannels = static_cast<int>(spec.numChannels);
        ringBufferSize = 8192; // größer als max. Taps + Blockgröße
        ringBufferPos = 0;

        ringBuffer.resize(numChannels);
        for (auto& buf : ringBuffer)
            buf.assign(ringBufferSize, 0.0f);

        reset();
    }

    void reset()
    {
        buffer.clear();
        buffer.resize(coeffs.size() + blockSize, 0.0f);
        bufferPos = 0;
    }

    void setLowpass(float cutoffHz, int numTaps)
    {
        coeffs = designLowpassFIR(cutoffHz, numTaps);
        reset();
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        auto numChannels = inputBlock.getNumChannels();
        auto numSamples = inputBlock.getNumSamples();
    
        const int filterLength = static_cast<int>(coeffs.size());
        const int halfLength = filterLength / 2;
    
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* in = inputBlock.getChannelPointer(ch);
            auto* out = outputBlock.getChannelPointer(ch);
    
            for (int n = 0; n < static_cast<int>(numSamples); ++n)
            {
                // Ringpuffer schreiben
                ringBuffer[ch][ringBufferPos] = in[n];
    
                // Faltung mit symmetrischer Impulsantwort
                float acc = 0.0f;
                for (int k = 0; k < filterLength; ++k)
                {
                    int index = (ringBufferPos - k + ringBufferSize) % ringBufferSize;
                    acc += coeffs[k] * ringBuffer[ch][index];
                }
    
                // Output wird verzögert um M/2 Samples (linear phase)
                int outIndex = (ringBufferPos - halfLength + ringBufferSize) % ringBufferSize;
                out[n] = acc;
    
                ringBufferPos = (ringBufferPos + 1) % ringBufferSize;
            }
        }
    }
    
private:
    std::vector<float> coeffs;
    std::vector<float> buffer;
    size_t bufferPos = 0;
    double sampleRate = 44100.0;
    size_t blockSize = 512;

    std::vector<float> designLowpassFIR(float cutoffHz, int numTaps)
    {
        std::vector<float> h(numTaps);
        int M = numTaps - 1;
        float fc = cutoffHz / static_cast<float>(sampleRate); // normiert

        for (int n = 0; n <= M; ++n)
        {
            float sinc;
            if (n == M / 2)
                sinc = 2.0f * fc;
            else
            {
                float x = float(M / 2 - n) * float_Pi;
                sinc = sin(2.0f * float_Pi * fc * (n - M / 2)) / x;
            }

            float window = 0.54f - 0.46f * cos(2.0f * float_Pi * n / M);
            h[n] = sinc * window;
        }

        return h;
    }
};
