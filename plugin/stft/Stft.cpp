#include "Stft.h"
#include "../interactable/DrawStrokeAction.h"
#include "StftBuffers.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_graphics/juce_graphics.h"

#include <iostream>

Stft::Stft(DialInputs &inputs)
    : m_forwardFFT(m_fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann, false),
      fftBuffers{StftBuffers(window, m_forwardFFT, inputs),
                 StftBuffers(window, m_forwardFFT, inputs)} {
  m_reconstructedAudio.setSize(2, 516);
};

Stft::~Stft() {};

void Stft::processEntireFile(juce::AudioFormatReader *reader) {
  m_stft.clear();
  std::cout << "Format created, again: " << reader->getFormatName()
            << std::endl;

  juce::AudioBuffer<float> fullBuffer((int)reader->numChannels,
                                      (int)reader->lengthInSamples);

  m_reconstructedAudio.setSize((int)reader->numChannels,
                               (int)reader->lengthInSamples);
  std::array<float, fftSize> firstOut;
  std::array<float, fftSize * 2> fourierData;

  if (!reader->read(&fullBuffer, 0, (int)reader->lengthInSamples, 0, true,
                    true)) {
    std::cout << "Failed to read audio file" << std::endl;
    return;
  } else {
    std::cout << "Successfully read audio file" << std::endl;
  }

  int index = 0;

  std::cout << "Checking buffer: " << fullBuffer.getSample(0, 500) << std::endl;
  std::cout << "Number of samples from buffer: " << fullBuffer.getNumSamples()
            << std::endl;

  for (int i = 0; i < fullBuffer.getNumSamples(); i++) {
    firstOut[(size_t)index++] = fullBuffer.getSample(0, i);

    if (i > 0 && fftSize == index) {
      std::fill(fourierData.begin(), fourierData.end(), 0.0f);
      std::copy(firstOut.begin(), firstOut.end(), fourierData.begin());

      addLineToStft(fourierData);

      index = 0;
    }
  }
  findMinMax();
}

void Stft::addLineToStft(std::array<float, fftSize * 2> &fourierData) {
  StftSegment segment;
  juce::dsp::WindowingFunction<float> window(
      fftSize, juce::dsp::WindowingFunction<float>::hamming, true);
  window.multiplyWithWindowingTable(fourierData.data(), fftSize);
  m_forwardFFT.performRealOnlyForwardTransform(fourierData.data(), true);
  segment.frequencyData = fourierData;
  m_currentSampleNum++;
  segment.StartIndex = m_currentSampleNum;
  m_stft.push_back(segment);
}

void Stft::invertFttSegments() {
  std::cout << "Inverting fft segments" << std::endl;
  std::cout << m_currentSampleNum << std::endl;
  for (int i = 0; i < m_currentSampleNum; ++i) {
    m_forwardFFT.performRealOnlyInverseTransform(
        m_stft[i].frequencyData.data());
    juce::dsp::WindowingFunction<float> window(
        fftSize, juce::dsp::WindowingFunction<float>::hamming, true);
    window.multiplyWithWindowingTable(m_stft[i].frequencyData.data(), fftSize);
    m_reconstructedAudio.copyFrom(m_reconstructedAudio.getNumChannels(),
                                  i * m_fftSize, m_stft[i].frequencyData.data(),
                                  m_fftSize);
  };
}

void Stft::printStftValues() {
  std::cout << "STFT contains " << m_stft.size() << " segments" << std::endl;
  for (size_t i = 0; i < m_stft.size(); ++i) {
    std::cout << "Segment " << i << " (StartIndex: " << m_stft[i].StartIndex
              << "):" << std::endl;
    std::cout << "  Frequency data preview (first 10 values): ";
    for (size_t j = 0; j < 10 && j < fftSize * 2; ++j) {
      std::cout << m_stft[i].frequencyData[j] << " ";
    }
    std::cout << "..." << std::endl;
  }
}

void Stft::drawSpectorgramWithStft(juce::Image &specImage) {
  const int imageWidth = specImage.getWidth();
  const int imageHeight = specImage.getHeight();
  std::cout << "image height" << imageHeight << std::endl;
  const int numFrames = static_cast<int>(m_stft.size());
  // Loop over each FFT frame
  for (int i = 0; i < numFrames; ++i) {
    // Get the FFT magnitudes for the current frame
    // const float *fftMagnitudes = m_stft[i].frequencyData.data();
    std::array<float, m_fftSize / 2> dbMagnitudes;
    for (int j = 0; j < m_fftSize / 2; ++j) {
      /** Since we arent using performFrequencyOnlyForwardTransform we need to
      convert the complex values from performRealOnlyForwardTransform into
      magnitude values **/
      float real = m_stft[i].frequencyData[2 * j];
      float imag = m_stft[i].frequencyData[2 * j + 1];
      float magnitude = std::sqrt(real * real + imag * imag);
      // Use a small epsilon to avoid log(0)
      dbMagnitudes[j] = 20.0f * std::log10(std::max(magnitude, 1e-6f));
    }

    // Find min/max dB for normalization (considering only half of the FFT bins)
    auto minMaxDb = juce::FloatVectorOperations::findMinAndMax(
        dbMagnitudes.data(), m_fftSize / 2);
    float minDb = minMaxDb.getStart();
    // float maxDb = juce::jmax(minMaxDb.getEnd(), m_maxDbGlobal); // Use the
    // local max dB found
    float maxDb = minMaxDb.getEnd(); // Use the local max dB found

    // Lock the rightmost column of the image (where we draw this FFT)
    juce::Image::BitmapData bitmap(specImage,
                                   imageWidth - 1, // rightmost column
                                   0,
                                   1, // width = 1 pixel
                                   imageHeight,
                                   juce::Image::BitmapData::writeOnly);

    // Draw frequency bins bottom top (optionally skew Y axis to emphasize low
    // frequencies)
    for (int y = 0; y < imageHeight; ++y) {
      // Map pixel Y normalized frequency bin (0..1spec)
      float skewedProportionY =
          1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);

      // Map to FFT index
      int fftIndex = juce::jlimit(
          0, m_fftSize / 2,
          static_cast<int>(skewedProportionY * (float(m_fftSize) / 2)));

      // Get the dB value for this frequency bin
      float dbValue = dbMagnitudes[fftIndex];

      float normalizedDb =
          juce::jmap(std::pow(dbValue - minDb, 2.0f), 0.0f,
                     std::pow(maxDb - minDb, 2.0f), 0.0f, 1.0f);

      // Convert normalized dB to color (HSV, from low frequency red to high
      // frequency blue)
      juce::Colour colour =
          juce::Colour::fromHSV(normalizedDb, // hue (based on magnitude)
                                1.0f,         // full saturation
                                normalizedDb, // value (brightness)
                                1.0f          // alpha
          );

      // Set the pixel color based on the calculated magnitude
      // if (i < 20) {
      //  bitmap.setPixelColour(0, y, juce::Colours::orange);
      //} else if (i > numFrames-20) {
      //  bitmap.setPixelColour(0, y, juce::Colours::orange);
      //} else {
      bitmap.setPixelColour(0, y, colour);
      //}
    }

    // Move column forward for next loop to draw into image
    specImage.moveImageSection(0, 0, 1, 0, imageWidth - 1, imageHeight);
  }
}

void Stft::drawOntoSpectrogram(int mouseEventX, int mouseEventY,
                               juce::Image &specImage) {

  for (int i = 0; i < 20; i++) {
    specImage.setPixelAt(mouseEventX + i, mouseEventY + i,
                         juce::Colour(100, 0, 0));
    specImage.setPixelAt(mouseEventX - i, mouseEventY - i,
                         juce::Colour(100, 0, 0));
  }
};

void Stft::drawOntoSpectrogram(const DrawingStroke &stroke,
                               juce::Image &specImage) {
  for (const auto &info : stroke.strokeInfo) {
    for (int i = 0; i < info.radius; i++) {
      specImage.setPixelAt(info.x + i, info.y + i, info.colour);
      specImage.setPixelAt(info.x - i, info.y - i, info.colour);
    }
  }
}

void Stft::findMinMax() {
  const int numFrames = static_cast<int>(m_stft.size());
  float maxNum = 0;
  for (int i = 0; i < numFrames; ++i) {
    const float *fftMagnitudes = m_stft[i].frequencyData.data();
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftMagnitudes,
                                                               m_fftSize / 2);
    if (maxNum < maxLevel.getEnd()) {
      maxNum = maxLevel.getEnd();
    }
  }
  m_maxDBGlobal = maxNum;
}

void Stft::updateSegmentsPassedInTrack(double samplesPassedInTrack,
                                       double latencySamples) {
  double samplesPassed = samplesPassedInTrack + latencySamples;
  if (samplesPassed != 0 && std::fmod(samplesPassed, double(fftSize)) == 0) {
    // std::cout << "samplesPassed: " << samplesPassed << std::endl;
    segmentsPassed = samplesPassed / fftSize;
    // std::cout << segmentsPassed << std::endl;
  }
}

int &Stft::getNumFrames() { return m_currentSampleNum; }

bool Stft::hasData() const { return !m_stft.empty(); }

int Stft::getLatencyInSamples() const { return m_fftSize; }
