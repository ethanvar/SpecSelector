#pragma once

#include "../interactable/DrawStrokeAction.h"
#include "../strokemap/StrokeMap.h"
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
  int m_fifoIndex = 0;
  int m_currentSampleNum = 0;
  std::vector<StftSegment> m_stft;
  float m_maxDBGlobal;
  juce::AudioBuffer<float> m_reconstructedAudio;

public:
  static constexpr auto fftOrder = 9;
  static constexpr auto fftSize = 1 << fftOrder;

  Stft(DialInputs &inputs);
  ~Stft();

  float velocity = 50;
  int segmentsPassed = 0;

  int &getNumFrames();
  void processEntireFile(juce::AudioFormatReader *reader);
  void findMinMax();
  void addLineToStft(std::array<float, StftSegment::fftSize * 2> &fourierData);
  void printStftValues();
  void drawSpectorgramWithStft(juce::Image &specImage);
  void drawOntoSpectrogram(int mouseEventX, int mouseEventY,
                           juce::Image &specImage);
  void drawOntoSpectrogram(const DrawingStroke &stroke, juce::Image &specImage);
  bool hasData() const;
  void invertFttSegments();
  StftBuffers fftBuffers[2];
  StrokeMap strokeMap;

  void updateSegmentsPassedInTrack(double samplesPassed, double latencySamples);
  int getLatencyInSamples() const;
};
