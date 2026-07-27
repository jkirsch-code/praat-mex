# Building praatMEX from Source

This guide covers building the `praatmex` MEX file on Windows, Linux, and macOS.

## Prerequisites

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| **MATLAB** | R2023a | Tested up to R2026a. MEX binary is version-locked. |
| **CMake** | 3.20 | 3.25+ recommended |
| **C++17 Compiler** | See platform | MSVC 2022 (Win), GCC ≥11 (Linux), Xcode CLI (macOS) |
| **Git** | Any | For fetching Praat source if not included |

---

## Windows (x64 / ARM64)

### Required
- **Visual Studio 2022** (Community/Professional/Enterprise) with "Desktop development with C++" workload
- **Windows 10/11 SDK** (included with VS)
- **MATLAB** with MEX support (default)

### Build Commands

**x64 (default):**
```powershell
cd +praatMEX
cmake -B build -DCMAKE_GENERATOR_PLATFORM=x64
cmake --build build --config Release
```

**ARM64:**
```powershell
cmake -B build -DCMAKE_GENERATOR_PLATFORM=ARM64
cmake --build build --config Release
```

### Output
```
build/Release/praatmex.mexw64
```

### Copy to Package Root
```powershell
copy build\Release\praatmex.mexw64 .
```

---

## Linux (x86_64 / ARM64)

### Required (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++-11 \
    matlab-support  # or install MATLAB separately
```

### Required (Fedora/RHEL)
```bash
sudo dnf install -y \
    cmake \
    gcc-c++ \
    matlab  # or install MATLAB separately
```

### Build Commands
```bash
cd +praatMEX
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-11
cmake --build build --config Release -j$(nproc)
```

### Output
```
build/praatmex.mexa64        # x86_64
build/praatmex.mexmaci64     # macOS Intel (if cross-compiling)
```

### Copy to Package Root
```bash
cp build/praatmex.mexa64 .
```

---

## macOS (Intel / Apple Silicon)

### Required
- **Xcode Command Line Tools**: `xcode-select --install`
- **MATLAB** for macOS (Intel or ARM native)
- **CMake**: `brew install cmake`

### Build Commands

**Apple Silicon (ARM64):**
```bash
cd +praatMEX
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

**Intel (x86_64) — requires Rosetta or Intel MATLAB:**
```bash
arch -x86_64 cmake -B build -DCMAKE_BUILD_TYPE=Release
arch -x86_64 cmake --build build --config Release
```

### Output
```
build/praatmex.mexmaca64     # Apple Silicon
build/praatmex.mexmaci64     # Intel
```

### Copy to Package Root
```bash
cp build/praatmex.mexmaca64 .
```

---

## Verifying the Build

### Quick Test (MATLAB)
```matlab
addpath('path/to/+praatMEX');
praatmex('init');  % Should return 1.0

[x, fs] = audioread('test_audio.wav');
ch1 = x(:,1); ch1 = ch1 / max(abs(ch1));

p = praatmex('pitch', ch1, fs, 'pitchMethod', 'ac', 'timeStep', 0.01);
fprintf('Pitch: %d frames\n', numel(p.freq));
```

### Full Test Suite
```matlab
run('test_praatmex_full.m');
```

Expected output: all algorithms report success with frame counts.

---

## Build Configuration Details

### CMake Options

| Variable | Default | Description |
|----------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` for debug symbols |
| `CMAKE_GENERATOR_PLATFORM` | auto | `x64`, `ARM64` (Windows) |
| `MATLAB_ROOT` | auto-detected | Override MATLAB install path |

### Compiler Flags (from CMakeLists.txt)

**MSVC:**
```
/permissive- /EHs /fp:precise /wd4068 /utf-8 /wd4554 /bigobj
```

**GCC/Clang:**
```
-std=c++17 -O3 -DNDEBUG -fPIC -Wall -Wextra
```

### Preprocessor Definitions
```
NO_GUI NO_GRAPHICS NO_NETWORK NOMINMAX UNICODE _UNICODE
_FILE_OFFSET_BITS=64 PLATFORM_WINDOWS FPM_64BIT
NO_OGG_VORBIS
```

---

## Troubleshooting

### "MATLAB not found"
Set `MATLAB_ROOT` explicitly:
```bash
cmake -B build -DMATLAB_ROOT="/usr/local/MATLAB/R2023a"
```
Windows:
```powershell
cmake -B build -DMATLAB_ROOT="C:/Program Files/MATLAB/R2023a"
```

### "Praat source missing"
The `praat/` directory must contain the patched Praat source. If you cloned without it:
```bash
git submodule update --init --recursive
# OR manually copy from a working build
```

### "Undefined reference to gsl_*"
The GSL stubs in `src/praatmex_gsl_stubs.cpp` must be compiled. Ensure:
```cmake
# In CMakeLists.txt - GSL sources are EXCLUDED, stubs are in src/
```

### "Memory error / crash"
- Try `Debug` build: `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
- Run with MATLAB's `-debug` flag
- Check `build_log*.txt` for previous build clues

### "MEX version mismatch"
Rebuild for your exact MATLAB version. MEX binaries are **not portable** across MATLAB versions.

---

## Continuous Integration (GitHub Actions)

See `.github/workflows/` for automated builds on:
- `windows-latest` (x64 + ARM64)
- `ubuntu-latest` (x86_64 + ARM64)
- `macos-latest` (Intel + Apple Silicon)

Artifacts: `.mexw64`, `.mexa64`, `.mexmaca64`, `.mexmaci64` attached to releases.

---

## File Size Reference

| Artifact | Size (approx) |
|----------|---------------|
| `praat.lib` (static) | ~96 MB |
| `praatmex.mexw64` | ~8.2 MB |
| Source (praat/) | ~200 MB |

---

## Support

- **Build logs**: `build_log*.txt` in `+praatMEX/`
- **CMake cache**: `build/CMakeCache.txt`
- **Issues**: GitHub Issues with `build` label