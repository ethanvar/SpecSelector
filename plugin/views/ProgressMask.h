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

    private:
        double currentProgress = 0.0;
        double currentSongLength = 1.0;
        juce::Image progressMask;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressMask)
};
