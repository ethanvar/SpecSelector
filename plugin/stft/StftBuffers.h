#pragma once
#include <juce_dsp/juce_dsp.h>

#include <array>
class StftBuffers {
private:
  const static constexpr int fftSize = 1 << 9;
  std::array<float, fftSize * 2> m_fftData;
  std::array<float, fftSize> inputfifo;
  std::array<float, fftSize> outputfifo;

  static constexpr int numBins = fftSize / 2 + 1; // 513 bins
  static constexpr int overlap = 4;               // 75% overlap
  static constexpr int hopSize = fftSize / overlap;
  static constexpr float windowCorrection = 22.0f / 3.0f;

  const juce::dsp::WindowingFunction<float> &window;
  const juce::dsp::FFT &forwardFFT;
  int count = 0;
  int pos = 0;
  std::array<float, fftSize> inputFifo;
  std::array<float, fftSize> outputFifo;

public:
  StftBuffers(const juce::dsp::WindowingFunction<float> &w,
              const juce::dsp::FFT &f);
  ~StftBuffers();
  void reset();
  float processSample(float sample, bool bypassed);
  void processSpectrum(float *data, int numBins);
  void processFrame(bool bypassed);
};
