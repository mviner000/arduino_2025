#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// RFID Configuration
#define SS_PIN 10
#define RST_PIN 7
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// LED & Buzzer Configuration
#define LED_PIN 3    // External LED on Digital Pin 3
#define BUZZER_PIN 2 // Buzzer on Digital Pin 2

// Timing Constants
#define DISPLAY_DURATION 2000  // 2 seconds display time
#define DEBOUNCE_DELAY 1000    // 1 second between reads

// Authorized UID
const String authorizedUID = "83A58322";

unsigned long lastReadTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Scanner");
  lcd.setCursor(0, 1);
  lcd.print("  Ready...  ");
  delay(1000);
  lcd.clear();

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  if (millis() - lastReadTime < DEBOUNCE_DELAY) return;

  if (!rfid.PICC_IsNewCardPresent()) {
    lcd.setCursor(0, 0);
    lcd.print(" HOTEL BEMBANGAN!");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    handleReadError();
    return;
  }

  lastReadTime = millis();
  String readUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    readUID += String(rfid.uid.uidByte[i], HEX);
  }
  readUID.toUpperCase();

  if (readUID == authorizedUID) {
    handleSuccessfulRead();
  } else {
    handleWrongCard();
  }

  rfid.PICC_HaltA();
}

void handleSuccessfulRead() {
  digitalWrite(LED_PIN, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("Boss Mack!");
  Serial.println("Valid Card Detected - Welcome, Boss Mack!");

  // Play single note (1000 Hz for 500 ms)
  tone(BUZZER_PIN, 1000, 500); 
  delay(500); // Wait for note to finish

  delay(DISPLAY_DURATION);
  digitalWrite(LED_PIN, LOW);
  lcd.clear();
}

void handleWrongCard() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PUTANGNAKA!");
  Serial.println("Invalid Card Detected!");
  alarmSound();  // Play alarm sound
  lcd.clear();
}

void handleReadError() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Read Error!");
  Serial.println("Error reading card");
  delay(1000);
  lcd.clear();
}

void alarmSound() {
  for (int i = 0; i < 5; i++) { // Beep 5 times
    tone(BUZZER_PIN, 1000);     // 1000 Hz tone
    delay(300);                 // Beep duration
    noTone(BUZZER_PIN);         
    delay(200);                 // Pause between beeps
  }
}