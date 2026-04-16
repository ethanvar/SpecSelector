#pragma once

#include "../PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "ProgressMask.h"
#include "StftDisplay.h"

class Mediator : public juce::Component {
  public:
    Mediator(AudioPluginAudioProcessor &p);
    ~Mediator();

    void resized() override;
    void paint(juce::Graphics &g) override;
    void updatetStftDisplay();
    void updateMaskProgress(double progress, double songLength);
    void mouseDown(const juce::MouseEvent& event) override;

  private:
    AudioPluginAudioProcessor &processorRef;
    ProgressMask m_mask;
    StftDisplay m_stftDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mediator)
};
