#include "Stft.h"
#include <iostream>

Stft::Stft(const int& sampleRate, const int& hopSize) : m_hopSize(hopSize), m_sampleRate(sampleRate), m_forwardFFT(13) {

};

Stft::~Stft () {

};

void Stft::addLineToStft(std::array<float, fftSize * 2> &fourierData) {
    StftSegment segment;
    juce::dsp::WindowingFunction<float> window(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
    window.multiplyWithWindowingTable(fourierData.data(),fftSize);
    m_forwardFFT.performFrequencyOnlyForwardTransform (fourierData.data());
    segment.frequencyData = fourierData;
    m_currentSampleNum++;
    segment.StartIndex = m_currentSampleNum;
    std::cout << m_currentSampleNum << std::endl;
    m_stft.push_back(segment);
}

void Stft::printStftValues() {
    std::cout << "STFT contains " << m_stft.size() << " segments" << std::endl;
    for (size_t i = 0; i < m_stft.size(); ++i) {
        std::cout << "Segment " << i << " (StartIndex: " << m_stft[i].StartIndex << "):" << std::endl;
        std::cout << "  Frequency data preview (first 10 values): ";
        for (size_t j = 0; j < 10 && j < fftSize * 2; ++j) {
            std::cout << m_stft[i].frequencyData[j] << " ";
        }
        std::cout << "..." << std::endl;
    }
}

void Stft::drawSpectorgramWithStft(juce::Image &specImage) {
    auto w = specImage.getWidth();
    int sliceWidth = juce::jlimit(1, w, sliceWidth);

    auto imageHeight = specImage.getHeight();
    
    for (size_t i=0; i < m_stft.size(); ++i) {
        specImage.moveImageSection(0, 0, sliceWidth, 0, w - sliceWidth, imageHeight);
        m_forwardFFT.performRealOnlyInverseTransform(m_stft[i].frequencyData.data());



    }
}


// void AudioPluginAudioProcessor::drawNextLineOfSpectrogram(juce::Image &specImage, 
//                                                            std::array<float, fftSize * 2> &fourierData, int sliceWidth)  {
//     //Add spectogram,  parameter 
//     auto w = specImage.getWidth();
//     sliceWidth = juce::jlimit(1, w, sliceWidth);
//     int drawX = w - sliceWidth;
//     auto imageHeight = specImage.getHeight();
//     specImage.moveImageSection(0, 0, sliceWidth, 0, w - sliceWidth, imageHeight);

//     juce::dsp::WindowingFunction<float> window(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
//     window.multiplyWithWindowingTable(fourierData.data(), fftSize);
//     forwardFFT.performFrequencyOnlyForwardTransform (fourierData.data());
//     auto maxLevel = juce::FloatVectorOperations::findMinAndMax (fourierData.data(), fftSize / 2);

//     juce::Image::BitmapData bitmap {
//         specImage,
//         drawX,
//         0,
//         sliceWidth,
//         imageHeight,
//         juce::Image::BitmapData::writeOnly
//     };

//     if (sliceWidth == 2) {
//         stft->addLineToStft(fourierData);
//     }

//     for (auto y = 1; y < imageHeight; ++y) {
//         auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.6f);
//         auto fftDataIndex = (size_t) juce::jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
//         auto level = juce::jmap (fourierData[fftDataIndex], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
//         if (sliceWidth > 1) {
//             for (int x = 0; x < sliceWidth; ++x)
//                 bitmap.setPixelColour(x, y, mapFFTValueToColour(level));
//         } else {
//             bitmap.setPixelColour(0, y, mapFFTValueToColour(level));
//         }
//     }
// }