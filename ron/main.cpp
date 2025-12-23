// main.cpp
#include <Windows.h>
#include <iostream>
#include "config.h"
#include "network.h"
#include "utils.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lparam);
}

int main() {
    std::string hostname = GetHostname();
    bool isDirector = (hostname == PC1_HOSTNAME);
    bool isClient = (hostname == PC2_HOSTNAME);
    std::cout << "Running on: " << hostname << (isDirector ? " (Director)" : " (Client)") << std::endl;

    SOCKET listener = INVALID_SOCKET;
    if(isDirector) {
        listener = SetupListener(SIGNAL_PORT);
        std::cout << "UDP listener started on port " << SIGNAL_PORT << std::endl;
    }

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OverlayWindow";
    wc.hbrBackground = CreateSolidBrush(RGB(255,0,0));
    RegisterClass(&wc);

    HWND hwnd = NULL;
    HWND previousWindow = NULL; 
    bool isToggled = false;

    if(isDirector) {
        POINT pt;
        GetCursorPos(&pt); 
        HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

        MONITORINFO mi = {};
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);

        int width = mi.rcMonitor.right - mi.rcMonitor.left;
        int height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        int window_width = 50;
        int window_height = 50;

        hwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_APPWINDOW,
            "OverlayWindow",
            "Overlay",
            WS_POPUP,
            width-window_width,
            height-window_height,
            window_width,
            window_height,
            NULL, NULL, wc.hInstance, NULL
        );
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        ShowWindow(hwnd, SW_HIDE);
    }

    while (true) 
    {
        if(isDirector && listener != INVALID_SOCKET && hwnd && isToggled) {
            if(ReceivedSignal(listener)) {
                ShowWindow(hwnd, SW_HIDE);
                if(previousWindow && IsWindow(previousWindow)) {
                    ForceFocus(previousWindow);
                }
                isToggled = false;
                previousWindow = NULL;
                std::cout << "Signal received - window hidden!" << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_END) & 0x0001) 
        {
            if(isDirector) 
            {
                std::cout << "Director: Toggling" << std::endl;
                isToggled = !isToggled;
               
                previousWindow = GetForegroundWindow();
        
                ShowWindow(hwnd, SW_SHOW);
                ForceFocus(hwnd);
                std::cout << "Window shown! Focus stolen!" << std::endl;

                std::string cmd = "-switchControlToClient:" + PC2_IP;
                ShellExecuteA(NULL, "open", 
                    "C:\\Program Files\\Input Director\\IDCmd.exe",
                    cmd.c_str(),
                    NULL, SW_HIDE);
                std::cout << "Switched to client" << std::endl;
            }
            else if (isClient) 
            {
                SendSignal(PC1_IP.c_str(), SIGNAL_PORT);
                std::cout << "Signal sent to director" << std::endl;
                
                std::string cmd = "-switchControlToClient:" + PC1_IP;
                ShellExecuteA(NULL, "open", 
                    "C:\\Program Files\\Input Director\\IDCmd.exe",
                    cmd.c_str(),
                    NULL, SW_HIDE);
                std::cout << "Switched to director" << std::endl;
            }
            else
            {
                std::cout << "UNKNOWN MACHINE" << std::endl;
            }
        }
        
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Sleep(10);
    }

    return 0;
}