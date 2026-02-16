#pragma once

#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

class ProgressMask: public juce::Component {
    public:
        ProgressMask();
        ~ProgressMask();

        void paint(juce::Graphics& g) override;
        void updateProgress(double progress, double songLength);
        void resized() override;
        void setFftSize(int size);        
        void setNumFrames(int& frames);

    private:
        double m_currentProgress = 0.0;
        double m_currentSongLength = 1.0;
        int m_fftSize = 0;
        int* m_numFrames = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressMask)
};
