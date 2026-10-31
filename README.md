# BinaryInspector

BinaryInspector is a cross-platform desktop application for inspecting and analyzing binary files.

**Built with:**
- C++20
- Qt 6 Widgets
- CMake

BinaryInspector provides a hexadecimal viewer, ASCII preview, byte-pattern search with direct navigation, string extraction, file analysis, SHA-256 calculation, Shannon entropy analysis, file comparison, and an extensible plugin system.

> **Note:** BinaryInspector is currently a read-only inspection tool. It does not modify the original files.

## Features

### Hex Viewer

- Open arbitrary binary files of any size.
- Display 64-bit file offsets.
- View hexadecimal byte values and synchronized ASCII representations.
- Efficient navigation through large files using on-demand chunk reads.
- Select bytes to inspect their offsets and data representations.
- Multi-byte selection and visual highlighting.

### Search Engine & Navigation

- ASCII and UTF-8 text search.
- Hexadecimal byte-pattern search.
- Wildcard support (`??`) in hex patterns.
- Chunk-based searching with overlap handling across chunk boundaries.
- **Direct Navigation:** Automatically jumps and scrolls the viewer to the found match.
- **Full Match Highlighting:** Visually highlights matching bytes across both HEX and ASCII columns.
- **Find Next (`F3`):** Navigate through subsequent occurrences without reopening the search dialog.
- **Go to Offset (`Ctrl+G`):** Jump to any file location using decimal or hexadecimal offsets.

Example HEX pattern:

```text
4D 5A 90 00
```

Example with wildcards:

```text
4D 5A ?? ?? 00
```

### String Extraction

- Extract printable ASCII strings.
- Configurable minimum string length.
- Display file offsets for every extracted string.

### Binary Analysis

- File size calculation and formatted human-readable output.
- SHA-256 cryptographic hash computation.
- Shannon entropy calculation (bits per byte).
- Complete byte frequency distribution histogram.
- Common file signature (magic bytes) detection.

Currently recognized signatures include:

- PE / DOS (`MZ`)
- ELF executables
- ZIP / DOCX / XLSX / JAR
- PNG images
- GIF images
- JPEG images
- PDF documents
- SQLite databases

### Byte Interpretation

Real-time interpretation of selected bytes:

- UINT16 (Little-Endian / Big-Endian)
- UINT32 (Little-Endian / Big-Endian)
- UINT64 (Little-Endian / Big-Endian)
- Raw hexadecimal string representation

### File Comparison

- Compare sizes between two files.
- Compare SHA-256 cryptographic hashes.
- Identify whether two files are identical or different.

### Plugin System

- Extensible Qt-based plugin architecture.
- Load custom analyzers dynamically from the `plugins/` directory.
- Extend BinaryInspector without modifying or rebuilding the core executable.

## Requirements

### Build Requirements

- CMake 3.21 or newer.
- A C++20-compatible compiler.
- Qt 6.2 or newer.
- Qt Core module.
- Qt Widgets module.

### Supported Toolchains

**Windows (primary configuration):**
- Microsoft Visual Studio 2022 with MSVC 2022 x64.
- Qt 6 MSVC 2022 64-bit.

**Alternative Windows toolchain:**
- MinGW-w64 with C++20 support.
- Matching Qt 6 MinGW 64-bit package.

**Linux:**
- GCC or Clang with C++20 support.
- Qt 6 development packages.

### Supported Platforms

- Windows 10/11.
- Linux.

> **Important:** The Qt package must match the compiler used to build the application. MSVC and MinGW builds are not binary compatible with each other.

## Building on Windows

### Prerequisites

Install:

- Visual Studio 2022 with the "Desktop development with C++" workload.
- CMake 3.21 or newer.
- Qt 6 with the matching MSVC kit.

The primary development configuration uses:

```text
Qt 6.12.0
MSVC 2022 64-bit
```

### Automated Clean Build

The project includes a PowerShell script that automates the complete Release build and deployment process.

Configure the Qt installation path in `clean-build.ps1` if necessary:

```powershell
$QtPath = "C:\Qt\6.12.0\msvc2022_64"
```

Run the script from the project root:

```powershell
.\clean-build.ps1
```

If PowerShell blocks script execution:

```powershell
powershell -ExecutionPolicy Bypass -File .\clean-build.ps1
```

The script performs the following operations:

1. Removes previous `build` and `dist` directories.
2. Configures CMake using Visual Studio 2022 x64.
3. Builds the Release configuration.
4. Installs the executable into `dist/bin`.
5. Runs `windeployqt` to copy the required Qt libraries and plugins.
6. Deploys the MSVC runtime when required.
7. Reports whether the build and deployment completed successfully.

### Manual Build

**Visual Studio 2022 (MSVC):**

Configure:

```powershell
cmake -S . -B build `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DCMAKE_PREFIX_PATH="C:\Qt\6.12.0\msvc2022_64"
```

Build:

```powershell
cmake --build build --config Release --parallel
```

Install:

```powershell
cmake --install build --config Release --prefix dist
```

Deploy Qt dependencies:

```powershell
C:\Qt\6.12.0\msvc2022_64\bin\windeployqt.exe `
    --release `
    --compiler-runtime `
    .\dist\bin\BinaryInspector.exe
```

Run:

```powershell
.\dist\bin\BinaryInspector.exe
```

### Alternative Windows Toolchains

MinGW and Ninja can also be used with matching Qt installations, but the automated `clean-build.ps1` script is configured specifically for MSVC 2022 x64.

For these alternative toolchains, configure the corresponding CMake generator and Qt installation manually.

## Building on Linux

### Ubuntu / Debian

Install dependencies:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev
```

Configure:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Build:

```bash
cmake --build build --parallel
```

Run:

```bash
./build/BinaryInspector
```

### Fedora

Install dependencies:

```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel
```

Configure and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run:

```bash
./build/BinaryInspector
```

## Deployment on Windows

The automated build script generates a distributable application directory:

```text
dist/
└── bin/
    ├── BinaryInspector.exe
    ├── Qt6Core.dll
    ├── Qt6Gui.dll
    ├── Qt6Widgets.dll
    ├── platforms/
    │   └── qwindows.dll
    └── ...
```

The actual DLLs and plugin directories depend on the Qt modules and plugins detected by `windeployqt`.

The entire `dist/bin` directory can be copied to another compatible Windows x64 machine.

**Important:**

- Do not distribute only `BinaryInspector.exe`.
- Keep the required Qt DLLs alongside the executable.
- Preserve the Qt plugin directory structure.
- The target machine may require the appropriate Microsoft Visual C++ Redistributable.
- Build and deployment must use compatible MSVC and Qt architectures.

The application does not require Qt Creator on the target machine.

## Usage

1. **Open a File:** Select `File -> Open...` (`Ctrl+O`) and choose a binary file.
2. **Inspect Bytes:** Scroll through hexadecimal values and ASCII characters. Click any byte to inspect its offset and numeric interpretations.
3. **Search Patterns:** Select `Search -> Find pattern...` (`Ctrl+F`). Choose ASCII or HEX mode and enter the pattern.
4. **Find Next Match:** Press `F3` or select `Search -> Find next`.
5. **Jump to Offset:** Select `Search -> Go to offset...` (`Ctrl+G`) to navigate using hexadecimal or decimal notation.
6. **File Analysis:** Select `Analyze -> File analysis` to view file metadata, size, SHA-256 hash, entropy, signatures, and byte distribution.
7. **Extract Strings:** Select `Analyze -> Extract strings` to extract printable ASCII strings with their starting offsets.
8. **Compare Files:** Select `File -> Compare with...` to compare file sizes and SHA-256 hashes against another binary file.
9. **Execute Plugins:** Select `Analyze -> Run plugins` to run analyzers located in the `plugins/` directory.

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+O` | Open binary file |
| `Ctrl+F` | Find pattern (ASCII or HEX) |
| `F3` | Find next occurrence |
| `Ctrl+G` | Go to offset |
| `Ctrl+Q` | Exit application |

## Plugin Development

BinaryInspector provides a Qt plugin interface:

```text
src/plugins/IAnalyzerPlugin.h
```

Plugins must implement the `IAnalyzerPlugin` interface:

```cpp
QString name() const;

QString analyze(const QString& filePath);
```

Plugins must declare standard Qt plugin metadata:

- `Q_PLUGIN_METADATA`
- `Q_INTERFACES`

Compile the plugin as a Qt-compatible shared library (`.dll` on Windows, `.so` on Linux) and place it inside:

```text
plugins/
```

The `plugins/` directory must be located alongside the `BinaryInspector` executable.

> **Security warning:** Only load plugins from trusted sources. Plugins execute native code with the permissions of the current user.

## Project Structure

```text
BinaryInspector/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── clean-build.ps1
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── FileReader.h
│   │   ├── FileReader.cpp
│   │   ├── Analysis.h
│   │   └── Analysis.cpp
│   ├── search/
│   │   ├── PatternSearch.h
│   │   └── PatternSearch.cpp
│   ├── ui/
│   │   ├── HexView.h
│   │   ├── HexView.cpp
│   │   ├── MainWindow.h
│   │   └── MainWindow.cpp
│   └── plugins/
│       ├── IAnalyzerPlugin.h
│       ├── PluginManager.h
│       └── PluginManager.cpp
├── build/
│   └── ... (generated build files)
└── dist/
    └── bin/
        └── ... (deployed application)
```

The `build/` and `dist/` directories are generated automatically and should generally not be committed to version control.

## Technical Notes

- The Hex Viewer reads on-demand slices from disk instead of buffering entire files in memory.
- Search and string extraction use chunk-based buffered streams with overlap buffers to catch patterns across chunk boundaries.
- String extraction currently scans printable ASCII sequences.
- Signature analysis uses heuristic magic byte pattern matching.
- File comparison checks file sizes and computes SHA-256 hashes.

## Future Improvements

- In-place hex editing and patching.
- Undo/redo support.
- Signed integer (INT8, INT16, INT32, INT64) and floating-point (FLOAT, DOUBLE) interpretation.
- UTF-8 and UTF-16 string extraction.
- Memory-mapped file access (`QFile::map`).
- Background threaded analysis using QThread / QtConcurrent with progress and cancellation.
- Graphical entropy visualization.
- Byte-by-byte visual diff viewer.
- Bookmark system for saving points of interest.
- Binary structure and template parsing.

## License

See `LICENSE` for the project license.