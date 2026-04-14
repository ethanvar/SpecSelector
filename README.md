# ChordChainer Plugin System

## Project Goal
This project implements a modular, cross-platform audio plugin system designed for Digital Audio Workstations (DAWs). Its primary goal is to allow users to process audio signals by visualizing and manipulating spectral data (via STFT) and exporting the resulting modified audio stream.

## Project Structure
The core logic resides within the `plugin/` directory, which contains the main components:

*   **`PluginProcessor`**: The core audio engine. It inherits from `juce::AudioProcessor` and is responsible for the real-time processing of audio blocks. It manages the STFT analysis and applies any necessary signal modifications based on user input.
*   **`PluginEditor`**: The Graphical User Interface (GUI). It inherits from `juce::AudioProcessorEditor` and provides the visual controls (sliders, buttons) for the user to interact with the plugin.
*   **`Mediator`**: Acts as the central communication hub, mediating interactions between the GUI (`PluginEditor`) and the audio engine (`PluginProcessor`).
*   **`stft/`**: Contains the Short-Time Fourier Transform implementation (`Stft.h`/`.cpp`), which is crucial for spectral analysis and visualization.
*   **`views/`**: Contains visual components like `StftDisplay` (for visualizing the spectrogram) and `ProgressMask`.

## Component Interaction Flow
1.  **Initialization:** The DAW loads the plugin, creating an instance of `PluginProcessor` and `PluginEditor`. The `PluginEditor` holds a reference to the `PluginProcessor`.
2.  **User Interaction (GUI $\rightarrow$ Engine):** A user interacts with a control in the `PluginEditor` (e.g., moving a slider). The `PluginEditor` notifies the `Mediator`, which in turn calls the appropriate setter method on the `PluginProcessor` to update the processing parameters.
3.  **Audio Processing (Engine):** During `processBlock`, the `PluginProcessor` reads its current parameters, performs the STFT analysis, applies the necessary signal modifications (e.g., based on a click event), and outputs the processed audio buffer.
4.  **Visualization:** The `StftDisplay` reads the processed data from the `PluginProcessor` to render the spectrogram in real-time.
5.  **Export (Future):** The system is designed to capture the processed state and allow for the export of the entire audio stream as a modified file (e.g., MP3).

## General Goal
To provide a robust, modular framework for building advanced, visually interactive audio effects plugins that can analyze, modify, and export audio based on spectral data.