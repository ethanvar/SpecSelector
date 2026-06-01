#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
  formatManager.registerBasicFormats();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
  return "JucePlugin_Name";
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  setLatencySamples(stft.getLatencyInSamples());

  fft[0].reset();
  fft[1].reset();

  // juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
  // juce::ignoreUnused (midiMessages);

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  auto numSamples = buffer.getNumSamples();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  if (auto playHead = getPlayHead()) {
    if (auto posInfo = playHead->getPosition()) {
      currentPosition = *posInfo;
    }
  }
  // This is the place where you'd normally do the guts of your plugin's
  // audio processing...
  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.
  bool bypassed = false;

  float *channelL = buffer.getWritePointer(0);
  float *channelR = buffer.getWritePointer(1);

  for (int channel = 0; channel < totalNumInputChannels; ++channel) {
    auto *channelData = buffer.getWritePointer(channel);
    juce::ignoreUnused(channelData);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
      if (currentPosition.getTimeInSamples() < 601600 &&
          currentPosition.getTimeInSamples() > 356352) {
        channelData[sample] *= noteOnVel;
      }

      float sampleL = channelL[sample];
      float sampleR = channelR[sample];

      sampleL = fft[0].processSample(sampleL, bypassed);
      sampleR = fft[1].processSample(sampleR, bypassed);

      channelL[sample] = sampleL;
      channelR[sample] = sampleR;
    }
    // ..do something to the data...
  }
}

void AudioPluginAudioProcessor::loadFile(const juce::File &audioFile) {
  if (audioFile.existsAsFile()) {
    std::cout << "Selected file: " << audioFile.getFullPathName() << std::endl;
    auto *reader = formatManager.createReaderFor(audioFile); // [10]

    // stft = Stft((reader->sampleRate), fftOrder);
    if (reader != nullptr) {
      std::cout << "Format created: " << reader->getFormatName() << std::endl;
      auto newSource =
          std::make_unique<juce::AudioFormatReaderSource>(reader, true);
      transportSource.setSource(newSource.get(), 0, nullptr,
                                reader->sampleRate);
      std::cout << "Total length in samples: "
                << transportSource.getTotalLength() << std::endl;
      std::cout << "Please dont seg fault" << std::endl;
      stft.processEntireFile(reader);
      // stft.printStftValues();
      // stft.drawSpectorgramWithStft(specImage);
      repaint = true;
      std::cout << repaint << std::endl;
      readerSource.reset(newSource.release());
    }
  }
}

void AudioPluginAudioProcessor::playEditedSpectrogram() {}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

double AudioPluginAudioProcessor::retTrackTimeInSeconds() {
  if (auto t = currentPosition.getTimeInSeconds()) {
    return *t;
  }
  return 0.0;
}

double AudioPluginAudioProcessor::retLoadedSongInSeconds() {
  if (auto t = transportSource.getLengthInSeconds()) {
    return t;
  }
  return 0.0;
}
double AudioPluginAudioProcessor::retSamplesPassed() {
  if (auto t = currentPosition.getTimeInSamples()) {
    return static_cast<double>(*t);
  }
  return 0.0;
}

double AudioPluginAudioProcessor::retLoadedSongInSamples() {
  if (auto t = transportSource.getTotalLength()) {
    return t;
  }
  return 0.0;
}

void AudioPluginAudioProcessor::setRepaint(bool value) { repaint = value; }

bool AudioPluginAudioProcessor::getRepaint() { return repaint; }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}
