#pragma once

#include "PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "views/ProgressMask.h"
#include "views/StftDisplay.h"

class Mediator : public juce::Component {
    public:
        Mediator(AudioPluginAudioProcessor &p);
        ~Mediator();

        void resized() override;
        void paint(juce::Graphics &g) override;
        void updatetStftDisplay();
        void updateMaskProgress(double progress, double songLength);


    private:
        AudioPluginAudioProcessor &processorRef;
        ProgressMask m_mask;
        StftDisplay m_stftDisplay;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mediator)
};
