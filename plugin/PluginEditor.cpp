#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"
#include <iostream>

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p), mediator(p) {
  juce::ignoreUnused(processorRef);
  startTimerHz(60);
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(1280, 720);
  addAndMakeVisible(specViewport);
  specViewport.setViewedComponent(&mediator, false);
  specViewport.setScrollBarPosition(false, true);

  midiVolume.setSliderStyle(juce::Slider::LinearBarVertical);
  midiVolume.setRange(0.0, 127.0, 1.0);
  midiVolume.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
  midiVolume.setPopupDisplayEnabled(true, false, this);
  midiVolume.setTextValueSuffix(" Volume");
  midiVolume.setValue(50.0);
  addAndMakeVisible(&midiVolume);
  midiVolume.addListener(this);

  midiPhase.setSliderStyle(juce::Slider::LinearBarVertical);
  midiPhase.setRange(0.0, 127.0, 1.0);
  midiPhase.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
  midiPhase.setPopupDisplayEnabled(true, false, this);
  midiPhase.setTextValueSuffix(" Phase");
  midiPhase.setValue(1.0);
  addAndMakeVisible(&midiPhase);
  midiPhase.addListener(this);

  addAndMakeVisible(&checkTime);
  checkTime.setButtonText("Check Time");
  checkTime.onClick = [this] { checkTimeClicked(); };
  addAndMakeVisible(&openButton);
  openButton.setButtonText("Choose file");
  openButton.onClick = [this] { openButtonClicked(); };

  addAndMakeVisible(&inverseFft);
  inverseFft.setButtonText("inverseFft");
  inverseFft.onClick = [this] { inverseFttButton(); };
  addAndMakeVisible(&playButton);
  playButton.setButtonText("Play Inverse");
  playButton.onClick = [this] { togglePlayInverseFft(); };
  addAndMakeVisible(&pauseButton);
  pauseButton.setButtonText("Undo");
  pauseButton.onClick = [this] { togglePlayInverseFft(); };
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.drawRect(spectrogramBounds, 2);
  g.setColour(juce::Colours::black);
  g.setFont(15.0f);
  g.drawFittedText("Midi Volume Test", 0, 0, getWidth(), 30,
                   juce::Justification::centred, 1);

  // draw image
  juce::RectanglePlacement placement;
  g.setColour(juce::Colours::white);
}

void AudioPluginAudioProcessorEditor::openButtonClicked() {
  chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                juce::File{}, "*.wav;*.mp3");
  auto chooserFlags = juce::FileBrowserComponent::openMode |
                      juce::FileBrowserComponent::canSelectFiles;
  chooser->launchAsync(chooserFlags, [this](const juce::FileChooser &fc) {
    auto file = fc.getResult();
    processorRef.loadFile(file);
  });
  openButton.setEnabled(true);
}

void AudioPluginAudioProcessorEditor::inverseFttButton() {
  processorRef.stft.invertFttSegments();
  std::cout << "inverse Button Clicked" << std::endl;
}

void AudioPluginAudioProcessorEditor::togglePlayInverseFft() {
  mediator.undoLastStroke();
  std::cout << "togglePlayInverseFft clicked" << std::endl;
}

void AudioPluginAudioProcessorEditor::checkTimeClicked() {
  std::cout << processorRef.retTrackTimeInSeconds() << std::endl;
}

void AudioPluginAudioProcessorEditor::resized() {
  spectrogramBounds = getLocalBounds().withSizeKeepingCentre(
      getWidth() * 50 / 100, getHeight() * 50 / 100);
  midiVolume.setBounds(40, 30, 20, getHeight() - 60);
  midiPhase.setBounds(70, 30, 20, getHeight() - 60);
  openButton.setBounds((getWidth() / 2) - 40, 40, 80, 30);
  inverseFft.setBounds((getWidth() / 2) - 40, 120, 80, 30);
  checkTime.setBounds((getWidth() / 2) - 40, 80, 80, 30);
  playButton.setBounds((getWidth() / 2) - 40,
                       120 + spectrogramBounds.getHeight() + 110, 80, 30);
  pauseButton.setBounds((getWidth() / 2) - 40,
                        120 + spectrogramBounds.getHeight() + 150, 80, 30);
  specViewport.setBounds(spectrogramBounds);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider *slider) {
  // processorRef.fft[0].PingStftCpp()
  //  Make sure you fix the extra velocities here
  if (slider == &midiPhase) {
    processorRef.dialInputs.phase = float(midiPhase.getValue());
  };
  if (slider == &midiVolume) {
    processorRef.dialInputs.magnitude = float(midiVolume.getValue());
  };
  // processorRef.noteOnVel = float(midiVolume.getValue() / double(100));

  // processorRef.fft[0].velocity = float(midiVolume.getValue() / double(100));
  // processorRef.fft[1].velocity = float(midiVolume.getValue() / double(100));
}

void AudioPluginAudioProcessorEditor::timerCallback() {
  double samplesPassed =
      processorRef.retSamplesPassed() + processorRef.getLatencySamples();

  // if (samplesPassed != 0 &&
  //   std::fmod(samplesPassed, double(processorRef.stft.fftSize)) == 0) {
  //  double segmentsPassed = samplesPassed / processorRef.stft.fftSize;
  //
  mediator.updateMaskProgressWithSegments(processorRef.stft.segmentsPassed);
  // mediator.updateMaskProgressWithSegments(segmentsPassed);
  //}

  if (processorRef.getRepaint() == true) {
    auto newBounds = getBounds();
    newBounds.setWidth(processorRef.stft.getNumFrames() + 1);
    newBounds.setHeight(newBounds.getHeight() * 50 / 100);
    mediator.setBounds(newBounds);
    mediator.resized();
    mediator.initializeStftDisplay();
  }
}
