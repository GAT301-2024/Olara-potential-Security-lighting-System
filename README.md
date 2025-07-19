# Secure ESP32 Lighting Control System

![Project Banner](https://theorycircuit.com/wp-content/uploads/2024/10/1728826310728.jpg)

A secure WiFi-controlled lighting system using ESP32 with password protection, brute-force lockout, and captive portal authentication.

## ✨ Features
- 🔒 Password-protected web interface
- 📶 Standalone WiFi AP (no internet required)
- ⚡ Control 3 LEDs (Red/Green/Orange)
- 🛡️ Brute-force protection (5 attempts lockout)
- 💾 Persistent credential storage
- 📱 Mobile-responsive dashboard

## 🛠️ Hardware Requirements
| Component | Quantity |
|-----------|----------|
| ESP32 Dev Board | 1 |
| Red LED | 1 |
| Green LED | 1 |
| Orange LED | 1 |
| 220Ω Resistors | 3 |
| Breadboard | 1 |
| Jumper Wires | As needed |

## 🚀 Installation

### Prerequisites
1. **Install Git** ([Download](https://git-scm.com/downloads))
2. **Set up Arduino IDE** with ESP32:
   - Add ESP32 board URL:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Install `ESP32` via *Tools > Board > Boards Manager*
3. **Connect ESP32** via USB (install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) if needed)

### Clone & Run
```bash
git clone https://github.com/fthnath/secure-lighting-control.git
cd secure-lighting-control

---

### **Troubleshooting Tips** 
```markdown
❌ **"Board not detected"?**  
- Check USB cable (some charge-only cables don’t transfer data)  
- Install correct drivers (CP210x/CH340)  

❌ **Compilation errors?**  
- Ensure all libraries are installed  
- Use Arduino IDE 2.x or newer  
