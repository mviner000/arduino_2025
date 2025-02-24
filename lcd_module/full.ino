#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN 10
#define RST_PIN 7
#define LED_PIN 3    // External LED
#define SPEAKER_PIN 4// Speaker/buzzer

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setup() {
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  
  // Set initial messages
  lcd.setCursor(0, 0);
  lcd.print("Lodi               "); // Pad with spaces
  lcd.setCursor(0, 1);
  lcd.print("Mack Rafanan");
  
  // Initialize RFID
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT); // Initialize speaker pin
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Activate both LED and speaker
  digitalWrite(LED_PIN, HIGH);
  tone(SPEAKER_PIN, 1000); // Generate 1kHz tone
  
  Serial.print(F("RFID Detected - UID:"));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Update LCD
  lcd.setCursor(0, 0);
  lcd.print("Hello, Boss.       "); // Pad with spaces
  delay(500); // Keep LED and buzzer on for 500ms
  
  // Turn off both devices
  digitalWrite(LED_PIN, LOW);
  noTone(SPEAKER_PIN);

  // Wait for 4500ms
  delay(4500);
  
  // Revert LCD to original message
  lcd.setCursor(0, 0);
  lcd.print("Lodi               "); // Pad with spaces
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}