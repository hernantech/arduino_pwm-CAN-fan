#include <SPI.h>
#include <mcp_can.h>

// CAN Configuration
#define CAN_CS_PIN    10   // Chip Select pin for MCP2515
#define CAN_INT_PIN   2    // Interrupt pin (optional)
#define BITRATE       CAN_500KBPS  // 500 kbps
#define BASE_ID       0x0A0  // Base ID for motor controller
#define FAULT_ID      0x0AB   // Fault message ID

MCP_CAN CAN(CAN_CS_PIN);  // CAN bus instance

// Decoding Functions
float decode_temperature(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 10.0f;
}

float decode_high_voltage(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 10.0f;
}

float decode_low_voltage(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 100.0f;
}

float decode_current(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 10.0f;
}

float decode_torque(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 10.0f;
}

int16_t decode_angular_velocity(uint8_t* data) {
  return (data[1] << 8) | data[0];
}

bool decode_boolean(uint8_t* data) {
  return data[0] > 0;
}

float decode_frequency(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 10.0f;
}

float decode_power(uint8_t* data) {
  int16_t value = (data[1] << 8) | data[0];
  return value / 10.0f;
}

// State Description Functions
const char* get_vsm_state_description(uint8_t state) {
  switch(state) {
    case 0: return "VSM Start State";
    case 1: return "Pre-charge Init State";
    case 2: return "Pre-charge Active State";
    case 3: return "Pre-charge Complete State";
    case 4: return "VSM Wait State";
    case 5: return "VSM Ready State";
    case 6: return "Motor Running State";
    case 7: return "Blink Fault Code State";
    case 14: return "Shutdown in Process";
    case 15: return "Recycle Power State";
    default: return "Unknown state";
  }
}

const char* get_inverter_state_description(uint8_t state) {
  switch(state) {
    case 0: return "Power on State";
    case 1: return "Stop State";
    case 2: return "Open Loop State";
    case 3: return "Closed Loop State";
    case 4: return "Wait State";
    case 8: return "Idle Run State";
    case 9: return "Idle Stop State";
    default: return "Unknown state";
  }
}

// Fault Code Processing
void decode_fault_codes(uint8_t* data, uint16_t* faults) {
  faults[0] = (data[1] << 8) | data[0]; // POST Fault Lo
  faults[1] = (data[3] << 8) | data[2]; // POST Fault Hi
  faults[2] = (data[5] << 8) | data[4]; // Run Fault Lo
  faults[3] = (data[7] << 8) | data[6]; // Run Fault Hi
}

void print_fault_descriptions(uint16_t fault_lo, uint16_t fault_hi, bool is_post) {
  const char* fault_names[] = {
    "Hardware Gate/Desaturation Fault", "HW Over-current Fault",
    "Accelerator Shorted", "Accelerator Open", "Current Sensor Low",
    "Current Sensor High", "Module Temperature Low", "Module Temperature High",
    "Control PCB Temperature Low", "Control PCB Temperature High",
    "Gate Drive PCB Temperature Low", "Gate Drive PCB Temperature High",
    "5V Sense Voltage Low", "5V Sense Voltage High", "12V Sense Voltage Low",
    "12V Sense Voltage High", "2.5V Sense Voltage Low", "2.5V Sense Voltage High",
    "1.5V Sense Voltage Low", "1.5V Sense Voltage High", "DC Bus Voltage High",
    "DC Bus Voltage Low", "Pre-charge Timeout", "Pre-charge Voltage Failure",
    "EEPROM Checksum Invalid", "EEPROM Data Out of Range", "EEPROM Update Required",
    "Hardware DC Bus Over-Voltage", "Gate Driver Initialization", "Reserved",
    "Brake Shorted", "Brake Open"
  };
  
  for (int i = 0; i < 32; i++) {
    uint16_t mask = (i < 16) ? (1 << i) : (1 << (i - 16));
    uint16_t word = (i < 16) ? fault_lo : fault_hi;
    if (word & mask) {
      Serial.print("  - ");
      Serial.println(fault_names[i]);
    }
  }
}

// Message Parsing Function
void parse_cascadia_message(uint32_t id, uint8_t len, uint8_t* data) {
  uint8_t msg_type = id - BASE_ID;
  
  if (id == FAULT_ID) {
    uint16_t faults[4];
    decode_fault_codes(data, faults);
    Serial.println("Fault Codes:");
    Serial.print("POST: Lo=0x"); Serial.print(faults[0], HEX);
    Serial.print(" Hi=0x"); Serial.println(faults[1], HEX);
    Serial.print("RUN: Lo=0x"); Serial.print(faults[2], HEX);
    Serial.print(" Hi=0x"); Serial.println(faults[3], HEX);
    
    Serial.println("POST Faults:");
    print_fault_descriptions(faults[0], faults[1], true);
    Serial.println("Run Faults:");
    print_fault_descriptions(faults[2], faults[3], false);
    return;
  }

  switch(msg_type) {
    case 0x00: { // Temperatures #1
      Serial.println("Temperatures #1:");
      Serial.print("Module A: "); Serial.print(decode_temperature(data)); Serial.println(" C");
      Serial.print("Module B: "); Serial.print(decode_temperature(data+2)); Serial.println(" C");
      Serial.print("Module C: "); Serial.print(decode_temperature(data+4)); Serial.println(" C");
      Serial.print("Gate Driver: "); Serial.print(decode_temperature(data+6)); Serial.println(" C");
      break;
    }
    case 0x01: { // Temperatures #2
      Serial.println("Temperatures #2:");
      Serial.print("Control Board: "); Serial.print(decode_temperature(data)); Serial.println(" C");
      Serial.print("RTD1: "); Serial.print(decode_temperature(data+2)); Serial.println(" C");
      Serial.print("RTD2: "); Serial.print(decode_temperature(data+4)); Serial.println(" C");
      Serial.print("RTD3: "); Serial.print(decode_temperature(data+6)); Serial.println(" C");
      break;
    }
    case 0x02: { // Temperatures #3 & Torque Shudder
      Serial.println("Temperatures #3:");
      Serial.print("Coolant: "); Serial.print(decode_temperature(data)); Serial.println(" C");
      Serial.print("Hotspot: "); Serial.print(decode_temperature(data+2)); Serial.println(" C");
      Serial.print("Motor: "); Serial.print(decode_temperature(data+4)); Serial.println(" C");
      Serial.print("Torque Shudder: "); Serial.print(decode_torque(data+6)); Serial.println(" Nm");
      break;
    }
    case 0x07: { // Voltage Information
      Serial.println("Voltages:");
      Serial.print("DC Bus: "); Serial.print(decode_high_voltage(data)); Serial.println(" V");
      Serial.print("Output: "); Serial.print(decode_high_voltage(data+2)); Serial.println(" V");
      Serial.print("VAB/Vd: "); Serial.print(decode_high_voltage(data+4)); Serial.println(" V");
      Serial.print("VBC/Vq: "); Serial.print(decode_high_voltage(data+6)); Serial.println(" V");
      break;
    }
    case 0x06: { // Current Information
      Serial.println("Currents:");
      Serial.print("Phase A: "); Serial.print(decode_current(data)); Serial.println(" A");
      Serial.print("Phase B: "); Serial.print(decode_current(data+2)); Serial.println(" A");
      Serial.print("Phase C: "); Serial.print(decode_current(data+4)); Serial.println(" A");
      Serial.print("DC Bus: "); Serial.print(decode_current(data+6)); Serial.println(" A");
      break;
    }
    case 0x05: { // Motor Position
      Serial.println("Motor Position:");
      Serial.print("Angle: "); Serial.print(decode_temperature(data)); Serial.println(" deg");
      Serial.print("Speed: "); Serial.print(decode_angular_velocity(data+2)); Serial.println(" RPM");
      Serial.print("Freq: "); Serial.print(decode_frequency(data+4)); Serial.println(" Hz");
      Serial.print("Delta Resolver: "); Serial.print(decode_temperature(data+6)); Serial.println(" deg");
      break;
    }
    case 0x0C: { // Torque & Timer
      Serial.println("Torque & Timer:");
      Serial.print("Cmd Torque: "); Serial.print(decode_torque(data)); Serial.println(" Nm");
      Serial.print("Feedback Torque: "); Serial.print(decode_torque(data+2)); Serial.println(" Nm");
      uint32_t timer = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];
      Serial.print("Power Timer: "); Serial.print(timer * 0.003); Serial.println(" sec");
      break;
    }
    case 0x0A: { // Internal States
      Serial.println("Internal States:");
      Serial.print("VSM State: "); Serial.println(get_vsm_state_description(data[0]));
      Serial.print("PWM Freq: "); Serial.print(data[1]); Serial.println(" kHz");
      Serial.print("Inverter State: "); Serial.println(get_inverter_state_description(data[2]));
      Serial.print("Relay State: 0x"); Serial.println(data[3], HEX);
      // Additional state parsing would go here
      break;
    }
    default:
      Serial.print("Unhandled message type: 0x");
      Serial.println(msg_type, HEX);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port
  
  // Initialize CAN bus
  if (CAN.begin(MCP_ANY, BITRATE, MCP_16MHZ) == CAN_OK) {
    Serial.println("CAN Initialized");
    CAN.setMode(MCP_NORMAL); // Set normal mode
  } else {
    Serial.println("CAN Initialization Failed");
    while (1); // Halt on failure
  }
}

void loop() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    uint32_t id;
    uint8_t len = 0;
    uint8_t buf[8];
    
    // Read message
    CAN.readMsgBuf(&id, &len, buf);
    
    Serial.print("\nID: 0x"); Serial.print(id, HEX);
    Serial.print(" Len: "); Serial.println(len);
    
    // Parse valid messages
    if ((id >= BASE_ID && id <= BASE_ID + 0x2F) || id == FAULT_ID) {
      parse_cascadia_message(id, len, buf);
    }
  }
}
