
# PeakRMSCompressorWorkbench

This repository hosts **PeakRMSCompressorWorkbench**, a JUCE-based application designed to test and analyze the effects of **dynamic range compression** algorithms on audio signals using **peak-based and RMS-based level detection** methods. It provides tools to measure and compare these effects via various metrics.

---

## Features

- **Peak and RMS Compression**: Analyze compression behavior using peak and RMS level detection methods.
- **Metrics Extraction**: LUFS, dynamic range, crest factor, and other key metrics.
- **Real-time Playback**: Integrates with [AudioFilePlayerPlugin](https://github.com/hollance/AudioFilePlayerPlugin) by **Matkat Music** for testing in real-time.

### Output Path Configuration
- The export metrics functionality saves the computed metrics as a `.txt` file. The default export directory is specified in the **`Config.h`** file located in:
`Source\util\include\Config.h`

- **Default Path**: 
  `C:\Users\Public\Documents`

  In this directory, the metrics files are saved under the `\PeakRMSCompressorWorkbench_testing_results` folder.

- **Compressed Files**:
  In `Config.h`, you can configure whether the compressed audio files should also be saved alongside the metrics. By default, this option is set to **false**.

To customize the output behavior, update the appropriate parameters in the **`Config.h`** file before running the application.

---

## Windows Setup

### Prerequisites

To set up and run this project, ensure you have the following installed:

1. [JUCE](https://juce.com/): A C++ framework for audio application development.
2. [AudioFilePlayerPlugin](https://github.com/hollance/AudioFilePlayerPlugin): Required for real-time audio compression playback.
3. [Visual Studio Community Edition 2022](https://visualstudio.microsoft.com/): The IDE for building the project.

---

### Step-by-Step Setup Guide
*(Based on [this video tutorial](https://youtu.be/Mo0Oco3Vimo?si=Sj4i6uZfiMJ6dL5S&t=1360))*

---

#### 1. Clone the Repository
```bash
git clone https://github.com/s1m0ness/PeakRMSCompressorWorkbench.git
```

---

#### 2. Run Setup Script
Run the `setup.sh` script located in the repository root to ensure `Config.h` stays local and doesn't get committed accidentally:
```bash
./setup.sh
```

---

#### 3. Install Visual Studio 2022
*(Video reference: [2:13](https://youtu.be/Mo0Oco3Vimo?si=KT-B_rA4bZAtaKIj&t=1397))*
- Download and install **[Visual Studio Community Edition 2022](https://visualstudio.microsoft.com/)**.
- During installation, select the **Desktop Development with C++** workload.

---

#### 4. Install JUCE Framework
*(Video reference: [27:32](https://youtu.be/Mo0Oco3Vimo?si=JDXoQBc6IURfOlis&t=1652))*
- Clone the JUCE repository:
  ```bash
  git clone https://github.com/juce-framework/JUCE.git
  ```
- Navigate to the Projucer build directory:
  ```plaintext
  JUCE\extras\Projucer\Builds\VisualStudio2022
  ```
- Open `Projucer.sln` in Visual Studio and build it.

---

#### 5. Set Up AudioFilePlayerPlugin and JUCE GLobal Paths
*(Video reference: [42:06](https://youtu.be/Mo0Oco3Vimo?si=l5YVJTxdnHX2p-Gb&t=2526))*
- Clone the AudioFilePlayerPlugin repository:
  ```bash
  git clone https://github.com/hollance/AudioFilePlayerPlugin.git
  ```
- Navigate to the directory:
  ```plaintext
  AudioFilePlayerPlugin\
  ```
- Right-click on `AudioFilePlayer.jucer` and select **Open in Program** and navigate to:
  ```plaintext
  JUCE\extras\Projucer
  ```
  and select `Projucer.jucer` file.
- Once the project opens in Projucer, go to **File > Global Paths > JUCE Modules** and select this directory:
- ```plaintext
  JUCE\modules
  ```
- in Projucer, click Visual Studio to export and build the solution.
---

#### 6. Set Up AudioPluginHost
*(Video reference: [35:32](https://youtu.be/Mo0Oco3Vimo?si=WoisnAayUc8t_EST&t=2132))*
- Navigate to the JUCE AudioPluginHost directory:
  ```plaintext
  JUCE\extras\AudioPluginHost
  ```
- Open `AudioPluginHost.jucer` in Projucer, click Visual Studio to export and build the solution.

---

#### 7. Configure the PeakRMSCompressorWorkbench Project
*(Video reference: [36:06](https://youtu.be/Mo0Oco3Vimo?si=rt7h6I_aILSQKOo4&t=2166))*
- Navigate to your `PeakRMSCompressorWorkbench` directory.
- Open `PeakRMSCompressorWorkbench.projucer` in Projucer.
- In Visual Studio:
  - Right-click on `PeakRMSCompressorWorkbench_VST3` in Solution Explorer and select **Properties**.
  - Go to **Configuration Properties > Debugging**.
  - Set the **Command** field to point to:
    ```plaintext
    JUCE\extras\AudioPluginHost\Builds\VisualStudio2022\x64\Debug\App\AudioPluginHost.exe
    ```
  - Click **Apply**.

---

#### 8. Modify VST3 Permissions
*(Video reference: [37:36](https://youtu.be/Mo0Oco3Vimo?si=sD74MpbJAyOmU3MD&t=2256))*
- Navigate to the following directory:
  ```plaintext
  C:\Program Files\Common Files
  ```
- Right-click the `VST3` folder (create it if doesn't exist), select **Properties**, then go to the **Security** tab.
- Select **Users** and allow modification permissions. Click **Apply**.

---

#### 9. Test the Project
*(Video reference: [38:49](https://youtu.be/Mo0Oco3Vimo?si=4A2KMZ622XDUdusK&t=2329))*
- Set `PeakRMSCompressorWorkbench_VST3` as the startup project in Visual Studio.
- Build and run the project.
- In **AudioPluginHost**:
  - Go to **Options > Edit the list of Available Plug-ins > Options > Scan for new or updated VST3 plug-ins**.
  - Add `AudioFilePlayer` and `PeakRMSCompressorWorkbench` VSTs to the host.
  - Save the configuration as a filter graph file:
    ```plaintext
    PeakRMSCompressorWorkbench\PeakRMSCompressorWorkbench.filtergraph
    ```

---

## Notes
- Make sure the **AudioPluginHost.exe** path is correctly set in the project properties.
- If you encounter issues with VST scanning, ensure that the VST3 folder permissions are correctly configured.

---

With this setup, you’re ready to test the **PeakRMSCompressorWorkbench** application and explore dynamic range compression metrics and real-time playback.
