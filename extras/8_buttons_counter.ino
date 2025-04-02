#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD setup (address, columns, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set to 0x27 for common I2C address, change if different

// Buttons & LEDs from the new setup
const int buttonPins[] = {2, 3, 4, 5};   // Buttons for LEDs on pins 10-13
const int ledPins[] = {10, 11, 12, 13};  // LEDs controlled by buttons 2-5

// Buttons & LEDs from the previous setup
const int buttonPins2[] = {6, 7, 8, 9};  // Buttons for LEDs on A0-A3
const int ledPins2[] = {A0, A1, A2, A3}; // LEDs controlled by buttons 6-9

void setup() {
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Buttons Pressed:");
  
  // Setup first set of buttons and LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); // Set buttons as input with internal pull-up resistors
    pinMode(ledPins[i], OUTPUT);          // Set LEDs as output
  }

  // Setup second set of buttons and LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins2[i], INPUT_PULLUP);
    pinMode(ledPins2[i], OUTPUT);
  }
}

void loop() {
  int pressedCount = 0;  // Counter for pressed buttons
  
  // Check first set of buttons and control LEDs
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      digitalWrite(ledPins[i], HIGH);
      pressedCount++;  // Increment counter if button is pressed
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }

  // Check second set of buttons and control LEDs
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins2[i]) == LOW) {
      digitalWrite(ledPins2[i], HIGH);
      pressedCount++;  // Increment counter if button is pressed
    } else {
      digitalWrite(ledPins2[i], LOW);
    }
  }
  
  // Update LCD with the count of pressed buttons
  lcd.setCursor(0, 1);  // Move cursor to second line
  lcd.print("      ");  // Clear previous count with spaces
  lcd.setCursor(0, 1);
  lcd.print(pressedCount);  // Display the current count
  lcd.print(" / 8");        // Show total possible buttons
  
  delay(100);  // Small delay to prevent LCD flickering
}