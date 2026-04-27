#pragma once

#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "../PluginProcessor.h"

class StftDisplay : public juce::Component {
  public:
    StftDisplay(AudioPluginAudioProcessor&);
    ~StftDisplay();
    
    void paint(juce::Graphics &g) override;
    void resized() override;
    void checking();
    void updateSpectrogram();

  private:
    int displayWidth;
    juce::Image spectrogram;
    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StftDisplay)
};
