#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

const uint64_t pipeIn = 0xE9E8F0F0E1LL;
RF24 radio(9, 10); // CE, CSN

int ch_width_1 = 0;
int ch_width_2 = 0;
int ch_width_3 = 0;
int ch_width_4 = 0;
int ch_width_5 = 0;

Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;

struct Signal {
  byte throttle;      
  byte pitch;
  byte roll;
  byte yaw;
  byte aux1;
};

Signal data;

void ResetData() {
  data.throttle = 0; // Motor Stop
  data.pitch = 127;  // Center
  data.roll = 127;   // Center
  data.yaw = 127;    // Center
  data.aux1 = 0;
}

void setup() {
  Serial.begin(115200); // Start serial for debugging

  // Set the pins for each PWM signal
  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);

  // Configure the NRF24 module
  ResetData();
  radio.begin();
  radio.openReadingPipe(1, pipeIn);
  radio.setChannel(108);
  radio.setAutoAck(false); // Enable auto acknowledgment
  radio.setDataRate(RF24_250KBPS); // Set data rate for stability
  radio.setPALevel(RF24_PA_MAX); // Set power level to maximum
  radio.startListening(); // Start listening for incoming data

  Serial.println("Receiver Initialized");
}

unsigned long lastRecvTime = 0;

void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(Signal));
    lastRecvTime = millis(); // Update the last received time

    // Print the received data for debugging
    Serial.print("Received Data - Throttle: ");
    Serial.print(data.throttle);
    Serial.print(", Pitch: ");
    Serial.print(data.pitch);
    Serial.print(", Roll: ");
    Serial.print(data.roll);
    Serial.print(", Yaw: ");
    Serial.println(data.yaw);
    Serial.print(", Aux1: ");
    Serial.println(data.aux1);
  }
}

void loop() {
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) { // If no data received for more than 1 second
    ResetData(); // Reset data to default values
    Serial.println("Signal lost, resetting data...");
  }

  recvData(); // Continuously check for new data

  // Map the received values to PWM signal range
  ch_width_1 = map(data.yaw, 0, 255, 1000, 2000);
  ch_width_2 = map(data.pitch, 0, 255, 1000, 2000); 
  ch_width_3 = map(data.throttle, 0, 255, 900, 2100); 
  ch_width_4 = map(data.roll, 0, 255, 1000, 2000);
  ch_width_5 = map(data.aux1, 0, 1, 1000, 2000); 

  // Write the PWM signal to each servo
  ch1.writeMicroseconds(ch_width_1);
  ch2.writeMicroseconds(ch_width_2);
  ch3.writeMicroseconds(ch_width_3);
  ch4.writeMicroseconds(ch_width_4);
  ch5.writeMicroseconds(ch_width_5);
}
