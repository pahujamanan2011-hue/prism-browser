# Prism Browser
A hyper-lightweight, secure, and keyboard-driven web browser built from scratch in C using SDL2, libcurl, Duktape (JS), and an integrated "Blackhole" ad-blocker.

```
### Linux (Ubuntu / Debian / Mint)
```bash
sudo apt update && sudo apt install build-essential cmake libsdl2-dev libsdl2-ttf-dev libcurl4-openssl-dev libssl-dev -y

```
### Linux (Fedora / RHEL)
```bash
sudo dnf groupinstall "Development Tools" -y && sudo dnf install cmake SDL2-devel SDL2_ttf-devel libcurl-devel openssl-devel -y

```
### Linux (Arch Linux)
```bash
sudo pacman -Syu base-devel cmake sdl2 sdl2_ttf curl openssl --noconfirm

```
### macOS (Using Homebrew)
```bash
brew install cmake sdl2 sdl2_ttf curl openssl

```
## 🛠️ Step 2: Download, Build, and Run
Now, copy and paste these commands **one by one** into your terminal to download the project and run it.
### For Windows Users (Inside MSYS2 UCRT64 Terminal):
```bash
# 1. Download the project code from GitHub
git clone https://github.com/pahujamanan2011-hue/prism-browser.git

# 2. Go inside the downloaded folder
cd prism-browser

# 3. Create a clean build folder
mkdir build && cd build

# 4. Configure the project setup
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# 5. Compile the browser code into an app
mingw32-make

# 6. Open and run your new browser!
./prism-browser.exe

```
### For Linux & macOS Users:
```bash
# 1. Download the project code from GitHub
git clone https://github.com/YOUR_USERNAME/prism-browser.git

# 2. Go inside the downloaded folder
cd prism-browser

# 3. Create a clean build folder
mkdir build && cd build

# 4. Configure the project setup
cmake -DCMAKE_BUILD_TYPE=Release ..

# 5. Compile the browser code into an app
make -j$(nproc)

# 6. Open and run your new browser!
./prism-browser

```
## 🎮 Browser Controls
| Keyboard Shortcut | Action |
|---|---|
| **Ctrl + O** | Focus / Unfocus URL Bar (Click here to type) |
| **Enter** | Load typed URL (When URL bar is active) |
| **Escape** | Cancel URL typing |
| **Ctrl + R** | Refresh / Reload the current page |
| **Alt + Left Arrow** | Go back to the last page |
| **Alt + Right Arrow** | Go forward to the next page |
| **Up / Down Arrow** | Scroll up or down line-by-line |
| **PageUp / PageDown** | Scroll up or down by half a screen |
| **Home / End** | Jump to the very top / very bottom of the page |
| **Ctrl + Q** | Safely close the browser and clean memory |
