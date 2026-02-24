#pragma once

#include "PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "views/ProgressMask.h"
#include "views/StftDisplay.h"

class Mediator : public juce::Component {
  public:
      Mediator(AudioPluginAudioProcessor &p);
      ~Mediator();

      void paint(juce::Graphics &g) override;
      void resized() override;

  private:
      AudioPluginAudioProcessor &processorRef;
      ProgressMask m_mask;
      StftDisplay m_stftDisplay;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mediator)
};
