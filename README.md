# 🌀 Wireless Fan Switch Controller — Dual Servo v2.0

A smart IoT solution to control your ceiling fan remotely using WiFi. Press buttons on a web interface to turn your fan ON/OFF automatically using servo motors.

---

## 📋 Table of Contents
1. [Quick Overview](#quick-overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Circuit Diagram](#circuit-diagram)
4. [Software Setup](#software-setup)
5. [Installation Guide](#installation-guide)
6. [Issues & Solutions](#issues--solutions)
7. [How to Use](#how-to-use)
8. [Troubleshooting](#troubleshooting)

---

## 🎯 Quick Overview

**What does this project do?**
- Allows you to control a physical fan switch from anywhere using your phone/computer
- Two servo motors physically press the ON and OFF buttons
- Creates a beautiful web dashboard to control everything
- Works on WiFi with a fixed IP address (no app needed!)

**Perfect for:**
- Smart home automation
- Lazy people 😄
- Home automation beginners
- Learning IoT projects

---

## 🛠️ Hardware Requirements

### Components You'll Need:

| Item | Quantity | Purpose |
|------|----------|---------|
| **NodeMCU ESP8266** | 1 | Brain of the project (WiFi microcontroller) |
| **SG90 Servo Motor** | 2 | Press fan switch buttons |
| **Micro USB Cable** | 1 | Power supply & programming |
| **Jumper Wires** | 8-10 | Connections |
| **Breadboard** (optional) | 1 | For neat wiring |
| **Power Bank or USB Adapter** | 1 | To power the NodeMCU |

### Total Cost: ~$20-30 USD

---

## 📐 Circuit Diagram

```
┌─────────────────────────────────────────┐
│          NodeMCU ESP8266                │
│  ┌───────────────────────────────────┐  │
│  │ GND  D1  D2  3V3  5V  GND  ...    │  │
│  └───┬──────┬───────┬────┬───────────┘  │
│      │      │       │    │              │
│      │      │       │    └──────────────┼─ USB Power (5V)
│      │      │       └──────────────────┼─ 3.3V to Servo power
│      │      │                          │
│      │      └──────┬─ D1 (GPIO5)       │
│      │             │                    │
│      │    ┌────────┴────────┐          │
│      │    │  Servo Motor 1   │          │
│      │    │  (ON Servo)      │          │
│      │    ├─ Signal → D1     │          │
│      │    ├─ Power → 5V      │          │
│      │    └─ GND → GND       │          │
│      │                        │          │
│      └──┬─ D2 (GPIO4)        │          │
│         │                     │          │
│         ┌────────┬────────┐   │          │
│         │  Servo Motor 2  │   │          │
│         │  (OFF Servo)    │   │          │
│         ├─ Signal → D2    │   │          │
│         ├─ Power → 5V ────┤   │          │
│         └─ GND → GND ─────────┼─────────┘
│
│  Physical Setup:
│  ┌─────────────────────────────────────┐
│  │  Ceiling Fan Switch                 │
│  │  ┌────────────────────────────────┐ │
│  │  │ [ON Button]  [OFF Button]      │ │
│  │  │    ▲              ▲            │ │
│  │  │    │              │            │ │
│  │  │  Servo1          Servo2        │ │
│  │  │ presses         presses        │ │
│  │  │   here            here         │ │
│  │  └────────────────────────────────┘ │
│  └─────────────────────────────────────┘
```

### Wire Connections Summary:

**Servo Motor 1 (ON Switch):**
- Signal → D1 (GPIO5)
- Power (Red) → 5V
- Ground (Brown) → GND

**Servo Motor 2 (OFF Switch):**
- Signal → D2 (GPIO4)
- Power (Red) → 5V
- Ground (Brown) → GND

**NodeMCU Power:**
- USB cable provides 5V power
- Powers both servos through USB port

---

## 💻 Software Setup

### Prerequisites (Before Starting):

1. **Install Arduino IDE or PlatformIO**
   - **Recommended:** PlatformIO (VS Code extension) — easier for beginners
   - **Alternative:** Arduino IDE

2. **Download Code:**
   - Clone or download the project files
   - Project location: `fan final\src\main.cpp`

3. **Required Libraries (Auto-installed):**
   - `ESP8266WiFi` — WiFi connectivity
   - `ESP8266WebServer` — Web server
   - `Servo` — Servo motor control

### Install PlatformIO (Recommended):

```bash
# 1. Open VS Code
# 2. Go to Extensions (Ctrl+Shift+X)
# 3. Search for "PlatformIO"
# 4. Click Install
# 5. Reload VS Code
```

---

## 📥 Installation Guide

### Step 1: Connect Hardware
```
1. Attach servo motors physically to fan switch buttons
   - Servo 1 should press the ON button
   - Servo 2 should press the OFF button

2. Connect servo signals:
   - Servo 1 Signal → D1
   - Servo 2 Signal → D2

3. Connect power:
   - Both servos Red wire → 5V
   - Both servos Brown wire → GND

4. Connect NodeMCU via USB to your computer
```

### Step 2: Configure WiFi Credentials
Edit `main.cpp` and change these lines (around line 20):

```cpp
const char* WIFI_SSID     = "Oxyzone";        // ← Change to your WiFi name
const char* WIFI_PASSWORD = "oxyzone@321";    // ← Change to your WiFi password
```

### Step 3: Calibrate Servo Angles (Optional)
The servos are pre-calibrated, but if they don't press buttons correctly, adjust:

```cpp
const int S1_NEUTRAL = 90;   // Servo 1 rest position
const int S1_PRESS   = 55;   // Servo 1 pressing position
const int S2_NEUTRAL = 90;   // Servo 2 rest position
const int S2_PRESS   = 125;  // Servo 2 pressing position
```

**How to calibrate:**
- Upload code
- Open Serial Monitor (115200 baud)
- Watch servo positions
- Adjust in 5-degree steps if needed

### Step 4: Upload Code

**Using PlatformIO:**
```bash
1. Open main.cpp in VS Code
2. Click: "Upload" button (bottom toolbar)
   OR use keyboard: Ctrl+Alt+U
3. Watch for success message
```

**Using Arduino IDE:**
```bash
1. File → Open → main.cpp
2. Tools → Board → NodeMCU 1.0
3. Tools → Port → Select COM port
4. Click Upload arrow
```

### Step 5: Find Your IP Address

After upload, open **Serial Monitor**:
- **Baud Rate:** 115200
- Look for: `IP  : 192.168.1.100`

Your device IP is now **fixed to 192.168.1.100**

---

## 🐛 Issues & Solutions

### Issue 1: IP Address Changed After Each Boot ❌

**Problem:** Every time the device rebooted, it got a new IP address (192.168.1.32, then 192.168.1.49, etc.)

**Root Cause:** DHCP (Dynamic Host Configuration Protocol) — the router assigns random IPs

**Solution Applied:** ✅
```cpp
// Set Static IP Address (added to main.cpp)
WiFi.config(IPAddress(192, 168, 1, 100), 
            IPAddress(192, 168, 1, 1), 
            IPAddress(255, 255, 255, 0));
```

**Now:** Device always has IP **192.168.1.100** — no more changes!

---

### Issue 2: Serial Communication Not Working ❌

**Problem:** Serial Monitor shows garbage characters

**Root Cause:** Wrong baud rate setting

**Solution:** ✅ Set baud rate to **115200** in Serial Monitor

```
Serial Monitor Settings:
- Baud Rate: 115200 ✓
- Data Bits: 8
- Stop Bits: 1
- Parity: None
```

---

### Issue 3: Upload Fails (COM Port Error) ❌

**Problem:** `Cannot configure port... device which does not exist`

**Root Cause:** 
- USB cable disconnected during upload
- NodeMCU reset/unplugged

**Solution:** ✅
```
1. Reconnect USB cable firmly
2. Wait 3 seconds
3. Try upload again
4. If still fails, restart PlatformIO/Arduino IDE
```

---

### Issue 4: Servo Doesn't Move ❌

**Problem:** Pressing buttons on web interface, but servo doesn't move

**Root Cause:**
- Wrong pin assignment
- Servo not powered
- Servo signal wire disconnected

**Solution:** ✅
```
1. Check USB power is connected (blue light on NodeMCU)
2. Check servo signal wire is in D1/D2
3. Check servo red/brown wires to 5V/GND
4. Open Serial Monitor and check for error messages
5. Try servo angles: set S1_PRESS to 0-180 range to find working position
```

---

### Issue 5: Can't Find Device on WiFi ❌

**Problem:** `http://192.168.1.100/` doesn't load

**Root Cause:**
- Device not connected to WiFi
- Wrong WiFi credentials
- Wrong IP address

**Solution:** ✅
```
1. Open Serial Monitor (115200 baud)
2. Look for: "Wi-Fi connected!" message
3. Read the IP address shown
4. If not connected:
   - Check WIFI_SSID in code
   - Check WIFI_PASSWORD in code
   - Make sure WiFi is on in your home
5. Try IP address from Serial Monitor output
```

---

## 🎮 How to Use

### Accessing the Web Interface:

1. **On same WiFi network as NodeMCU**
2. **Open browser and go to:**
   ```
   http://192.168.1.100/
   ```

3. **You'll see a beautiful dashboard with:**
   - Large fan icon (rotating when ON)
   - Status badge (ON/OFF)
   - Green "Press ON" button
   - Red "Press OFF" button
   - Command counter
   - Servo status indicators

### Controlling the Fan:

**To Turn ON:**
- Click green **"Press ON"** button
- Servo 1 presses fan's ON button
- Fan turns on
- Status updates on screen

**To Turn OFF:**
- Click red **"Press OFF"** button
- Servo 2 presses fan's OFF button
- Fan turns off
- Status updates on screen

### API Endpoints (Advanced Users):

```
http://192.168.1.100/on       → Turn fan ON
http://192.168.1.100/off      → Turn fan OFF
http://192.168.1.100/status   → Get fan status (JSON)
http://192.168.1.100/info     → Get device info (JSON)
```

**Example response:**
```json
{
  "fan_on": true,
  "busy": false
}
```

---

## 🔧 Troubleshooting

### Q: Dashboard loads but buttons don't work?
**A:** 
- Check Serial Monitor for errors
- Verify WiFi is connected (look for "Wi-Fi connected!" message)
- Try refreshing the page
- Check browser console for JavaScript errors (F12)

---

### Q: Servo moves but doesn't press button hard enough?
**A:**
- Open Serial Monitor (115200 baud)
- Look for servo press values: `[S1] Pressing ON -> XXdeg`
- Adjust angles (S1_PRESS or S2_PRESS) in code
- Upload again and test

---

### Q: Device disconnects from WiFi randomly?
**A:**
- Signal strength too weak (RSSI should be -70 or better)
- Move NodeMCU closer to router
- Check for WiFi interference
- USB power might be insufficient

---

### Q: How do I reset the device?
**A:**
```
1. Quick method: 
   - Unplug USB cable
   - Wait 3 seconds
   - Plug back in

2. Hard reset (clear WiFi):
   - Hold RESET button on NodeMCU
   - Wait 5 seconds
   - Release
```

---

### Q: How do I change the IP address?
**A:**
Edit `main.cpp` line ~570:
```cpp
// Change these numbers:
WiFi.config(IPAddress(192, 168, 1, 100),    // ← Change 100 to any number 1-254
            IPAddress(192, 168, 1, 1),
            IPAddress(255, 255, 255, 0));
```

---

### Q: Can I use this on 2.4GHz WiFi only?
**A:**
Yes! ESP8266 only supports 2.4GHz WiFi (not 5GHz)

---

## 📊 Project Specifications

| Feature | Detail |
|---------|--------|
| **Microcontroller** | ESP8266 (NodeMCU) |
| **WiFi Standard** | 802.11 b/g/n (2.4GHz) |
| **Servo Motors** | 2x SG90 (5-6V) |
| **Operating Voltage** | 5V USB |
| **IP Address** | 192.168.1.100 (Fixed) |
| **Port** | 80 (HTTP) |
| **Update Frequency** | 3 seconds (polling) |
| **Max Range** | Entire WiFi network |
| **Memory Usage** | ~30% RAM, ~31% Flash |

---

## 🎓 Learn More

### Understanding the Code:

1. **WiFi Connection** (lines 550-580)
   - Connects to WiFi
   - Sets static IP

2. **Servo Control** (lines 430-470)
   - Controls servo movements
   - Presses buttons

3. **Web Server** (lines 330-420)
   - Hosts the dashboard
   - Responds to commands

4. **Main Loop** (line 620+)
   - Handles incoming requests
   - Controls servos

---

## 📝 Development Notes

- **Last Updated:** May 2, 2026
- **Version:** 2.0 (Dual Servo)
- **Developer:** ILHAM FAROOQUE
- **GitHub:** https://github.com/ilham786
- **Instagram:** https://www.instagram.com/ilham_farooque/

---

## 📜 License & Credits

This project is open-source. Feel free to modify and share!

**Libraries Used:**
- ESP8266WiFi (Arduino core)
- ESP8266WebServer (Arduino core)
- Servo (Arduino standard library)

---

## 🚀 Next Steps / Future Improvements

- [ ] Add MQTT support for advanced automation
- [ ] Create mobile app for iOS/Android
- [ ] Add motion sensor for auto-on feature
- [ ] Implement schedule-based control
- [ ] Add voice control (Alexa/Google Home)
- [ ] Create data logging dashboard
- [ ] Add multiple fan support

---

## ❓ FAQ

**Q: Will this damage my fan?**
A: No! The servo only presses the button briefly (450ms), just like you would.

**Q: Can I use with any fan?**
A: Yes! Works with any fan that has a physical push-button switch.

**Q: What if WiFi goes down?**
A: Device will show "Connection error" in dashboard. Reconnect WiFi and refresh.

**Q: Can multiple people control it?**
A: Yes! Anyone on the same WiFi network can access the dashboard.

**Q: Is it secure?**
A: This version has no authentication. Add username/password for security if needed.

---

## 💬 Support

Having issues? Follow these steps:

1. **Check Serial Monitor output** (115200 baud)
2. **Review Troubleshooting section above**
3. **Check GitHub Issues** for similar problems
4. **Contact:** [Your contact info here]

---

**Happy Controlling! 🎉**

*"From clicking buttons to clicking buttons... but remotely!"* 😄
