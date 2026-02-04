#include "PluginProcessor.h"
#include <iostream>
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), specImage (juce::Image::RGB, 1200, 600, true), progressMask(juce::Image::RGB, 1200, 600, true)
{
    juce::ignoreUnused (processorRef);
    startTimerHz(60); 
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1024, 768);
    juce::Logger::getCurrentLogger()->writeToLog ("Hello");
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
    addAndMakeVisible(&checkTime);
    checkTime.setButtonText("Check Time");
    checkTime.onClick = [this] {checkTimeClicked(); };
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Choose file");
    openButton.onClick = [this] { openButtonClicked(); };
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

    //draw image
    juce::RectanglePlacement placement;
    g.setColour (juce::Colours::white);

    // g.drawImage (specImage, spectrogramBounds.toFloat());
    g.drawImageWithin(specImage, spectrogramBounds.getX(), spectrogramBounds.getY(), spectrogramBounds.getWidth(), spectrogramBounds.getHeight(), placement, false);

}

void AudioPluginAudioProcessorEditor::openButtonClicked() {
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...", juce::File {}, "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        processorRef.loadFile(file, specImage);
    });
    openButton.setEnabled (true);
}

void AudioPluginAudioProcessorEditor::checkTimeClicked() {
    std::cout << processorRef.retTrackTimeInSeconds() << std::endl;
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Update spectrogram bounds when window resizes
    spectrogramBounds = getLocalBounds().withSizeKeepingCentre(
        getWidth() * 75 / 100, getHeight() * 50 / 100);
    midiVolume.setBounds (40, 30, 20, getHeight() - 60);
    openButton.setBounds ((getWidth()/2)-40, 60, 80, 30);
    checkTime.setBounds ((getWidth()/2)-40, 100, 80, 30);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
    processorRef.noteOnVel = float(midiVolume.getValue()/double(100));
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    if (processorRef.getRepaint() == true)
    {
        std::cout << "Redrawing" << std::endl;
        repaint();
        std::cout << "Redrawn?" << std::endl;
        processorRef.setRepaint(false);
    }
}
