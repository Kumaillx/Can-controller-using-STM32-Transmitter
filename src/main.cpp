#include <Arduino.h>
#include "STM32_CAN.h"

// Onboard LED is on PC13 for Bluepill
#define LED_PIN PC13

STM32_CAN CAN;
uint8_t counter = 0;
unsigned long previousMillis = 0;
const long interval = 500;

void setup() {
  // Initialize the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn LED off (it's active low)

  // Initialize Serial for debugging
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect
  Serial.println("CAN Sender - Initializing...");

  // Initialize CAN bus at 500kbps
  if (CAN.begin(500000)) {
    Serial.println("CAN Initialized Successfully");
  } else {
    Serial.println("CAN Initialization FAILED");
    // Loop and blink fast if CAN init fails
    while (1) {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }
  
  digitalWrite(LED_PIN, HIGH); // Start with LED off
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Create a CAN message
    CAN_Message msg;
    msg.id = 0x123;
    msg.length = 1; // Only sending one byte of data
    msg.data[0] = counter;
    msg.extended = false;

    // Send the message
    CAN.write(msg);

    // Print to serial monitor
    Serial.print("Sent CAN message with ID 0x123, counter: ");
    Serial.println(counter);

    // Blink the LED to indicate transmission
    digitalWrite(LED_PIN, LOW); // Turn LED on
    delay(50);
    digitalWrite(LED_PIN, HIGH); // Turn LED off

    // Increment the counter for the next message
    counter++;
  }
}