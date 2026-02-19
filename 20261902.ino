#include "DFRobot_DF2301Q.h"
#include <Wire.h>

// Relay pin for Fan
#define RELAY_PIN 7

// Relay logic - CHANGE THESE if fan behaves opposite
#define RELAY_ON LOW    // Try HIGH if fan turns on immediately
#define RELAY_OFF HIGH  // Try LOW if fan turns on immediately

// Create I2C voice module object
DFRobot_DF2301Q_I2C voiceModule;

bool fanState = false;
uint8_t currentVolume = 5;
bool isListening = false;  // Track if module is awake

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  Serial.println("==================================");
  Serial.println("   VOICE FAN CONTROL - I2C MODE");
  Serial.println("==================================");
  
  // Initialize relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  fanState = false;
  Serial.println("✓ Relay initialized (pin 7)");
  
  // Initialize I2C
  Wire.begin();
  delay(100);
  
  Serial.println("\nInitializing I2C Voice Module...");
  
  if (voiceModule.begin()) {
    Serial.println("✓ Voice module connected!");
    
    // Configure module
    voiceModule.setVolume(currentVolume);
    voiceModule.setMuteMode(false);
    voiceModule.setWakeTime(15);  // 15 seconds wake time
    
    Serial.print("Volume: ");
    Serial.println(currentVolume);
    
  } else {
    Serial.println("✗ Voice module not found!");
    Serial.println("Check SDA→A4, SCL→A5 connections");
    while(1);
  }
  
  Serial.println("\n✓ SYSTEM READY!");
  Serial.println("WAKE WORDS (don't control fan):");
  Serial.println("  'Hello Robot'");
  Serial.println("  'Hi Lexin'");
  Serial.println("COMMANDS (after wake word):");
  Serial.println("  'Turn on the fan'");
  Serial.println("  'Turn off the fan'");
  Serial.println("\nType 1=ON, 0=OFF in Serial Monitor");
  Serial.println("==================================");
}

void loop() {
  // Get voice command
  uint8_t commandID = voiceModule.getCMDID();
  
  if (commandID != 0) {
    Serial.println();
    Serial.print("Command ID received: ");
    Serial.println(commandID);
    
    // Process the command
    processVoiceCommand(commandID);
    
    // Small delay to prevent multiple triggers
    delay(1000);
  }
  
  // Check for serial commands
  if (Serial.available()) {
    processSerialCommand();
  }
  
  delay(50);
}

void processVoiceCommand(uint8_t cmdID) {
  // FIRST: Handle wake words (should NOT control fan)
  if (cmdID == 1 || cmdID == 2 || cmdID == 3 || cmdID == 4 || cmdID == 5) {
    // These are common wake word IDs
    Serial.println("Wake word detected!");
    Serial.println("Module is now listening...");
    isListening = true;
    return;  // IMPORTANT: Return here, don't control fan!
  }
  
  // Handle actual commands (only if we heard a wake word first)
  if (!isListening) {
    Serial.println("Say a wake word first ('Hello Robot' or 'Hi Lexin')");
    return;
  }
  
  // NOW process fan control commands
  if (cmdID == 11) {  // Common ID for "Turn on the fan"
    Serial.println("Command: 'Turn on the fan'");
    if (!fanState) {
      digitalWrite(RELAY_PIN, RELAY_ON);
      fanState = true;
      Serial.println("✓ Fan turned ON");
    } else {
      Serial.println("⚠ Fan already ON");
    }
  }
  
  else if (cmdID == 12) {  // Common ID for "Turn off the fan"
    Serial.println("Command: 'Turn off the fan'");
    if (fanState) {
      digitalWrite(RELAY_PIN, RELAY_OFF);
      fanState = false;
      Serial.println("✓ Fan turned OFF");
    } else {
      Serial.println("⚠ Fan already OFF");
    }
  }
  
  else if (cmdID == 13) {  // Common ID for "Volume up"
    Serial.println("Command: 'Volume up'");
    if (currentVolume < 7) {
      currentVolume++;
      voiceModule.setVolume(currentVolume);
      Serial.print("✓ Volume: ");
      Serial.println(currentVolume);
    } else {
      Serial.println("✗ Volume at maximum");
    }
  }
  
  else if (cmdID == 14) {  // Common ID for "Volume down"
    Serial.println("Command: 'Volume down'");
    if (currentVolume > 1) {
      currentVolume--;
      voiceModule.setVolume(currentVolume);
      Serial.print("✓ Volume: ");
      Serial.println(currentVolume);
    } else {
      Serial.println("✗ Volume at minimum");
    }
  }
  
  // Try alternative command IDs
  else if (cmdID == 75) {
    Serial.println("Command (75): 'Turn on the fan'");
    digitalWrite(RELAY_PIN, RELAY_ON);
    fanState = true;
    Serial.println("✓ Fan turned ON");
  }
  
  else if (cmdID == 76) {
    Serial.println("Command (76): 'Turn off the fan'");
    digitalWrite(RELAY_PIN, RELAY_OFF);
    fanState = false;
    Serial.println("✓ Fan turned OFF");
  }
  
  else if (cmdID == 100) {
    Serial.println("Command (100): 'Volume up'");
    if (currentVolume < 7) {
      currentVolume++;
      voiceModule.setVolume(currentVolume);
      Serial.print("✓ Volume: ");
      Serial.println(currentVolume);
    }
  }
  
  else if (cmdID == 101) {
    Serial.println("Command (101): 'Volume down'");
    if (currentVolume > 1) {
      currentVolume--;
      voiceModule.setVolume(currentVolume);
      Serial.print("✓ Volume: ");
      Serial.println(currentVolume);
    }
  }
  
  else {
    Serial.print("Unknown command ID: ");
    Serial.println(cmdID);
    Serial.println("Say a wake word first, then:");
    Serial.println("1. 'Turn on the fan'");
    Serial.println("2. 'Turn off the fan'");
    Serial.println("3. 'Volume up'");
    Serial.println("4. 'Volume down'");
  }
  
  // Reset listening state after processing a command
  isListening = false;
  Serial.print("Fan status: ");
  Serial.println(fanState ? "ON" : "OFF");
}

void processSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case '1':
      digitalWrite(RELAY_PIN, RELAY_ON);
      fanState = true;
      Serial.println("Serial: Fan ON");
      break;
      
    case '0':
      digitalWrite(RELAY_PIN, RELAY_OFF);
      fanState = false;
      Serial.println("Serial: Fan OFF");
      break;
      
    case 'S':
    case 's':
      Serial.println("\n=== STATUS ===");
      Serial.print("Fan: ");
      Serial.println(fanState ? "ON" : "OFF");
      Serial.print("Volume: ");
      Serial.println(currentVolume);
      Serial.print("Module listening: ");
      Serial.println(isListening ? "YES" : "NO (say wake word)");
      Serial.println("==============");
      break;
      
    case 'T':
    case 't':
      Serial.println("\nTesting: Say these and note IDs:");
      Serial.println("1. 'Hello Robot' (wake word)");
      Serial.println("2. 'Turn on the fan'");
      Serial.println("3. 'Turn off the fan'");
      Serial.println("4. 'Volume up'");
      Serial.println("5. 'Volume down'");
      break;
      
    case 'R':
    case 'r':
      isListening = false;
      Serial.println("Reset: Module listening state cleared");
      break;
      
    case '?':
      Serial.println("\nCommands:");
      Serial.println("1 = Fan ON");
      Serial.println("0 = Fan OFF");
      Serial.println("S = Status");
      Serial.println("T = Test instructions");
      Serial.println("R = Reset listening state");
      Serial.println("? = This help");
      break;
  }
}