// code for ESP32 board-2 as receiver with speaker

#include "BluetoothSerial.h" // Including library for Bluetooth
#include "driver/dac.h"  // Include DAC library for audio output

// Bluetooth Serial object for communication
BluetoothSerial btSerial;

// non-blocking delays using millis
unsigned long previousMillis = 0;
const long interval = 1;  // Adjust the interval for data processing

// Smoothing factor for moving average filter
const int smoothingWindowSize = 10;  // Number of samples for average filtering
int smoothingBuffer[smoothingWindowSize];  // Buffer to store recent audio data
int smoothingIndex = 0;
long smoothingSum = 0;

void setup() {
  Serial.begin(115200);

  // Initialize Bluetooth Serial with the device name
  if (!btSerial.begin("ESP32_Receiver")) {
    // If Bluetooth initialization fails, print an error and halt execution
    Serial.println("Bluetooth initialization failed!");
    while (true);  // Stop further execution
  }
  Serial.println("Bluetooth initialized successfully!");

  // Enable DAC for audio output (GPIO 25)
  dac_output_enable(DAC_CHANNEL_1);

  // Initialize smoothing buffer with zeros
  for (int i = 0; i < smoothingWindowSize; i++) {
    smoothingBuffer[i] = 0;
  }
}

void loop() {
  // Get the current time
  unsigned long currentMillis = millis();

  // Non-blocking delay using millis()
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check if Bluetooth has incoming audio data
    if (btSerial.available()) {
      int audioData = 0;

      // Read the incoming audio data from Bluetooth
      btSerial.readBytes((char*)&audioData, sizeof(audioData));

      // Map the audio data (0-4095 from ADC) to DAC output range (0-255)
      int mappedAudioData = map(audioData, 0, 4095, 0, 255);

      // Apply a simple moving average filter for smoothing
      smoothingSum = smoothingSum - smoothingBuffer[smoothingIndex];  // Remove the oldest sample
      smoothingBuffer[smoothingIndex] = mappedAudioData;              // Store the new sample
      smoothingSum = smoothingSum + smoothingBuffer[smoothingIndex];  // Add the new sample to the sum

      // Move the index to the next position
      smoothingIndex = (smoothingIndex + 1) % smoothingWindowSize;

      // Compute the smoothed audio value
      int smoothedAudioData = smoothingSum / smoothingWindowSize;

      // Output the smoothed audio data to the DAC (speaker output)
      dac_output_voltage(DAC_CHANNEL_1, smoothedAudioData);
    }
    else {
      static unsigned long lastPrint = 0;
      if (millis() - lastPrint > 1000) {  
        Serial.println("No incoming audio data available.");
        lastPrint = millis();
      }
    }
  }
}
