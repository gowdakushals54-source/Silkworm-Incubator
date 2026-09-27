#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// --- Configuration ---
#define DHTPIN 4              // DHT Sensor Data Pin (GPIO 4)
#define GAS_SENSOR_PIN 34     // Gas Sensor Analog Pin (GPIO 34)
#define FAN_RELAY_PIN 16      // Relay Channel 1 for Fan (GPIO 16)
#define ALARM_RELAY_PIN 17    // Relay Channel 2 for Buzzer/LED (GPIO 17)
#define DHTTYPE DHT11         // (Change to DHT22 if using that model)

// --- Thresholds (Set based on your environment) ---
const float TEMP_THRESHOLD_C = 26.6; 
// **CALIBRATE THIS!** Use Serial Monitor to find your 'normal' gas reading.
// A temporary, known-high value to ensure the fan switches on for testing.
const int GAS_THRESHOLD = 1500; 

// --- Initialize Components ---
DHT dht(DHTPIN, DHTTYPE);
// Common I2C Address is 0x27 or 0x3F. (Address, columns, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
  Serial.begin(115200);
  
  // Set Relay Pins as Outputs
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(ALARM_RELAY_PIN, OUTPUT);
  
  // *Initialize OFF (HIGH is OFF for Active-LOW Relays)*
  digitalWrite(FAN_RELAY_PIN, HIGH); 
  digitalWrite(ALARM_RELAY_PIN, HIGH); 

  // Initialize Sensors and Display
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.print("Ventilation System");
  lcd.setCursor(0, 1);
  lcd.print("Ready!     ");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read Sensor Data
  float t = dht.readTemperature(); // Celsius
  int gasValue = analogRead(GAS_SENSOR_PIN);
  
  // Check for read failures
  if (isnan(t)) {
    Serial.println("Error: Failed to read from DHT sensor!");
    lcd.clear();
    lcd.print("T/H Read Error!");
    delay(1000);
    return;
  }
  
  // --- Control Logic Check ---
  bool alarmTriggered = false;
  String triggerReason = "NONE";
  
  // Condition 1: Temperature Check
  if (t > TEMP_THRESHOLD_C) {
    alarmTriggered = true;
    triggerReason = "TEMP HIGH";
  }
  
  // Condition 2: Gas Check (if not already triggered by temp)
  if (gasValue > GAS_THRESHOLD) {
    alarmTriggered = true;
    if (triggerReason == "NONE") {
      triggerReason = "TOXIC GAS";
    } else {
      triggerReason = "BOTH";
    }
  }
  
  // --- Actuator Control ---
  // LOW = ON, HIGH = OFF
  if (alarmTriggered) {
    // Turn FAN & ALARM ON 
    digitalWrite(FAN_RELAY_PIN, LOW);
    digitalWrite(ALARM_RELAY_PIN, LOW);
  } else {
    // Turn FAN & ALARM OFF
    digitalWrite(FAN_RELAY_PIN, HIGH);
    digitalWrite(ALARM_RELAY_PIN, HIGH);
  }
  
  // --- Display and Serial Output ---
  Serial.print("T: "); Serial.print(t); Serial.print("*C | Gas: "); Serial.println(gasValue);

  // Line 1: T and Gas Value
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print((char)223); // Degree symbol
  lcd.print("C Gas:");
  lcd.print(gasValue);
  lcd.print("    "); 

  // Line 2: Status
  lcd.setCursor(0, 1);
  if (alarmTriggered) {
    lcd.print("ALARM: ");
    lcd.print(triggerReason);
  } else {
    lcd.print("Status: Normal  ");
  }

  delay(2000); 
}