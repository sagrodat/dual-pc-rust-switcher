// utils.cpp
#include "utils.h"

std::string GetHostname() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "";
}

void ForceFocus(HWND hwnd) {
    HWND foreground = GetForegroundWindow();
    DWORD foregroundThread = GetWindowThreadProcessId(foreground, NULL);
    DWORD currentThread = GetCurrentThreadId();
    
    AttachThreadInput(currentThread, foregroundThread, TRUE);
    
    BringWindowToTop(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    SetActiveWindow(hwnd);
    
    AttachThreadInput(currentThread, foregroundThread, FALSE);
}