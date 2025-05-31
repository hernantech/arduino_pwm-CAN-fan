# CAN Bus PWM Fan Controller  
**Using TCAN332DCNR CAN Transceiver with Arduino**  

This project demonstrates how to use an Arduino with a **TCAN332DCNR CAN transceiver** (or compatible) to monitor and control PWM fans via CAN bus, specifically for **Cascadia Motion motor controller** telemetry.  

---

## 📌 **Hardware Setup**  

### **Required Components**  
- **Arduino Board** (Uno, Nano, Mega, etc.)  
- **TCAN332DCNR CAN Transceiver** (or MCP2515 + TJA1050 combo)  
- **PWM Fan(s)** (4-pin, 12V)  
- **Breadboard & Jumper Wires**  
- **12V Power Supply** (for fans)  
- **120Ω Termination Resistor** (if needed for CAN bus)  

### 🔌 **Wiring Connections**  

| **TCAN332DCNR Pin** | **Arduino Pin** | **Notes** |
|---------------------|----------------|-----------|
| `VCC` (5V)         | `5V`           | Power     |
| `GND`              | `GND`          | Ground    |
| `TXD`              | `D11` (MOSI)   | SPI Data Out |
| `RXD`              | `D12` (MISO)   | SPI Data In |
| `SCK`              | `D13` (SCK)    | SPI Clock |
| `CS`               | `D10`          | Chip Select (Configurable) |
| `INT` (Optional)   | `D2`           | Interrupt (Optional) |

#### **PWM Fan Connections**  
- **Fan PWM (Control)** → `D3` (or any PWM-capable pin)  
- **Fan Tach (RPM Feedback)** → `D5` (for interrupts)  
- **Fan +12V** → External 12V supply  
- **Fan GND** → Common ground  

---

## 📡 **Software Setup**  

### **Required Libraries**  
1. **SPI** (Built-in)  
2. **MCP_CAN** (Install via Arduino Library Manager)  

### **Installation**  
1. Download the `.ino` file from this repository.  
2. Open in **Arduino IDE**.  
3. Install `MCP_CAN` via:  
   - **Sketch → Include Library → Manage Libraries → Search "MCP_CAN" → Install**  

---

## 🚀 **Usage**  

### **1. CAN Bus Monitoring**  
The code automatically decodes:  
- **Temperatures** (Motor, Modules, RTDs)  
- **Voltages & Currents** (DC Bus, Phase Currents)  
- **Motor Speed & Torque**  
- **Fault Codes** (POST & Run Faults)  

**Output Example:**  
```
ID: 0x0A0 Len: 8  
Temperatures #1:  
Module A: 42.5 C  
Module B: 41.2 C  
Module C: 43.0 C  
Gate Driver: 38.7 C  
```

### **2. PWM Fan Control**  
The code adjusts fan speed based on **motor temperature** (default: `0x0A0` message).  

#### **Customization Options**  
- **Set Fan Speed Logic** (Modify in `loop()`):  
  ```cpp
  float motor_temp = decode_temperature(data); // From CAN message
  int fan_speed = map(motor_temp, 30, 80, 50, 255); // 30°C=50%, 80°C=100%
  analogWrite(FAN_PWM_PIN, fan_speed);
  ```
- **Manual Override** (Add a potentiometer or Serial input control).  

---

## ⚠️ **Troubleshooting**  

| **Issue** | **Solution** |
|-----------|--------------|
| **No CAN messages received** | Check wiring, termination resistor, and baud rate (`CAN_500KBPS`). |
| **Fans not spinning** | Verify PWM pin and 12V power supply. |
| **Garbage data** | Ensure correct `BASE_ID` (default `0x0A0` for Cascadia). |
| **CAN bus errors** | Use **120Ω termination resistors** at both ends of the bus. |

---

## 📜 **License**  
MIT License - Free for personal and commercial use.  

---

## 🔗 **References**  
- [TCAN332DCNR Datasheet (TI)](https://www.ti.com/lit/ds/symlink/tcan332.pdf)  
- [MCP_CAN Library Docs](https://github.com/coryjfowler/MCP_CAN_lib)  

---

🎉 **Happy CAN Bus Fan Tuning!** 🎉
