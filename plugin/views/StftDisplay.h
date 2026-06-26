#pragma once

#include "../PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

struct PixelUpdate {
  int pixelX;
  int pixelY;
  juce::Colour colour;
};

class StftDisplay : public juce::Component {
public:
  StftDisplay(AudioPluginAudioProcessor &);
  ~StftDisplay();

  void paint(juce::Graphics &g) override;
  void resized() override;
  void checking();
  void initializeSpectrogram();
  void updateSpectrogram(PixelUpdate &update);
  juce::Image spectrogram;

private:
  int displayWidth;
  AudioPluginAudioProcessor &processorRef;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StftDisplay)
};
