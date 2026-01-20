# JUCE Audio Plugin Template

A minimal JUCE audio plugin template with CMake build system.

## Requirements

- CMake 3.22 or higher
- JUCE 7 or higher
- C++17 compatible compiler

## Building

1. Clone JUCE (if not already installed):
```bash
git clone https://github.com/juce-framework/JUCE.git
cd JUCE
```

2. Build and install JUCE (CMake):
```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build
```

3. Build this plugin:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Plugin Features

## Files

- `PluginProcessor.h/cpp` - Audio processing logic
- `PluginEditor.h/cpp` - UI implementation
- `CMakeLists.txt` - CMake build configuration

## License

This template is provided as-is for educational purposes.
