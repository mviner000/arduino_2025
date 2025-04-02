#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// LCD Setup (I2C Address: 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// MP3 Module Pins (RX, TX)
SoftwareSerial mp3Serial(2, 3);
DFRobotDFPlayerMini mp3; // Create an instance of the DFPlayerMini class

// Button & Busy Pins
const int playButton = 4;
const int stopButton = 5;
const int busyPin = 6;

bool isPlaying = false;

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");

  // Configure Pins
  pinMode(playButton, INPUT_PULLUP);
  pinMode(stopButton, INPUT_PULLUP);
  pinMode(busyPin, INPUT);

  // Start MP3 Module
  mp3Serial.begin(9600);
  delay(1000);

  // Check MP3 Connection
  if (!mp3.begin(mp3Serial)) { // Initialize DFPlayer
    lcd.clear();
    lcd.print("MP3 Error!");
    while (true); // Stop execution if there's an error
  }
  
  lcd.clear();
  lcd.print("MP3 Connected!");
  mp3.volume(15); // Set volume (0-30)
  delay(2000);
  showDefaultMessage(); // Show default text
}

void loop() {
  // Play Button Pressed
  if (digitalRead(playButton) == LOW) {
    delay(50); // Simple debounce
    if (digitalRead(playButton) == LOW) {
      mp3.play(1); // Play track 0001.mp3
      showPlayingMessage();
      isPlaying = true;
    }
    while (digitalRead(playButton) == LOW); // Wait for release
  }

  // Stop Button Pressed
  if (digitalRead(stopButton) == LOW) {
    delay(50);
    if (digitalRead(stopButton) == LOW) {
      mp3.stop(); // Stop playback
      showDefaultMessage();
      isPlaying = false;
    }
    while (digitalRead(stopButton) == LOW);
  }

  // Check if track ended naturally
  if (isPlaying && digitalRead(busyPin) == LOW) {
    isPlaying = false;
    showDefaultMessage();
  }
}

// LCD Display Functions
void showDefaultMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lodi MP3");
  lcd.setCursor(0, 1);
  lcd.print("Mack Rafanan");
}

void showPlayingMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Playing 0001.mp3");
}