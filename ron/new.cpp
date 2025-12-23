#include <Windows.h>
#include <iostream>
#include <string>

const std::string PC1_HOSTNAME = "DESKTOP-V45G59E";  // Director PC
const std::string PC2_HOSTNAME = "DESKTOP-VKLG84S";  // Client PC
const std::string PC1_IP = "192.168.0.46"; // Director IP
const std::string PC2_IP = "192.168.0.66"; // Client IP

// Window procedure - handles window messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lparam);
}

// Get current PC hostname
std::string GetHostname() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "";
}

void SendKeyPress(WORD vkCode) {
    INPUT inputs[2] = {};
    
    // Key down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vkCode;
    inputs[0].ki.dwFlags = 0;
    
    // Key up
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    
    SendInput(2, inputs, sizeof(INPUT));
}

// Helper function to force focus
void ForceFocus(HWND hwnd) {
    // Attach to foreground thread
    HWND foreground = GetForegroundWindow();
    DWORD foregroundThread = GetWindowThreadProcessId(foreground, NULL);
    DWORD currentThread = GetCurrentThreadId();
    
    AttachThreadInput(currentThread, foregroundThread, TRUE);
    
    // Aggressive focus stealing
    BringWindowToTop(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    SetActiveWindow(hwnd);
    
    AttachThreadInput(currentThread, foregroundThread, FALSE);
}

int main() {
    // Register window class - defines window properties
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;                           // Function to handle messages
    wc.hInstance = GetModuleHandle(NULL);               // Current program instance
    wc.lpszClassName = "OverlayWindow";                 // Window class name
    wc.hbrBackground = CreateSolidBrush(RGB(255,0,0)); // White background
    RegisterClass(&wc);

    HWND hwnd = NULL;      // Window handle
    HWND previousWindow = NULL; 
    bool isToggled = false; // Track visibility state

    // Auto-detect which PC we're on
    std::string hostname = GetHostname();
    bool isDirector = (hostname == PC1_HOSTNAME);
    bool isClient = (hostname == PC2_HOSTNAME);
    std::cout << "Running on: " << hostname << (isDirector ? " (Director)" : " (Client)") << std::endl;

    while (true) 
    {
        // side button pressed
        if (GetAsyncKeyState(VK_END) & 0x0001) 
        {
            if(isDirector) 
            {
                // DIRECTOR: Toggle window + switch
                std::cout << "Director: Window is now " << isToggled << std::endl;
                isToggled = !isToggled;
               
                // Save current focused window
                previousWindow = GetForegroundWindow();

                // On which monitor is the mouse?
                POINT pt;
                GetCursorPos(&pt); 
                HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

                // Get that monitors info
                MONITORINFO mi = {};
                mi.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(hMonitor, &mi);

                // Get Monitor dimensions
                int width = mi.rcMonitor.right - mi.rcMonitor.left;
                int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

                // Window size
                int window_width = 50; // px
                int window_height = 50; //px

                // Create topmost window 
                hwnd = CreateWindowEx(
                    WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_APPWINDOW,
                    "OverlayWindow",
                    "Overlay",
                    WS_POPUP | WS_VISIBLE,
                    width-window_width,
                    height-window_height,
                    window_width,
                    window_height,
                    NULL, NULL, wc.hInstance, NULL
                );
                SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

                Sleep(50);
                ForceFocus(hwnd);
                std::cout << "Window drawn! Focus stolen!" << std::endl;

                // Switch to client
                std::string cmd = "-switchControlToClient:" + PC2_IP;
                ShellExecuteA(NULL, "open", 
                    "C:\\Program Files\\Input Director\\IDCmd.exe",
                    cmd.c_str(),
                    NULL, SW_HIDE);
                std::cout << "Switched to client" << std::endl;
                
                // else // Toggle off
                // {
                //     if(hwnd)
                //     {
                //         DestroyWindow(hwnd);
                //         hwnd = NULL;
                        
                //         // Restore focus
                //         if(previousWindow && IsWindow(previousWindow)) 
                //         {
                //             ForceFocus(previousWindow);
                //         }
                //         previousWindow = NULL;
                //         std::cout << "Switched back to local" << std::endl;
                //     }
                // }
            }
            else if (isClient) 
            {
                //Switch to director (client on 2nd pc)
                std::string cmd = "-switchControlToClient:" + PC1_IP;
                ShellExecuteA(NULL, "open", 
                    "C:\\Program Files\\Input Director\\IDCmd.exe",
                    cmd.c_str(),
                    NULL, SW_HIDE);
                std::cout << "Switched to director" << std::endl;
            }
            else
            {
                std::cout<<"UNKNOWN MACHINE"<<std::endl;
            }
        }
        
        // Process window messages
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