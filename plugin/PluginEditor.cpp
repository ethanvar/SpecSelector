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
    midiVolume.setSliderStyle (juce::Slider::LinearBarVertical);
    midiVolume.setRange (0.0, 127.0, 1.0);
    midiVolume.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled (true, false, this);
    midiVolume.setTextValueSuffix (" Volume");
    midiVolume.setValue (50.0);
    addAndMakeVisible(&midiVolume);
    midiVolume.addListener(this);
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
    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Midi Volume Test", 0, 0, getWidth(), 30, juce::Justification::centred, 1);

}

void AudioPluginAudioProcessorEditor::resized()
{
    // Update spectrogram bounds when window resizes
    spectrogramBounds = getLocalBounds().withSizeKeepingCentre(
        getWidth() * 75 / 100, getHeight() * 50 / 100);
    midiVolume.setBounds (40, 30, 20, getHeight() - 60);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
    processorRef.noteOnVel = float(midiVolume.getValue()/double(100));
}