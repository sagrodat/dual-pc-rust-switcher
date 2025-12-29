# Dual-PC Rust Gaming Setup with Input Director

A Windows application that solves the full-screen game mouse control issue when using Input Director KVM software across two PCs running Rust.

## The Problem This Solves

Many software KVMs don't handle mouse movement properly in full-screen games. Input Director handles KVM switching perfectly, but when controlling the client PC, mouse delta (dx/dy movement) still applies to the host PC's full-screen applications - causing movement.

## Demonstration

![Demo](output.gif)

As you can see in the demonstration:

- **Without this program**: Using Input Director alone, mouse movement still affects the PC with keyboard and mouse connected, causing unintended character movement in Rust. For example, if you sprint somewhere on PC1 (using a keybind) and switch to PC2 to do something else, your character on PC1 goes off target and turns due to mouse movement on PC2.
- **With this program**: In the right bottom corner, a tiny black window appears saying "USING 2nd PC", and mouse movement is no longer passed to the game

## The Solution

When the hotkey is pressed:

1. A small overlay window pops up on top of the screen on PC1
2. This pushes Rust to the background, preventing mouse movement from affecting the game
3. Input Director switches control to PC2
4. You can still see PC1's Rust game through the overlay (set appropriate background FPS in Rust settings)
5. When switching back, PC2 sends a signal over WiFi to PC1 to close the overlay and refocus on Rust

## How It Works

1. **Director PC (PC1)**: Pressing END key shows overlay window, steals focus from Rust, and transfers control to Client PC
2. **Client PC (PC2)**: Pressing END key sends UDP signal back to Director, hides overlay, and returns control
3. UDP signals ensure proper window state synchronization between machines
4. Input Director handles the actual mouse/keyboard switching between PCs

## Requirements

- Windows OS (both PCs)
- [Input Director](https://www.inputdirector.com/) installed on both PCs (version 2.3+ required)
- Both PCs on same local network
- Rust game instances running on both PCs

## Input Director Setup

**CRITICAL: Both systems must be configured as Directors with mutual client connections:**

1. Install Input Director on both PCs
2. On PC1: Configure as Director and add PC2 as a Client
3. On PC2: Configure as Director and add PC1 as a Client
4. **The hotkey configured in Input Director must NOT overlap with any hotkeys in Rust or this software**
5. Recommended: Use a different hotkey in Input Director than the END key used by this program or change END hotkey in config.h

## Important: Rust Settings

**Set background FPS in Rust to prevent lag on the inactive PC:**

This ensures the game maintains smooth preview on PC1 while you're controlling PC2.

## Configuration

Edit `config.h` with your network and Input Director settings:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <Windows.h>

const std::string PC1_HOSTNAME = "YOUR-PC1-NAME";  // Director PC
const std::string PC2_HOSTNAME = "YOUR-PC2-NAME";  // Client PC
const std::string PC1_IP = "192.168.0.100";         // Director PC IP
const std::string PC2_IP = "192.168.0.101";         // Client PC IP
const int SIGNAL_PORT = 9999;

// Path to Input Director executable
const std::string INPUT_DIRECTOR_PATH = "C:\\Program Files\\Input Director\\inputdirector.exe";

// Show/hide console window
const int CONSOLE_VISIBILITY = SW_SHOW;  // SW_HIDE or SW_SHOW

const int WINDOW_OPACITY = 255;  // 0-255, 0=invisible, 255=opaque
const char* WINDOW_TEXT = "USING 2nd PC";  // Text to display

#endif
```

## Build

Requires Windows SDK and Winsock2 library.

```bash
g++ main.cpp network.cpp utils.cpp -o program.exe -static -lws2_32 -lgdi32 -mwindows -static-libgcc -static-libstdc++
```

## Usage

1. Configure hostnames, IPs, and Input Director path in `config.h`
2. Build and run on both PCs
3. Launch Rust on both machines
4. Press **END** key to switch between PCs

The overlay window can be made invisible by setting `WINDOW_OPACITY` to 0 in config.h.

## Utility: Get Virtual Key Code

The included `get_key_code.cpp` helps identify Windows virtual key codes:

```bash
g++ get_key_code.cpp -o get_key_code.exe
```

Run it and press keys to see their VK codes. Useful for changing the keybind (currently VK_END).

## Customization

- **Change keybind**: Replace `VK_END` in main.cpp with desired virtual key code
- **Window appearance**: Modify `WINDOW_TEXT` and `WINDOW_OPACITY` in config.h
  - Set `WINDOW_OPACITY` to 0 for completely invisible overlay
- **Console visibility**: Set `CONSOLE_VISIBILITY` to `SW_HIDE` to hide console or `SW_SHOW` to show it
- **Network port**: Change `SIGNAL_PORT` if 9999 conflicts with other applications
- **Input Director path**: Update `INPUT_DIRECTOR_PATH` if installed in non-default location

## License

MIT
