# PeakRMSCompressorWorkbench

This repository contains the **PeakRMSCompressorWorkbench**, a JUCE-based application designed for testing of audio dynamic range compression algorithms. The application provides tools for measuring and comparing the effects of dynamic range compression using peak-based and rms-based level detection on audio signals.

---

## Features
- **Peak-based and RMS-based Compression**: Evaluate compression effects using both approaches.
- **Metrics Extraction**: Includes LUFS, dynamic range, crest factor, and more.
- **User Configurations**: Configurable presets and real-time parameter adjustment.
- **Export Metrics**: Save detailed metrics to output files.

---

## Windows setup

### Prerequisites
To run this project, you'll need:
1. [JUCE](https://juce.com/): A C++ framework for audio application development.
2. [AudioFilePlayerPlugin](https://github.com/hollance/AudioFilePlayerPlugin): A JUCE example project required for real-time audio compression playback.

---

### Step 1: Download JUCE
Follow these steps to set up JUCE on Windows:

1. **Download JUCE**:
   - Visit [JUCE Downloads](https://juce.com/get-juce) and download the latest version.
   - Extract the downloaded ZIP file to a location of your choice.

2. **Install Visual Studio**:
   - Download and install [Visual Studio Community Edition](https://visualstudio.microsoft.com/) (or a compatible version).
   - During installation, ensure you select the **Desktop Development with C++** workload.

3. **Open JUCE Projucer**:
   - Navigate to the JUCE folder and launch the `Projucer.exe` file.

4. **Set Global Paths**:
   - In Projucer, go to `File -> Global Paths`.
   - Set the global path to your Visual Studio installation.

---

### Step 2: Download AudioFilePlayerPlugin
1. Clone the repository for `AudioFilePlayerPlugin`:
   ```bash
   git clone https://github.com/hollance/AudioFilePlayerPlugin.git
