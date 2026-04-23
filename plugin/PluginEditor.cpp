#include "PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"
#include <iostream>
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), mediator(p)
{
    juce::ignoreUnused (processorRef);
    startTimerHz(60); 
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1280, 720);
    addAndMakeVisible(specViewport);
    specViewport.setViewedComponent(&mediator, false);
    specViewport.setScrollBarPosition(false, true);

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
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.drawRect (spectrogramBounds, 2);
    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Midi Volume Test", 0, 0, getWidth(), 30, juce::Justification::centred, 1);

    //draw image
    juce::RectanglePlacement placement;
    g.setColour (juce::Colours::white);


}

void AudioPluginAudioProcessorEditor::openButtonClicked() {
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...", juce::File {}, "*.wav;*.mp3");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        processorRef.loadFile(file);
    });
    openButton.setEnabled (true);
}

void AudioPluginAudioProcessorEditor::checkTimeClicked() { 
    std::cout << processorRef.retTrackTimeInSeconds() << std::endl;
}

void AudioPluginAudioProcessorEditor::resized()
{
    spectrogramBounds = getLocalBounds().withSizeKeepingCentre(
        getWidth() * 50 / 100, getHeight() * 50 / 100);
    midiVolume.setBounds (40, 30, 20, getHeight() - 60);
    openButton.setBounds ((getWidth()/2)-40, 60, 80, 30);
    checkTime.setBounds ((getWidth()/2)-40, 100, 80, 30);
    specViewport.setBounds(spectrogramBounds);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
    processorRef.noteOnVel = float(midiVolume.getValue()/double(100));
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
  double latencySeconds = processorRef.getLatencySamples() / processorRef.getSampleRate();
  double totalSeconds = processorRef.retLoadedSongInSeconds();
  double currentSeconds = processorRef.retTrackTimeInSeconds() + latencySeconds;
  mediator.updateMaskProgress(currentSeconds, totalSeconds);

  if (processorRef.getRepaint() == true) {
    auto newBounds = getBounds();
    newBounds.setWidth(processorRef.stft.getNumFrames() + 1);
    newBounds.setHeight(newBounds.getHeight() * 50 / 100);
    mediator.setBounds(newBounds);
    mediator.resized();
    mediator.updatetStftDisplay();
  }
}
