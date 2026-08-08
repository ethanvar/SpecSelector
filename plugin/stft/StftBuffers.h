#pragma once
#include "../events/EventBus.h"
#include "../interactable/DialInputs.h"
#include "../strokemap/StrokeMap.h"
#include <array>
#include <atomic>
#include <juce_dsp/juce_dsp.h>

class StftBuffers {
private:
  const static constexpr int fftSize = 1 << 9;
  std::array<float, fftSize * 2> fftData;

  static constexpr int numBins = fftSize / 2 + 1;
  static constexpr int overlap = 4;
  static constexpr int hopSize = fftSize / overlap;
  static constexpr float windowCorrection = 22.0f / 3.0f;

  const juce::dsp::WindowingFunction<float> &window;
  const juce::dsp::FFT &forwardFFT;
  int count = 0;
  int pos = 0;
  std::array<float, fftSize> inputFifo;
  std::array<float, fftSize> outputFifo;
  DialInputs &inputsRef;
  const StrokeMap *strokeMap = nullptr;
  int &segmentsPassed;
  std::atomic<int> currentIndex{0};
  // std::unique_ptr<int> imageHeight;

public:
  StftBuffers(const juce::dsp::WindowingFunction<float> &w,
              const juce::dsp::FFT &f, DialInputs &inputs, StrokeMap *map,
              int &_segsPassed);
  ~StftBuffers();
  void reset();
  float processSample(float sample, bool bypassed);
  void processSpectrum(float *data, int numBins);
  void processFrame(bool bypassed);
  void getCurrentIndex();
};
