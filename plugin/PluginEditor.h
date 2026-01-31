#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Slider::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void openButtonClicked();
    void timerCallback();

private:
    // Rectangle area for spectrogram visualization
    juce::Rectangle<int> spectrogramBounds;
    juce::Image specImage;
    void sliderValueChanged (juce::Slider * slider) override;
    juce::Slider midiVolume;
    juce::TextButton openButton;
    std::unique_ptr<juce::FileChooser> chooser;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};