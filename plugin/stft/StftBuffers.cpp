#include "StftBuffers.h"

StftBuffers::StftBuffers(const juce::dsp::WindowingFunction<float> &w,
                         const juce::dsp::FFT &f, DialInputs &inputs)
    : window(w), forwardFFT(f), inputsRef(inputs) {}

StftBuffers::~StftBuffers() {}

void StftBuffers::reset() {
  count = 0;
  pos = 0;
  std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
  std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
}

float StftBuffers::processSample(float sample, bool bypassed) {
  inputFifo[pos] = sample;
  float outputSample = outputFifo[pos];
  // float outputSample = 0;
  outputFifo[pos] = 0.0f;

  pos += 1;
  if (pos == fftSize) {
    pos = 0;
  }

  count += 1;
  if (count == hopSize) {
    count = 0;
    processFrame(bypassed);
  }

  return outputSample;
}

void StftBuffers::processFrame(bool bypassed) {
  const float *inputPtr = inputFifo.data();
  float *fftPtr = fftData.data();

  std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
  if (pos > 0) {
    std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
  }

  window.multiplyWithWindowingTable(fftPtr, fftSize);

  if (!bypassed) {
    forwardFFT.performRealOnlyForwardTransform(fftPtr, true);
    processSpectrum(fftPtr, numBins);
    forwardFFT.performRealOnlyInverseTransform(fftPtr);
  }

  window.multiplyWithWindowingTable(fftPtr, fftSize);

  for (int i = 0; i < fftSize; ++i) {
    fftPtr[i] *= windowCorrection;
  }

  for (int i = 0; i < pos; ++i) {
    outputFifo[i] += fftData[i + fftSize - pos];
  }
  for (int i = 0; i < fftSize - pos; ++i) {
    outputFifo[i + pos] += fftData[i];
  }
}

void StftBuffers::processSpectrum(float *data, int numBins) {
  std::complex<float> *cdata = reinterpret_cast<std::complex<float> *>(data);

  for (int i = 0; i < numBins; ++i) {
    float magnitude = std::abs(cdata[i]);
    float phase = std::arg(cdata[i]);
    // std::cout << inputsRef.magnitude << std::endl;
    //  This is where you'd do your spectral processing..
    //
    magnitude *= (float(0.01) * inputsRef.magnitude);
    phase *= inputsRef.phase;
    cdata[i] = std::polar(magnitude, phase);
  }
}
