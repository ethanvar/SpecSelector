#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    
    // Initialize spectrogram bounds (centered, 75% width, 50% height)
    spectrogramBounds = getLocalBounds().withSizeKeepingCentre(
        getWidth() * 75 / 100, getHeight() * 50 / 100);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draw spectrogram placeholder rectangle
    g.setColour (juce::Colours::darkgrey);
    g.fillRect (spectrogramBounds);
    g.setColour (juce::Colours::lightgrey);
    g.drawRect (spectrogramBounds, 2);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Update spectrogram bounds when window resizes
    spectrogramBounds = getLocalBounds().withSizeKeepingCentre(
        getWidth() * 75 / 100, getHeight() * 50 / 100);
}