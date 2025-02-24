#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// RFID Configuration
#define SS_PIN 10
#define RST_PIN 7
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// LED & Buzzer Configuration
#define LED_PIN 3   // External LED on Digital Pin 3
#define BUZZER_PIN 2 // Buzzer on Digital Pin 2

// Timing Constants
#define DISPLAY_DURATION 2000  // 2 seconds display time
#define DEBOUNCE_DELAY 1000    // 1 second between reads

unsigned long lastReadTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  rfid.PCD_Init();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Scanner");
  lcd.setCursor(0, 1);
  lcd.print("  Ready...  ");
  delay(1000);
  lcd.clear();

  // Configure LED and Buzzer
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  if (millis() - lastReadTime < DEBOUNCE_DELAY) return;

  if (!rfid.PICC_IsNewCardPresent()) {
    lcd.setCursor(0, 0);
    lcd.print(" HOTEL BEMBANGAN! ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off when idle
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    handleReadError();
    return;
  }

  lastReadTime = millis();
  handleSuccessfulRead();
  rfid.PICC_HaltA();
}

void handleSuccessfulRead() {
  // Turn on LED and buzzer
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  
  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  
  lcd.setCursor(0, 1);
  lcd.print("Boss Mack!");

  // Serial output
  Serial.println("Valid Card Detected - Welcome, Boss Mack!");

  // Keep LED & buzzer on for display duration
  delay(DISPLAY_DURATION);

  // Turn off LED and buzzer
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.clear();
}

void handleReadError() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Read Error!");
  Serial.println("Error reading card");

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  delay(1000);
  lcd.clear();
}
