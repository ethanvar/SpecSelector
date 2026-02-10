#pragma once

#include <vector>
#include <array>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>

struct StftSegment
{
    static constexpr int fftSize = 1 << 9;
    std::array<float, fftSize * 2> frequencyData;
    int StartIndex;
};


class Stft {
    private:
        static constexpr auto m_fftOrder = 9;
        juce::dsp::FFT m_forwardFFT;
        static constexpr auto m_fftSize = 1 << m_fftOrder;
        std::array<float, m_fftSize> m_fifo;
        std::array<float, m_fftSize * 2> m_fftData;
        int m_fifoIndex = 0;

        // int m_hopSize;
        // int m_sampleRate;
        int m_currentSampleNum = 0;
        std::vector<StftSegment> m_stft;
        
    public:
        static constexpr auto fftOrder = 9;
        static constexpr auto fftSize = 1 << fftOrder;

        Stft();
        ~Stft();

        void processEntireFile(juce::AudioFormatReader *reader);

        void addLineToStft(std::array<float, StftSegment::fftSize * 2> &fourierData);
        void printStftValues();
        void drawSpectorgramWithStft(juce::Image &specImage);
        bool hasData() const;
};
