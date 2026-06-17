#pragma once

#include "PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "views/Mediator.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Slider::Listener,
                                              private juce::Timer {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
  ~AudioPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;
  void checkTimeClicked();
  void openButtonClicked();
  void inverseFttButton();
  void togglePlayInverseFft();
  void timerCallback() override;

private:
  // Rectangle area for spectrogram visualization
  juce::Rectangle<int> spectrogramBounds;
  juce::Viewport specViewport;
  Mediator mediator;
  // juce::Image specImage;
  // ProgressMask mask;
  // StftDisplay stftDisplay;
  void sliderValueChanged(juce::Slider *slider) override;
  juce::Slider midiVolume;
  juce::Slider midiPhase;
  juce::TextButton checkTime;
  juce::TextButton openButton;
  juce::TextButton inverseFft;
  juce::TextButton playButton;
  juce::TextButton pauseButton;
  std::unique_ptr<juce::FileChooser> chooser;

  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor &processorRef;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
