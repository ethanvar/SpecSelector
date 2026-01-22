#pragma once

#include <vector>
#include <array>
#include <complex>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_utils/juce_audio_utils.h>

struct StftSegment
{
    static constexpr int fftSize = 1 << 13;
    std::array<float, fftSize * 2> frequencyData;
    int StartIndex;
};


class Stft {
    private:
        juce::dsp::FFT m_forwardFFT;
        int m_hopSize;
        int m_sampleRate;
        int m_currentSampleNum = 0;
        std::vector<StftSegment> m_stft;
        
    public:
        static constexpr int fftSize = 1 << 13;
        Stft(const int& sampleRate, const int& hopSize);
        ~Stft();
        void addLineToStft(std::array<float, StftSegment::fftSize * 2> &fourierData);
        void printStftValues();
        void drawSpectorgramWithStft(juce::Image &specImage);
};
