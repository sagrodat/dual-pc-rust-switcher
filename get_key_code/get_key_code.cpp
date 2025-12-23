#include <windows.h>
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Key Code Detector ===\n";
    std::cout << "Press any key to see its VK code\n";
    std::cout << "Press ESC to exit\n\n";
    
    bool keyStates[256] = {false};
    
    while (true) {
        // Check ESC to exit
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            std::cout << "\nExiting...\n";
            break;
        }
        
        // Check all virtual key codes
        for (int vk = 0; vk < 256; vk++) {
            bool isPressed = (GetAsyncKeyState(vk) & 0x8000) != 0;
            
            // Detect rising edge (key just pressed)
            if (isPressed && !keyStates[vk]) {
                std::cout << "Key pressed: VK = 0x" 
                         << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << vk
                         << " (" << std::dec << vk << ")\n";
            }
            
            keyStates[vk] = isPressed;
        }
        
        Sleep(10);
    }
    
    return 0;
}