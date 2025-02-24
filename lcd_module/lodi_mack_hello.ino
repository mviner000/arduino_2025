#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Initialize the LCD
  lcd.init();
  
  // Turn on the backlight
  lcd.backlight();
  
  // Set cursor to first column, first row
  lcd.setCursor(0, 0);
  
  // Print the first line
  lcd.print("Lodi");
  
  // Set cursor to first column, second row
  lcd.setCursor(0, 1);
  
  // Print the second line
  lcd.print("Mack Rafanan");
}

void loop() {
  // Empty loop - message stays static
}