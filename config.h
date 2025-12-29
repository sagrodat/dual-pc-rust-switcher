// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <Windows.h>

const std::string PC1_HOSTNAME = "DESKTOP-V45G59E";  // Director PC
const std::string PC2_HOSTNAME = "DESKTOP-VKLG84S";  // Client PC
const std::string PC1_IP = "192.168.0.46";
const std::string PC2_IP = "192.168.0.66";
const int SIGNAL_PORT = 9999;

// Show/hide console window
const int CONSOLE_VISIBILITY = SW_SHOW;  // SW_HIDE or SW_SHOW

const int WINDOW_OPACITY = 255;  // 0-255, 0=invisible, 255=opaque
const char* WINDOW_TEXT = "USING RON";  // Text to display

#endif