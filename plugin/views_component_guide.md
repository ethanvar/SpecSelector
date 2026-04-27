# Views Component Guide

This document describes the purpose and responsibilities of the visual components located in the `views/` directory. These components are responsible for rendering specific aspects of the plugin's UI.

## 1. `ProgressMask`
**Purpose:**
This component is responsible for visualizing the overall progress of the audio playback, such as song playback time or processing completion percentage. It acts as a visual indicator for time-based state changes.

**Key Responsibilities:**
*   **`updateProgress(double progress, double songLength)`:** Updates the visual progress bar based on the current time and total song length.
*   **`paint(juce::Graphics& g)`:** Renders the progress mask onto the given graphics context.
*   **State:** Manages and exposes the current progress and total song length.

## 2. `StftDisplay`
**Purpose:**
This component is dedicated to visualizing the Short-Time Fourier Transform (STFT) data, displaying the spectrogram of the audio signal.

**Key Responsibilities:**
*   **`updateSpectrogram()`:** Triggers the drawing of the spectrogram using data provided by the `PluginProcessor`'s STFT object.
*   **`paint(juce::Graphics& g)`:** Draws the stored spectrogram image onto the component.
*   **Interaction:** It handles mouse events (though this needs to be wired correctly) to allow interaction with the displayed data.

## Usage Notes
These components are designed to be managed and updated by a central coordinator, such as the `Mediator`, which calls their respective update methods (`updateSpectrogram`, `updateProgress`) during the plugin's lifecycle.