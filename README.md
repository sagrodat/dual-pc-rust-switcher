# Dual-PC Rust Gaming Setup

A Windows application for seamlessly switching between two Rust game instances across two PCs using Input Director.

## Overview

This program creates a transparent overlay window and manages focus switching between two PCs running Rust. When the keybind is pressed, it switches control to the other PC via Input Director while maintaining smooth gameplay.

## How It Works

1. **Director PC**: Pressing END key shows overlay window, steals focus from Rust, and transfers control to Client PC
2. **Client PC**: Pressing END key sends signal back to Director, hides overlay, and returns control
3. UDP signals ensure proper window state synchronization between machines

## Requirements

- Windows OS (both PCs)
- [Input Director](https://www.inputdirector.com/) v2.3 or higher (CLI integration required)
- Both PCs on same local network
- Rust game instances running on both PCs

## Input Director Setup

**CRITICAL: Both systems must be configured as Directors with mutual client connections:**

1. Install Input Director on both PCs
2. On PC1: Configure as Director and add PC2 as a Client
3. On PC2: Configure as Director and add PC1 as a Client
4. **The hotkey configured in Input Director must NOT overlap with any hotkeys in Rust or this software**
5. Recommended: Use a different hotkey in Input Director than the END key used by this program

## Important: Rust Settings

**Set target FPS in Rust background to prevent lag:**

In Rust console (F1):

```
fps.limit 60
```

This ensures the game maintains smooth preview on the inactive PC while you're controlling the other machine.

## Configuration

Edit `config.h` with your network settings:

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

// Show/hide console window
const int CONSOLE_VISIBILITY = SW_SHOW;  // SW_HIDE or SW_SHOW

const int WINDOW_OPACITY = 255;  // 0-255, 0=invisible, 255=opaque
const char* WINDOW_TEXT = "USING 2nd PC";  // Text to display

#endif
```

## Build

Requires Windows SDK and Winsock2 library.

```bash
g++ main.cpp network.cpp utils.cpp -o program.exe -lws2_32 -lgdi32 -mwindows
```

## Usage

1. Configure hostnames and IPs in `config.h`
2. Build and run on both PCs
3. Launch Rust on both machines
4. Press **END** key to switch between PCs

## Utility: Get Virtual Key Code

The included `get_key_code.cpp` helps identify Windows virtual key codes:

```bash
g++ get_key_code.cpp -o get_key_code.exe
```

Run it and press keys to see their VK codes. Useful for changing the keybind (currently VK_END).

## Customization

- **Change keybind**: Replace `VK_END` in main.cpp with desired virtual key code
- **Window appearance**: Modify `WINDOW_TEXT` and `WINDOW_OPACITY` in config.h
- **Console visibility**: Set `CONSOLE_VISIBILITY` to `SW_HIDE` to hide console or `SW_SHOW` to show it
- **Network port**: Change `SIGNAL_PORT` if 9999 conflicts with other applications

## Security Note

Local network IP addresses (192.168.x.x) are private and only accessible within your network. They cannot be accessed from the internet.

## License

MIT
