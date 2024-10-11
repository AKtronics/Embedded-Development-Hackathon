// code for ESP32 board-1 as transmitter with microphone

#include "BluetoothSerial.h" // Including library for Bluetooth 
#include "Filters.h"  // Include library for filtering

// Pin where the microphone is connected
#define MIC_PIN 34

// Bluetooth Serial object to handle communication
BluetoothSerial btSerial;

FilterOnePole lowpassFilter(LOWPASS, 3300); // Filtering frequencies above 3300 Hz
FilterOnePole highpassFilter(HIGHPASS, 400);// Filtering frequencies below 400 Hz

// non-blocking delays using mills
unsigned long previousMillis = 0;
const long interval = 5;  // Interval (in milliseconds) to send data for smooth delay process

// Smoothing factor for moving average filter
const int smoothingWindowSize = 10;  // Number of samples for average filter
int smoothingBuffer[smoothingWindowSize];  // Buffer to store recent audio data
int smoothingIndex = 0;
long smoothingSum = 0;

void setup() {
  Serial.begin(115200);

  // Initialize Bluetooth
  if (!btSerial.begin("ESP32_Voice_Transmitter")) {
    Serial.println("Error initializing Bluetooth");
    while (true); 
  }
  Serial.println("Bluetooth initialized successfully.");

  // Set the microphone pin as input
  pinMode(MIC_PIN, INPUT);

  // Initialize smoothing buffer with zeros
  for (int i = 0; i < smoothingWindowSize; i++) {
    smoothingBuffer[i] = 0;
  }
}

void loop() {
  // Check the current time
  unsigned long currentMillis = millis();

  // non-blocking delay using millis() to ensure real-time performance
  if (currentMillis - previousMillis >= interval) {
    // Save the last time data was sent
    previousMillis = currentMillis;

    // Capture the raw audio data from the microphone
    int rawAudioData = analogRead(MIC_PIN);

    // Apply low-pass filter to allow frequencies below 3300 Hz
    int filteredAudioData = lowpassFilter.input(rawAudioData);

    // Apply high-pass filter to allow frequencies above 400 Hz (bandpass result)
    filteredAudioData = highpassFilter.input(filteredAudioData);

    // Apply a simple moving average for smoothing the filtered audio signal
    smoothingSum = smoothingSum - smoothingBuffer[smoothingIndex];  // Subtract the oldest sample
    smoothingBuffer[smoothingIndex] = filteredAudioData;            // Store the new sample
    smoothingSum = smoothingSum + smoothingBuffer[smoothingIndex];  // Add the new sample to the sum

    // Move the index to the next position
    smoothingIndex = (smoothingIndex + 1) % smoothingWindowSize;

    // Compute the smoothed audio value
    int smoothedAudioData = smoothingSum / smoothingWindowSize;

    // Transmit the smoothed audio data over Bluetooth
    // Convert the smoothed audio data into bytes and send over Bluetooth
    btSerial.write((uint8_t*)&smoothedAudioData, sizeof(smoothedAudioData));
    Serial.println(smoothedAudioData);
  }
}
