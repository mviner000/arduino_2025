#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// MP3 Module Pins (RX, TX)
SoftwareSerial mp3Serial(2, 3);
DFRobotDFPlayerMini mp3; // Create an instance of the DFPlayerMini class

// Button & Busy Pins
const int playButton = 4;
const int stopButton = 5;
const int busyPin = 6;

bool isPlaying = false;

void setup() {
  // Configure Pins
  pinMode(playButton, INPUT_PULLUP);
  pinMode(stopButton, INPUT_PULLUP);
  pinMode(busyPin, INPUT);

  // Start Serial (optional - for debugging)
  Serial.begin(9600);
  Serial.println("Initializing...");

  // Start MP3 Module
  mp3Serial.begin(9600);
  delay(1000);

  // Check MP3 Connection
  if (!mp3.begin(mp3Serial)) { // Initialize DFPlayer
    Serial.println("MP3 Error!");
    while (true); // Stop execution if there's an error
  }
  
  Serial.println("MP3 Connected!");
  mp3.volume(15); // Set volume (0-30)
}

void loop() {
  // Play Button Pressed
  if (digitalRead(playButton) == LOW) {
    delay(50); // Simple debounce
    if (digitalRead(playButton) == LOW) {
      mp3.play(1); // Play track 0001.mp3
      isPlaying = true;
      Serial.println("Playing");
    }
    while (digitalRead(playButton) == LOW); // Wait for release
  }

  // Stop Button Pressed
  if (digitalRead(stopButton) == LOW) {
    delay(50);
    if (digitalRead(stopButton) == LOW) {
      mp3.stop(); // Stop playback
      isPlaying = false;
      Serial.println("Stopped");
    }
    while (digitalRead(stopButton) == LOW);
  }

  // Check if track ended naturally
  if (isPlaying && digitalRead(busyPin) == HIGH) {
    isPlaying = false;
    Serial.println("Track ended");
  }
}