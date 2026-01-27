#include "Stft.h"

#include <iostream>

Stft::Stft() : m_forwardFFT(13) {

};

Stft::~Stft () {

};


void Stft::processEntireFile(juce::AudioFormatReader* reader) {
    // use tranposrtSource to read ahead of live processBlock and process spectogram in advance
    // std::cout << "total sample length: " << reader->lengthInSamples << std::endl;
    std::cout << "Format created, again: " << reader->getFormatName() << std::endl;

    juce::AudioBuffer<float> fullBuffer((int)reader->numChannels, (int)reader->lengthInSamples);
    // juce::AudioSourceChannelInfo bufferToFill = juce::AudioSourceChannelInfo(fullBuffer);

    std::array<float, fftSize> firstOut;
    std::array<float, fftSize * 2> fourierData;

    if (!reader->read(&fullBuffer, 0,(int)reader->lengthInSamples, 0, true, true)) {
        std::cout << "Failed to read audio file" << std::endl;
        return;
    } else {
        std::cout << "Successfully read audio file" << std::endl;
    }

    int index = 0;

    std::cout << "Is it segfaulting here?" << std::endl;
    std::cout << "Checking buffer: " << fullBuffer.getSample(0, 500) << std::endl;
    std::cout << "First out size: " << firstOut.size() << std::endl;


    for (int i = 0; i <= fullBuffer.getNumSamples(); i++) {
        firstOut[(size_t) index++] = fullBuffer.getSample(0, i);
    
        if ((i%fftSize) == 0) {
            std::fill (fourierData.begin(), fourierData.end(), 0.0f);
            std::copy (firstOut.begin(), firstOut.end(), fourierData.begin());
            // this->addLineToStft(fourierData);
            // drawNextLineOfSpectrogram(instantSpectogram, fourierData, 1);
            index = 0;
        }
    }
}


// void Stft::pushNextSampleIntoFifo (float sample) noexcept
// {
//     // if the fifo contains enough data, set a flag to say
//     // that the next line should now be rendered..
//     if (m_fifoIndex == m_fftSize) // [8]
//     {
//         if (!nextFFTBlockReady) // [9]
//         {
//             std::fill (m_fftData.begin(), m_fftData.end(), 0.0f);
//             std::copy (m_fifo.begin(), m_fifo.end(), m_fftData.begin());
//             nextFFTBlockReady = true;
//         }
//         m_fifoIndex = 0;
//     }
//     m_fifo[(size_t) m_fifoIndex++] = sample; // [9]
// }



void Stft::addLineToStft(std::array<float, fftSize * 2> &fourierData) {
    StftSegment segment;
    juce::dsp::WindowingFunction<float> window(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
    window.multiplyWithWindowingTable(fourierData.data(),fftSize);
    m_forwardFFT.performFrequencyOnlyForwardTransform (fourierData.data());
    segment.frequencyData = fourierData;
    m_currentSampleNum++;
    segment.StartIndex = m_currentSampleNum;
    std::cout << m_currentSampleNum << std::endl;
    m_stft.push_back(&segment);
}

void Stft::printStftValues() {
    std::cout << "STFT contains " << m_stft.size() << " segments" << std::endl;
    for (size_t i = 0; i < m_stft.size(); ++i) {
        std::cout << "Segment " << i << " (StartIndex: " << m_stft[i]->StartIndex << "):" << std::endl;
        std::cout << "  Frequency data preview (first 10 values): ";
        for (size_t j = 0; j < 10 && j < fftSize * 2; ++j) {
            std::cout << m_stft[i]->frequencyData[j] << " ";
        }
        std::cout << "..." << std::endl;
    }
}

void Stft::drawSpectorgramWithStft(juce::Image &specImage) {
    auto w = specImage.getWidth();
    // int sliceWidth = juce::jlimit(1, w, sliceWidth);

    auto imageHeight = specImage.getHeight();
    
    for (size_t i=0; i < m_stft.size(); ++i) {
        auto rightHandEdge = specImage.getWidth() -1;
        specImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight); 
        auto maxLevel = juce::FloatVectorOperations::findMinAndMax(m_stft[i]->frequencyData.data(), m_fftSize/2);

        m_forwardFFT.performRealOnlyInverseTransform(m_stft[i]->frequencyData.data());
        juce::Image::BitmapData bitmap { specImage, int(i), 0, 1, imageHeight, juce::Image::BitmapData::writeOnly };

        for (auto y = 1; y < imageHeight; ++y) {
            auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.2f);
            auto fftDataIndex = (size_t) juce::jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
            auto level = juce::jmap(m_stft[i]->frequencyData[y], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
            bitmap.setPixelColour (0, y, juce::Colour::fromHSV (level, 1.0f, level, 1.0f));
        }

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