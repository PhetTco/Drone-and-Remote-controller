#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const uint64_t pipeOut = 0xE9E8F0F0E1LL;   //The same as in the receiver 0xE9E8F0F0E1LL 
RF24 radio(9, 10); // select CE,CSN pin for NRF4L01 Module
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Signal {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  volatile byte aux1; // Declare as volatile for use in interupt function
};
Signal data;

// Aux1 interrupt pin
const int aux1Pin = 2;

void ResetData() {
  //Center of analog JOYSTICK
  data.throttle = 0; // Motor Stop (254/2=127) (Signal lost position )
  data.pitch = 127;  // Center (Signal lost position)
  data.roll = 127;   // Center (Signal lost position)
  data.yaw = 127;    // Center (Signal lost position)
  data.aux1 = 0; 
}

void aux1ISR() {
  // Update aux1 value in the ISR
  data.aux1 = digitalRead(aux1Pin);
}

void setup() {
  // Start everything up
  radio.begin();
  radio.setChannel(108);
  radio.openWritingPipe(pipeOut);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);    // More stable communication with the lowest data rate.
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening(); // Start the radio communication for Transmitter
  Serial.begin(115200); // Start serial for debugging
  Wire.begin();
  lcd.begin(16, 2);  // Initialize the LCD
  lcd.backlight();
  ResetData();

  // Check if nRF24L01 is connected
  if (!radio.isChipConnected()) {
    Serial.println("nRF24L01 module not connected!");
  } else {
    Serial.println("nRF24L01 module is connected.");
  }

  radio.printDetails(); 

  // Set up the aux1Pin as an input and attach an interrupt
  pinMode(aux1Pin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(aux1Pin), aux1ISR, CHANGE); // Interrupt on state change (RISING or FALLING also possible)
}

// Joystick center and its borders
int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return (reverse ? 255 - val : val);
}

void loop() {
  // Control Stick Calibration
  data.throttle = mapJoystickValues(analogRead(A0), 0, 512, 1023, false);
  data.roll = mapJoystickValues(analogRead(A1), 0, 512, 1023, false);
  data.pitch = mapJoystickValues(analogRead(A2), 0, 512, 1023, false);
  data.yaw = mapJoystickValues(analogRead(A3), 0, 512, 1023, false;

  if (radio.write(&data, sizeof(Signal))) {
    // Print the data if sent successfully
    Serial.print("Data sent successfully - Throttle: ");
    Serial.print(data.throttle);
    Serial.print(", Roll: ");
    Serial.print(data.roll);
    Serial.print(", Pitch: ");
    Serial.print(data.pitch);
    Serial.print(", Yaw: ");
    Serial.println(data.yaw);
    Serial.print(", Aux1: ");
    Serial.println(data.aux1);
    lcd.setCursor(0, 0);
    lcd.print("Sent Success");
    lcd.setCursor(0, 1);
    lcd.print(data.throttle);
    lcd.print(",");
    lcd.print(data.roll);
    lcd.print(",");
    lcd.print(data.pitch);
    lcd.print(",");
    lcd.print(data.yaw);
    lcd.print("   ");
  } else {
    // Print an error message if sending failed
    Serial.println("Failed to send data.");
    Serial.print(data.throttle);
    Serial.print(", Roll: ");
    Serial.print(data.roll);
    Serial.print(", Pitch: ");
    Serial.print(data.pitch);
    Serial.print(", Yaw: ");
    Serial.println(data.yaw);
    lcd.setCursor(0, 0);
    lcd.print("Sent Failed   ");
  }

  // Optional for testing in serial monitor
  // delay(1000);
}
