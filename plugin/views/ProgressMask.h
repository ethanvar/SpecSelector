#include "juce_graphics/juce_graphics.h"

class ProgressMask { 
    public:
        ProgressMask();
        ~ProgressMask();
        void setMask(juce::Graphics& g, juce::Rectangle<int>& r, double& progress, double& songLength);
    private:
        juce::Image progressMask;

};
