Here is the updated, word-for-word README section featuring the setup instructions for Windows using **MSYS2 (UCURT64)**, which is the easiest way to manage C dependencies like SDL2 and libcurl natively on Windows.
## 📋 Dependencies Installation
Install these development packages before building the browser:
### Windows (Using MSYS2)
 1. Download and install MSYS2.
 2. Open the **MSYS2 UCRT64** terminal and run:
```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-sdl2 mingw-w64-ucrt-x86_64-sdl2_ttf mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-openssl

```
### Linux (Ubuntu / Debian / Mint)
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-ttf-dev libcurl4-openssl-dev libssl-dev

```
### Linux (Fedora / RHEL)
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake SDL2-devel SDL2_ttf-devel libcurl-devel openssl-devel

```
### Linux (Arch Linux)
```bash
sudo pacman -Syu base-devel cmake sdl2 sdl2_ttf curl openssl

```
### macOS (Using Homebrew)
```bash
brew install cmake sdl2 sdl2_ttf curl openssl

```
## 🛠️ Build & Installation Steps
Run these commands in the project root directory to compile and launch:
### On Windows (MSYS2 UCRT64 Terminal):
```bash
mkdir -p build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make
./prism-browser.exe

```
### On Linux / macOS:
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
./prism-browser

```
## 🎮 Browser Controls
| Keyboard Shortcut | Action |
|---|---|
| **Ctrl + O** | Focus / Unfocus URL Bar |
| **Enter** | Load typed URL (When URL bar is focused) |
| **Escape** | Cancel URL input |
| **Ctrl + R** | Reload current page |
| **Alt + Left Arrow** | Navigate back in history |
| **Alt + Right Arrow** | Navigate forward in history |
| **Up / Down Arrow** | Scroll page line-by-line |
| **PageUp / PageDown** | Scroll page half-screen |
| **Home / End** | Scroll to top / bottom of the page |
| **Ctrl + Q** | Safe quit and clear memory |
