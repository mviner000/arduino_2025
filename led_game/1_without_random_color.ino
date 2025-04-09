/*
  Reaction Time: Single-Player Challenge
  For Arduino Uno with one button, buzzer, and RGB LED
  With auto level-up feature based on reaction time
  Added random encouraging comments to keep players engaged
*/

// Pins
const int BUTTON_PIN = 2;
const int BUZZER_PIN = 9;
const int RED_PIN = 11;
const int GREEN_PIN = 12;
const int BLUE_PIN = 10;

// Game Settings
const unsigned long MIN_WAIT = 2000;
const unsigned long MAX_WAIT = 5000;
const unsigned long MAX_REACTION_TIME = 2000;
const unsigned long NO_BEST_TIME = 999999; // Used instead of ULONG_MAX

// Auto level-up threshold - adjust these values as needed
const unsigned long LEVEL_UP_THRESHOLD[] = {250, 220, 200, 180, 160}; // Reaction time thresholds for each level

// Game States
enum GameState {
  IDLE,
  WAIT_START,
  COUNTDOWN,
  RACE,
  SHOW_RESULT
};

// Game Variables
GameState gameState = IDLE;
unsigned long stateStartTime;
unsigned long raceStartTime;
unsigned long waitTime;
unsigned long lastReactionTime = 0;
unsigned long bestReactionTime = NO_BEST_TIME;
int difficultyLevel = 1;
int gamePlayCount = 0; // Track number of games played for comment variety

// Colors (R, G, B)
const int COLORS[][3] = {
  {0, 0, 0},     // Off
  {255, 0, 0},   // Red
  {0, 255, 0},   // Green
  {0, 0, 255},   // Blue
  {255, 255, 0}  // Yellow
};

// Encouraging comments array
const char* ENCOURAGING_COMMENTS[] = {
  "Nice try! You're getting faster.",
  "Keep at it! Just a bit more focus.",
  "So close! You've got this.",
  "Almost there! Try again.",
  "Great effort! Keep practicing.",
  "You're improving with each try!",
  "Stay focused, you can do better!",
  "Just a fraction of a second away!",
  "Don't give up! Next time for sure.",
  "Your reflexes are warming up!",
  "That was a good one! Try again?",
  "Almost beat your record!",
  "You're getting into the rhythm now.",
  "One more try - you're so close!",
  "Focus and breathe - you got this!"
};
const int NUM_COMMENTS = sizeof(ENCOURAGING_COMMENTS) / sizeof(char*);

// Function prototypes
void setColor(int colorIndex, int brightness = 255);
void playTone(int freq, int dur);
void playVictoryMelody();
void printScores();
void startRace();
void resetGame();
void falseStart();
unsigned long getMinWait();
unsigned long getMaxWait();
void startCountdown();
void handleIdleState(unsigned long currentTime);
void handleWaitStartState(unsigned long currentTime);
void handleCountdownState(unsigned long currentTime);
void handleRaceState(unsigned long currentTime);
void handleResultState(unsigned long currentTime);
bool checkForAutoLevelUp(unsigned long reactionTime); // Fixed to return bool
void playLevelUpSound();
void printRandomComment();

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  randomSeed(analogRead(0));
  Serial.begin(9600);
  Serial.println("=== REACTION TIME CHALLENGE ===");
  Serial.println("Auto level-up feature enabled!");
  gameState = IDLE;
  setColor(0);
}

void loop() {
  unsigned long currentTime = millis();
  
  switch(gameState) {
    case IDLE:
      handleIdleState(currentTime);
      break;
    case WAIT_START:
      handleWaitStartState(currentTime);
      break;
    case COUNTDOWN:
      handleCountdownState(currentTime);
      break;
    case RACE:
      handleRaceState(currentTime);
      break;
    case SHOW_RESULT:
      handleResultState(currentTime);
      break;
  }
}

void handleIdleState(unsigned long currentTime) {
  // Pulse blue light
  int brightness = (sin(currentTime / 500.0) + 1) * 127;
  setColor(3, brightness);
  
  // Check if button is pressed to start the game
  if (digitalRead(BUTTON_PIN) == LOW) {
    startCountdown();
  }
  
  // Check for difficulty adjustment (long press)
  static unsigned long buttonPressStart = 0;
  static bool longPressDetected = false;
  
  if (digitalRead(BUTTON_PIN) == LOW && buttonPressStart == 0) {
    buttonPressStart = currentTime;
  }
  
  if (digitalRead(BUTTON_PIN) == LOW && buttonPressStart > 0 && 
      currentTime - buttonPressStart > 2000 && !longPressDetected) {
    // Long press detected
    difficultyLevel = (difficultyLevel % 5) + 1;
    Serial.print("Difficulty changed to: ");
    Serial.println(difficultyLevel);
    playTone(440 + (difficultyLevel * 100), 100);
    longPressDetected = true;
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH && buttonPressStart > 0) {
    if (currentTime - buttonPressStart < 2000 && !longPressDetected) {
      // Short press - start game
      startCountdown();
    }
    buttonPressStart = 0;
    longPressDetected = false;
  }
}

void handleWaitStartState(unsigned long currentTime) {
  // Turn off the LED during wait
  setColor(0);
  
  // Check for false starts
  if (digitalRead(BUTTON_PIN) == LOW) {
    falseStart();
    gameState = SHOW_RESULT;
    stateStartTime = currentTime;
    lastReactionTime = 0; // No reaction time on false start
    return;
  }
  
  // Check if wait time has elapsed
  if (currentTime - stateStartTime >= waitTime) {
    startRace();
  }
}

void handleCountdownState(unsigned long currentTime) {
  // Calculate elapsed time in countdown
  unsigned long elapsedTime = currentTime - stateStartTime;
  
  // Simple countdown with LED and tones
  if (elapsedTime < 1000) {
    if (elapsedTime < 200) {
      setColor(1); // Red
      tone(BUZZER_PIN, 330, 100);
    } else {
      setColor(0); // Off
    }
  } else if (elapsedTime < 2000) {
    if (elapsedTime < 1200) {
      setColor(1); // Red
      tone(BUZZER_PIN, 330, 100);
    } else {
      setColor(0); // Off
    }
  } else if (elapsedTime < 3000) {
    if (elapsedTime < 2200) {
      setColor(1); // Red
      tone(BUZZER_PIN, 330, 100);
    } else {
      setColor(0); // Off
    }
  } else {
    // End of countdown, start waiting period
    gameState = WAIT_START;
    stateStartTime = currentTime;
    waitTime = random(getMinWait(), getMaxWait());
    Serial.println("Get ready, wait for GO!");
  }
}

void handleRaceState(unsigned long currentTime) {
  // Set green light to indicate race is on
  setColor(2);
  
  // Check if player has pressed the button
  if (digitalRead(BUTTON_PIN) == LOW) {
    gamePlayCount++; // Increment games played counter
    lastReactionTime = currentTime - raceStartTime;
    Serial.print("Reaction time: ");
    Serial.print(lastReactionTime);
    Serial.println("ms");
    
    // Check if this is a new best time
    if (lastReactionTime < bestReactionTime) {
      bestReactionTime = lastReactionTime;
      Serial.print("New best time: ");
      Serial.print(bestReactionTime);
      Serial.println("ms");
      
      // Play victory melody
      tone(BUZZER_PIN, 988, 200);
      delay(200);
      tone(BUZZER_PIN, 1319, 400);
      delay(10);
      noTone(BUZZER_PIN);
    } else {
      tone(BUZZER_PIN, 660, 200);
      delay(10);
      noTone(BUZZER_PIN);
    }
    
    // Check for auto level-up
    bool didLevelUp = checkForAutoLevelUp(lastReactionTime);
    
    // If player didn't level up, give encouraging comment
    if (!didLevelUp) {
      printRandomComment();
    }
    
    gameState = SHOW_RESULT;
    stateStartTime = currentTime;
  }
  
  // Check for timeout
  if (currentTime - raceStartTime > MAX_REACTION_TIME) {
    Serial.println("Time's up! No reaction detected.");
    lastReactionTime = 0; // No reaction time on timeout
    gameState = SHOW_RESULT;
    stateStartTime = currentTime;
    tone(BUZZER_PIN, 220, 500);
    delay(10);
    noTone(BUZZER_PIN);
  }
}

void handleResultState(unsigned long currentTime) {
  // Display result for a few seconds
  if (lastReactionTime > 0) {
    // Flash green with faster pulsing for better times
    int pulseSpeed = map(lastReactionTime, 0, MAX_REACTION_TIME, 50, 200);
    int brightness = (sin(currentTime / pulseSpeed) + 1) * 127;
    setColor(2, brightness);
  } else {
    // No reaction or false start - flash red
    int brightness = (sin(currentTime / 100.0) + 1) * 127;
    setColor(1, brightness);
  }
  
  // Return to idle state after showing result
  if (currentTime - stateStartTime > 3000) {
    gameState = IDLE;
    printScores();
    Serial.println("Ready for next round...");
  }
}

// New function to display random encouraging comments
void printRandomComment() {
  // Only show comments occasionally to avoid being too repetitive
  // More frequent early on, less frequent as player plays more games
  bool showComment = (random(100) < (100 - min(80, gamePlayCount)));
  
  if (showComment) {
    int commentIndex = random(NUM_COMMENTS);
    Serial.println(ENCOURAGING_COMMENTS[commentIndex]);
  }
}

// Modified to return whether a level-up occurred
bool checkForAutoLevelUp(unsigned long reactionTime) {
  // Only level up if not already at max level
  if (difficultyLevel < 5) {
    // Check if reaction time is better than the threshold for the current level
    if (reactionTime <= LEVEL_UP_THRESHOLD[difficultyLevel - 1]) {
      difficultyLevel++;
      Serial.println("*****************************");
      Serial.print("LEVEL UP! Difficulty increased to: ");
      Serial.println(difficultyLevel);
      Serial.println("*****************************");
      
      // Play level-up sound
      playLevelUpSound();
      return true;
    }
  }
  return false;
}

// New function for level-up sound
void playLevelUpSound() {
  // Play ascending notes to indicate level-up
  tone(BUZZER_PIN, 523, 100); // C5
  delay(120);
  tone(BUZZER_PIN, 659, 100); // E5
  delay(120);
  tone(BUZZER_PIN, 784, 100); // G5
  delay(120);
  tone(BUZZER_PIN, 1047, 200); // C6
  delay(210);
  noTone(BUZZER_PIN);
}

void setColor(int colorIndex, int brightness) {
  analogWrite(RED_PIN, COLORS[colorIndex][0] * brightness / 255);
  analogWrite(GREEN_PIN, COLORS[colorIndex][1] * brightness / 255);
  analogWrite(BLUE_PIN, COLORS[colorIndex][2] * brightness / 255);
}

void playTone(int freq, int dur) {
  tone(BUZZER_PIN, freq, dur);
  delay(10); // Short delay to ensure the tone starts
  noTone(BUZZER_PIN); // Explicitly stop the tone
}

void playVictoryMelody() {
  // This function is no longer used directly - the melody is played inline
  tone(BUZZER_PIN, 988, 200);
  delay(200);
  tone(BUZZER_PIN, 1319, 400);
  delay(400);
  noTone(BUZZER_PIN);
}

void printScores() {
  Serial.print("Last reaction time: ");
  if (lastReactionTime > 0) {
    Serial.print(lastReactionTime);
    Serial.println("ms");
  } else {
    Serial.println("N/A (false start or timeout)");
  }
  
  Serial.print("Best reaction time: ");
  if (bestReactionTime < NO_BEST_TIME) {
    Serial.print(bestReactionTime);
    Serial.println("ms");
  } else {
    Serial.println("N/A");
  }
  
  Serial.print("Difficulty: ");
  Serial.println(difficultyLevel);
}

void startRace() {
  gameState = RACE;
  raceStartTime = millis();
  Serial.println("GO!");
  tone(BUZZER_PIN, 880, 200);
  delay(10);
  noTone(BUZZER_PIN);
}

void resetGame() {
  bestReactionTime = NO_BEST_TIME;
  gamePlayCount = 0;
  gameState = IDLE;
  setColor(0);
  Serial.println("Game reset");
}

void falseStart() {
  Serial.println("False start!");
  tone(BUZZER_PIN, 220, 500);
  delay(10);
  noTone(BUZZER_PIN);
}

unsigned long getMinWait() {
  return max(1000, MIN_WAIT - (difficultyLevel * 200));
}

unsigned long getMaxWait() {
  return max(2000, MAX_WAIT - (difficultyLevel * 300));
}

void startCountdown() {
  gameState = COUNTDOWN;
  stateStartTime = millis();
  Serial.println("3... 2... 1...");
}