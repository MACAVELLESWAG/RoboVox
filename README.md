# RoboVox — Lightweight MIDI-Controlled Robotic Vocal Synth

**Zero Suit Samus Precision Edition**  
Elite JUCE plugin crafted by juce-elite-weaver. Monophonic TTS instrument powered by espeak-ng.

## Vision
- **Intuitive & Simple**: Type text. Draw monophonic MIDI in your DAW piano roll. Hear robotic vocals pitched to your notes.
- **Lightweight**: espeak-ng backend — tiny footprint, lightning fast synthesis, no heavy ML models.
- **Robotic Vocal Character**: Perfect for chiptune, cyberpunk, experimental, game audio, or signature robotic leads.
- **Monophonic**: Last-note priority with clean re-trigger on new notes. Ideal for melodic phrasing via piano roll.

## How It Works (Intuitive Workflow)
1. Load RoboVox in your DAW (VST3 / AU / Standalone).
2. **Enter text** in the large input box (phrases, lyrics snippets, commands — keep it punchy for best results).
3. **Draw monophonic MIDI** in the piano roll (one note at a time recommended).
4. On each Note-On: RoboVox instantly re-synthesizes the text at the exact pitch of that MIDI note and plays it back.
5. Tweak **Speed**, **Pitch Offset**, **Gain**, and **Robotic Intensity** (affects pitch range/expression) live.
6. Result: Your text "sings" robotically across the melody you draw. Re-triggering on every note gives that classic talking-synth performative feel.

Perfect for short-to-medium phrases. For very long text, it restarts cleanly on each note — punchy and musical.

## Tech Stack (Elite Choices)
- **JUCE 7/8** (modern CMake, APVTS, real-time safe)
- **espeak-ng** (open source, lightweight, C API, pitch/speed/voice controllable, built-in robotic character)
- Real-time safe audio thread (zero blocking calls, lock-free handoff for synthesized buffers)
- Worker thread for synthesis (non-blocking, fast <10-50ms typical)
- Premium minimal UI: dark sci-fi theme, large text editor, tactile controls, instant feedback

## Build Instructions

### Prerequisites

**Linux (easiest)**:
```bash
sudo apt update
sudo apt install espeak-ng espeak-ng-data espeak-ng-dev pkg-config
```

**Windows (detailed below)**:
You need:
- Visual Studio 2022 Community (or newer) with **"Desktop development with C++"** workload + CMake tools
- Git for Windows
- CMake (usually comes with VS)

### Building on Windows — Step by Step (Samus Precision)

#### Step 1: Build espeak-ng from source (required)

1. Open **Developer Command Prompt for VS 2022** (important — use the x64 Native one).
2. Run:
   ```cmd
   git clone https://github.com/espeak-ng/espeak-ng.git
   cd espeak-ng
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
   cmake --build . --config Release
   ```
3. After build, note these two paths:
   - **Include dir**: `espeak-ng\src\include` (contains `espeak-ng\speak_lib.h`)
   - **Library**: `espeak-ng\build\src\Release\espeak-ng.lib` + `espeak-ng.dll`

   You will need the `.dll` next to your plugin later for runtime.

#### Step 2: Build RoboVox

1. Open the same **Developer Command Prompt**.
2. Navigate to the RoboVox folder.
3. Configure with the path to your espeak-ng folder:

   ```cmd
   mkdir build && cd build
   cmake .. -DESPEAK_ROOT=C:\path\to\espeak-ng -DCMAKE_BUILD_TYPE=Release
   ```

   Example:
   ```cmd
   cmake .. -DESPEAK_ROOT=C:\Users\YourName\espeak-ng -DCMAKE_BUILD_TYPE=Release
   ```

4. Build:
   ```cmd
   cmake --build . --config Release
   ```

5. The built plugin will be in:
   `build\RoboVox_artefacts\Release\`

   - `RoboVox.vst3`
   - `RoboVox.exe` (Standalone — easiest for testing)

#### Step 3: Runtime (Important)

Copy `espeak-ng.dll` from your espeak-ng build into the same folder as the plugin (or next to the Standalone .exe).

Also copy the entire `espeak-ng-data` folder from the espeak-ng source into the same directory as the plugin (or ensure it's in PATH / working dir). Without the data files, synthesis will fail silently or crash.

For VST3 in your DAW, place the DLL + data folder next to the .vst3 or in a known location.

#### Alternative (Advanced): Use vcpkg

If you have vcpkg:
```cmd
vcpkg install espeak-ng:x64-windows
```
Then configure with:
```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```
This can simplify linking but you still need to handle the data files.

### Build the Plugin (General / Linux reference)
1. Clone or download this folder.
2. Configure & build (Linux example):
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release -j$(nproc)
   ```

The plugins appear in `build/RoboVox_artefacts/`

- **Standalone** → best for quick testing with virtual MIDI keyboard
- **VST3** for your DAW

### Usage Tips (Pro Workflow)
- Keep text concise for rhythmic, punchy robotic delivery.
- Use piano roll to "play" the voice like a melodic instrument.
- Automate Speed/Pitch/Gain for evolving robotic performances.
- Try different voices via future UI expansion (en, en-us, etc.).
- Monophonic mode shines in piano roll — avoid chords for clean results.

## Architecture (Elite Standard)
See detailed breakdown in the source. Clean separation:
- `PluginProcessor`: MIDI, APVTS, audio mixing, state
- `TTSynthesizer` (Core): espeak-ng wrapper, worker thread, lock-free buffer handoff, pitch mapping, resampling
- `PluginEditor`: Simple, logical, high-DPI ready layout with custom dark LookAndFeel
- Real-time guarantees enforced. No allocations or espeak calls on audio thread.

## Future Elite Iterations (Roadmap)
- Phoneme-level stepping for true singing synthesis feel (advance words/syllables per note)
- Embedded espeak-ng data for portable builds
- More voices + MBROLA diphone support (higher quality)
- Visual waveform/ level scope + MIDI activity LEDs
- Preset system with factory robotic phrases

This is **release-ready quality** foundation. No compromises.

**Samus out.** Precision delivered. Now go make it sing.

---
Built under juce-elite-weaver protocol. Exceptional results only.