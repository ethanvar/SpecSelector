#pragma once

#include "StftBuffers.h"
#include <array>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

struct StftSegment {
  static constexpr int fftSize = 1 << 9;
  std::array<float, fftSize * 2> frequencyData;
  int StartIndex;
};

class Stft {
private:
  juce::dsp::FFT m_forwardFFT;
  juce::dsp::WindowingFunction<float> window;
  static constexpr auto m_fftOrder = 9;
  static constexpr auto m_fftSize = 1 << m_fftOrder;
  std::array<float, m_fftSize * 2> m_fftData;
  int m_fifoIndex = 0;

  static constexpr int numBins = m_fftSize / 2 + 1; // 513 bins
  static constexpr int overlap = 4;                 // 75% overlap
  static constexpr int hopSize = m_fftSize / overlap;
  static constexpr float windowCorrection = 22.0f / 3.0f;

  int count = 0;
  int pos = 0;
  std::array<float, m_fftSize> inputFifo;
  std::array<float, m_fftSize> outputFifo;
  // int m_hopSize;
  // int m_sampleRate;
  int m_currentSampleNum = 0;
  std::vector<StftSegment> m_stft;
  float m_maxDBGlobal;
  juce::AudioBuffer<float> m_reconstructedAudio;

public:
  static constexpr auto fftOrder = 9;
  static constexpr auto fftSize = 1 << fftOrder;

  Stft();
  ~Stft();

  float velocity = 50;

  int &getNumFrames();
  void processEntireFile(juce::AudioFormatReader *reader);
  void findMinMax();
  void addLineToStft(std::array<float, StftSegment::fftSize * 2> &fourierData);
  void printStftValues();
  void drawSpectorgramWithStft(juce::Image &specImage);
  bool hasData() const;
  void invertFttSegments();
  void PingStftCpp();

  void reset();
  float processSample(float sample, bool bypassed);
  void processSpectrum(float *data, int numBins);
  void processFrame(bool bypassed);
  int getLatencyInSamples() const;
};
