#pragma once
#include <stdint.h>
#include <vector>
#include <cstring>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi */
#endif // !M_PI

#define NUM_BESSELI0_ITERATIONS 32
#define NUM_LNGAMMA_ITERATIONS (256)
#define EULER_GAMMA            (0.57721566490153286)

namespace DigitalFilter
{
   enum FIRFilterType
   {
    LowPass,
    HighPass,
    BandPass,
    BandStop
   };
  
   enum FIRFilterWindowType
   {
    Rectangular,
    Triangular,
    Welch,
    Sine,
    Hann,
    Hamming,
    Blackman,
    Nuttall,
    BlackmanNuttall,
    BlackmanHarris,
    FlatTop,
    Kaiser
   };


   class FIRFilter {
  
    FIRFilterType  m_Filter_Type;
    FIRFilterWindowType m_Window_Type;
  
    uint16_t    m_Total_Samples;
    uint16_t    m_Shift_Samples;
    float    m_Cutoff_freq_fc1;
    float    m_Cutoff_freq_fc2;
    float    m_Fs;
    float    m_Sampling_Time;
  
    float    m_Beta;
  
    std::vector<float> m_vCoff;
  
    float*    m_pfDataBuffer;
    uint16_t    m_wBufferIndex;
    float    m_fOut;
  
   public:
  
    /// @brief FIRFilter Constructor
    /// @details Initializes an FIR filter with specified parameters, including filter type, window type, 
    /// order, sampling frequency, cutoff frequencies, and stopband attenuation. The constructor calculates 
    /// the filter coefficients based on the selected window and stores them for real-time filtering.
    /// @param filterType Type of FIR filter (Low-pass, High-pass, Band-pass, etc.).
    /// @param window Window function used for coefficient calculation (e.g., Hamming, Hanning, Kaiser).
    /// @param order Number of filter coefficients (higher order = sharper cutoff).
    /// @param fs Sampling frequency of the signal.
    /// @param fc1 Primary cutoff frequency.
    /// @param fc2 Secondary cutoff frequency (used in band-pass/band-stop filters, default = 0.0).
    /// @param as Stopband attenuation in dB (used for Kaiser window, default = 60.0).
    FIRFilter(FIRFilterType filterType, FIRFilterWindowType window, uint16_t order , float fs, float fc1, float fc2 = 0.0f, float as = 60.0f):
     m_Filter_Type(filterType), m_Window_Type(window), m_Fs(fs), m_Total_Samples(order), 
     m_Cutoff_freq_fc1(fc1), m_Cutoff_freq_fc2(fc2),
     m_vCoff(order, 0.0f)
    {
     m_Sampling_Time = 1.0f / fs;
     m_Shift_Samples = order / 2;
     m_pfDataBuffer = new float[order];
  
     memset(m_pfDataBuffer, 0, sizeof(float) * order);
     m_wBufferIndex = 0;
     m_fOut = 0.0f;
  
     if(window == FIRFilterWindowType::Kaiser){
      m_Beta = kaiser_beta_As(as);
     }
  
     cofficientsCal();
    }
  
    ~FIRFilter(){
     delete[] m_pfDataBuffer;
    }
  
    /// @brief Applies the FIR filter to the input signal.
    /// @details This function processes an input sample through the FIR filter by maintaining a circular buffer, 
    /// applying convolution with precomputed filter coefficients, and returning the filtered output.
    /// @param in The new input sample to be filtered.
    /// @return The filtered output sample.
    float Filter(float in) {
     m_pfDataBuffer[m_wBufferIndex] = in;
     
     m_wBufferIndex++;
  
     if (m_wBufferIndex == m_Total_Samples) {
      m_wBufferIndex = 0;
     }
     m_fOut = 0.0f;
  
     uint16_t sumIndex = m_wBufferIndex;
     for (std::size_t n = 0; n < m_Total_Samples; n++) {
      sumIndex = (sumIndex == 0) ? m_Total_Samples - 1 : sumIndex - 1;
      m_fOut += m_vCoff[n] * m_pfDataBuffer[sumIndex];
     }
     return m_fOut;
    }
  
    const std::vector<float>& GetImpulseResponse(){
     return m_vCoff;
    }
  
    void process (juce::dsp::ProcessContextReplacing<float>& context)
    {
     auto in = context.getInputBlock();
     auto out = context.getOutputBlock();
  
        jassert (in.getNumChannels () == 1);
        jassert (out.getNumChannels () == 1);

     for (int i = 0; i < in.getNumSamples(); i++)
      out.setSample(0, i, Filter(in.getSample(0, i)));
    
    }


   private:
    void cofficientsCal() {
     
     for (int n = 0; n < m_Total_Samples; n++) {
      if (n != m_Shift_Samples) {
       switch (m_Filter_Type)
       {
       case DigitalFilter::LowPass:
        m_vCoff[n] = static_cast<float>(sin(2.0 * M_PI * m_Cutoff_freq_fc1 * m_Sampling_Time * (n - m_Shift_Samples)) / (M_PI * m_Sampling_Time * (n - m_Shift_Samples)));
        break;
       case DigitalFilter::HighPass:
        m_vCoff[n] = static_cast<float>(sin(M_PI * (n - m_Shift_Samples)) - sin(2.0 * M_PI * m_Cutoff_freq_fc1 * m_Sampling_Time * (n - m_Shift_Samples))) / (M_PI * m_Sampling_Time * (n - m_Shift_Samples));
        break;
       case DigitalFilter::BandPass:
        m_vCoff[n] = static_cast<float>(sin(2.0 * M_PI * m_Cutoff_freq_fc2 * m_Sampling_Time * (n - m_Shift_Samples)) - sin(2.0 * M_PI * m_Cutoff_freq_fc1 * m_Sampling_Time * (n - m_Shift_Samples))) / (M_PI * m_Sampling_Time * (n - m_Shift_Samples));
        break;
       case DigitalFilter::BandStop:
        m_vCoff[n] = static_cast<float>(sin(2.0 * M_PI * m_Cutoff_freq_fc1 * m_Sampling_Time * (n - m_Shift_Samples)) - sin(2.0 * M_PI * m_Cutoff_freq_fc2 * m_Sampling_Time * (n - m_Shift_Samples)) + sin(M_PI * (n - m_Shift_Samples))) / (M_PI * m_Sampling_Time * (n - m_Shift_Samples));
        break;
       default:
        break;
       }
      }
      else {
       switch (m_Filter_Type)
       {
       case DigitalFilter::LowPass:
        m_vCoff[n] = 2.0f * m_Cutoff_freq_fc1;
        break;
       case DigitalFilter::HighPass:
        m_vCoff[n] = static_cast<float>(1.0 / m_Sampling_Time - 2.0 * m_Cutoff_freq_fc1);
        break;
       case DigitalFilter::BandPass:
        m_vCoff[n] = 2.0 * (m_Cutoff_freq_fc2 -  m_Cutoff_freq_fc1);
        break;
       case DigitalFilter::BandStop:
        m_vCoff[n] = 2.0 * m_Cutoff_freq_fc1 - 2.0 * m_Cutoff_freq_fc2 + static_cast<float>(1.0 / m_Sampling_Time);
        break;
       default:
        break;
       }
      }
     }
  
     for (int n = 0; n < m_Total_Samples; n++) {
      switch (m_Window_Type)
      {
      case DigitalFilter::Rectangular:
       //m_vWindow[n] = 1.0f;
       m_vCoff[n] *= (m_Sampling_Time * 1.0f);
       break;
      case DigitalFilter::Triangular:
       //m_vWindow[n] = 1.0 - abs((n - 0.5 * m_Total_Samples) / (0.5 * m_Total_Samples));
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (1.0f - abs((n - 0.5f * m_Total_Samples) / (0.5f * m_Total_Samples)))) ;
       break;
      case DigitalFilter::Welch:
       //m_vWindow[n] = 1.0 - pow((n - 0.5 * m_Total_Samples) / (0.5 * m_Total_Samples), 2.0);
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (1.0f - pow((n - 0.5f * m_Total_Samples) / (0.5f * m_Total_Samples), 2.0f)));
       break;
      case DigitalFilter::Sine:
       //m_vWindow[n] = sin(M_PI * n / ((float)m_Total_Samples));
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * sin(M_PI * n / ((float)m_Total_Samples)));
       break;
      case DigitalFilter::Hann:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * 0.5f * (1.0f - cos(2.0 * M_PI * n / ((double)m_Total_Samples))));
       break;
      case DigitalFilter::Hamming:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * ((25.0f / 46.0f) - (21.0f / 46.0f) * cos(2.0f * M_PI * n / ((double)m_Total_Samples))));
       break;
      case DigitalFilter::Blackman:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (0.42f - 0.5f * cos(2.0f * M_PI * n / ((double)m_Total_Samples)) + 0.08f * cos(4.0f * M_PI * n / ((double)m_Total_Samples))));
       break;
      case DigitalFilter::Nuttall:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (0.355768f - 0.487396f * cos(2.0 * M_PI * n / ((float)m_Total_Samples)) + 0.144232f * cos(4.0f * M_PI * n / ((double)m_Total_Samples)) - 0.012604f * cos(6.0f * M_PI * n / ((double)m_Total_Samples))));
       break;
      case DigitalFilter::BlackmanNuttall:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (0.3635819f - 0.4891775 * cos(2.0 * M_PI * n / ((float)m_Total_Samples)) + 0.1365995f * cos(4.0f * M_PI * n / ((float)m_Total_Samples)) - 0.0106411f * cos(6.0f * M_PI * n / ((float)m_Total_Samples))));
       break;
      case DigitalFilter::BlackmanHarris:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (0.35875f - 0.48829f * cos(2.0f * M_PI * n / ((float)m_Total_Samples)) + 0.14128f * cos(4.0f * M_PI * n / ((float)m_Total_Samples)) - 0.01168f * cos(6.0f * M_PI * n / ((float)m_Total_Samples))));
       break;
      case DigitalFilter::FlatTop:
       m_vCoff[n] *= static_cast<float>(m_Sampling_Time * (0.21557895f - 0.41663158f * cos(2.0f * M_PI * n / ((float)m_Total_Samples)) + 0.277263158f * cos(4.0f * M_PI * n / ((float)m_Total_Samples)) - 0.083578947f * cos(6.0f * M_PI * n / ((float)m_Total_Samples)) + 0.006947368f * cos(8.0 * M_PI * n / ((float)m_Total_Samples))));
       break;
      case DigitalFilter::Kaiser:
       m_vCoff[n]  *= m_Sampling_Time * kaiser(n, m_Total_Samples, m_Beta);
       break;
      default:
       m_vCoff[n] *= m_Sampling_Time * 1.0f;
       break;
      }
     }
     
    }
  
    // Kaiser window 
    //  _n      :   sample index
    //  _N      :   window length (samples)
    //  _beta   :   window taper parameter
    float kaiser(uint16_t int_n, uint16_t iN, float beta)
    {
     // validate input
     
     if (int_n > iN) {
     //fprintf(stderr,"error: kaiser(), sample index must not exceed window length\n");
     // exit(1);
     } else if (beta < 0) {
       printf(stderr,"error: kaiser(), beta must be greater than or equal to zero\n");
       exit(1);
     }

     float t = (float)int_n - (float)(iN - 1) / 2;
     float r = 2.0f*t / (float)(iN);
     // float r = 2.0f*(float)_i/(float)_N;
     float a = besseli0f(beta*sqrtf(1 - r*r));
     float b = besseli0f(beta);
     return (a / b);
    }
  
    float kaiser_beta_As(float A) {
  
     A = fabsf(A);
     float beta2;
     if (A > 50.0f) {
      beta2 = 0.1102f*(A - 8.7f);
     }
     else if (A > 21.0f)
     {
      beta2 = 0.5842*powf(A - 21, 0.4f) + 0.07886f*(A - 21);
     }
     else {
      beta2 = 0.0f;
     }
     return beta2;
    }
     
    float lngammaf(float z)
    {
     float g;
     if (z < 0) {
      fprintf(stderr,"error: lngammaf(), undefined for z <= 0\n");
      exit(1);
     }
     else if (z < 10.0f) {
      return lngammaf(z + 1.0f) - logf(z);
     }
     else {
      // high value approximation
      g = 0.5*(logf(2 * M_PI) - log(z));
      g += z*(logf(z + (1 / (12.0f*z - 0.1f / z))) - 1);
     }
  
     return g;
    }
  
    float besseli0f(float z) {
     // use better low-signal approximation
     if (z == 0.0f) {
      return 1.0f;
     }
     unsigned int k;
     float t, y = 0.0f;
     for (k = 0; k < NUM_BESSELI0_ITERATIONS; k++) {
  
      t = k * logf(0.5f*z) - lngammaf((float)k + 1.0f);
      y += expf(2 * t);
     }
     return y;
    }
  
  };
}