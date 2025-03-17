// flappy_bird_tft_with_button_uno.ino

#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library

// Define screen dimensions
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// Define pins for ILI9341 display
#define TFT_CS   10  // Chip select
#define TFT_DC   9   // Data/Command
#define TFT_RST  8   // Reset pin

// Create display instance with hardware SPI plus CS/DC/RST pins
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Color definitions
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xFFFF
#define TFT_BLUE 0x001F
#define TFT_CYAN 0x07FF
#define TFT_RED 0xF800
#define TFT_GREEN 0x07E0
#define TFT_YELLOW 0xFFE0
#define TFT_ORANGE 0xFD20
#define TFT_BROWN 0x8A22
#define TFT_SKYBLUE 0x867D
#define TFT_LIGHTGREEN 0x87F0

// Button pin
#define FLAP_BUTTON_PIN 2  // Digital pin for flap button

// Game constants
#define BIRD_WIDTH 16
#define BIRD_HEIGHT 12
#define PIPE_WIDTH 40
#define PIPE_GAP 70
#define GRAVITY 0.6
#define FLAP_FORCE -7.0
#define GROUND_HEIGHT 30
#define MAX_PIPES 4

// Game variables
float birdX, birdY, birdVelocity;
int score;
boolean gameOver;
unsigned long lastInputTime;
unsigned long lastUpdateTime;
unsigned long lastPipeTime;
boolean buttonPressed;
boolean prevButtonState;

// Pipe structure
struct Pipe {
  int x;
  int gapTop;
  boolean counted;
};

// Pipe array
Pipe pipes[MAX_PIPES];

void setup() {
  Serial.begin(9600);
  
  // Initialize display
  tft.begin();
  tft.setRotation(0); // Portrait orientation
  tft.fillScreen(TFT_BLACK);
  
  // Initialize button pin as input with pull-up resistor
  pinMode(FLAP_BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("Flappy Bird Game Initialized!");
  Serial.print("Screen dimensions: ");
  Serial.print(TFT_WIDTH);
  Serial.print("x");
  Serial.println(TFT_HEIGHT);
  
  // Initialize game
  resetGame();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check for game restart
  if (gameOver) {
    boolean buttonState = digitalRead(FLAP_BUTTON_PIN);
    if (buttonState == LOW && (currentTime - lastInputTime > 500)) {
      resetGame();
    }
    return;
  }
  
  // Handle button input for flap
  handleButtonInput();
  
  // Update game at 60fps (approximately)
  if (currentTime - lastUpdateTime > 16) {
    lastUpdateTime = currentTime;
    updateGame();
    drawGame();
  }
}

void resetGame() {
  // Reset bird position and velocity
  birdX = TFT_WIDTH / 4;
  birdY = TFT_HEIGHT / 2;
  birdVelocity = 0;
  
  // Reset score
  score = 0;
  
  // Reset game state
  gameOver = false;
  buttonPressed = false;
  prevButtonState = HIGH;  // Initialize to not pressed (pull-up resistor)
  
  // Initialize pipes
  for (int i = 0; i < MAX_PIPES; i++) {
    pipes[i].x = TFT_WIDTH + (i * (PIPE_WIDTH + 80));
    pipes[i].gapTop = random(40, TFT_HEIGHT - GROUND_HEIGHT - PIPE_GAP - 40);
    pipes[i].counted = false;
  }
  
  lastUpdateTime = millis();
  lastInputTime = millis();
  lastPipeTime = millis();
  
  // Draw initial game screen
  drawBackground();
  drawBird();
  drawPipes();
  drawGround();
  drawScore();
}

void handleButtonInput() {
  unsigned long currentTime = millis();
  
  // Read button state (LOW when pressed because of pull-up resistor)
  boolean buttonState = digitalRead(FLAP_BUTTON_PIN);
  
  // Detect button press (falling edge)
  if (buttonState == LOW && prevButtonState == HIGH && (currentTime - lastInputTime > 150)) {
    birdVelocity = FLAP_FORCE;
    lastInputTime = currentTime;
  }
  
  // Save current button state for next comparison
  prevButtonState = buttonState;
}

void updateGame() {
  // Update bird position and velocity
  birdVelocity += GRAVITY;
  birdY += birdVelocity;
  
  // Keep bird within screen bounds
  if (birdY < 0) {
    birdY = 0;
    birdVelocity = 0;
  }
  
  // Check for ground collision
  if (birdY > TFT_HEIGHT - GROUND_HEIGHT - BIRD_HEIGHT) {
    birdY = TFT_HEIGHT - GROUND_HEIGHT - BIRD_HEIGHT;
    gameOver = true;
  }
  
  // Update pipes
  for (int i = 0; i < MAX_PIPES; i++) {
    // Move pipes left
    pipes[i].x -= 2;
    
    // Check if pipe is off screen and reset it
    if (pipes[i].x < -PIPE_WIDTH) {
      pipes[i].x = TFT_WIDTH + 40;
      pipes[i].gapTop = random(40, TFT_HEIGHT - GROUND_HEIGHT - PIPE_GAP - 40);
      pipes[i].counted = false;
    }
    
    // Check for collision with pipes
    if (birdX + BIRD_WIDTH > pipes[i].x && birdX < pipes[i].x + PIPE_WIDTH) {
      // Check if bird is within the gap
      if (birdY < pipes[i].gapTop || birdY + BIRD_HEIGHT > pipes[i].gapTop + PIPE_GAP) {
        gameOver = true;
      }
      // Update score when passing through a pipe
      else if (!pipes[i].counted && birdX > pipes[i].x + PIPE_WIDTH / 2) {
        score++;
        pipes[i].counted = true;
      }
    }
  }
  
  // If game over, display game over screen
  if (gameOver) {
    drawGameOver();
  }
}

void drawBackground() {
  // Clear screen with sky color
  tft.fillScreen(TFT_SKYBLUE);
}

void drawBird() {
  // Draw bird (simple rectangle with color)
  int birdDrawY = (int)birdY;
  
  // First clear the old bird position (a bit larger to ensure clean redraw)
  tft.fillRect(birdX - 1, 0, BIRD_WIDTH + 2, TFT_HEIGHT - GROUND_HEIGHT, TFT_SKYBLUE);
  
  // Draw the bird body
  tft.fillRoundRect(birdX, birdDrawY, BIRD_WIDTH, BIRD_HEIGHT, 3, TFT_YELLOW);
  
  // Draw wing
  if (birdVelocity < 0) {
    // Wing up position
    tft.fillRect(birdX + 2, birdDrawY + BIRD_HEIGHT - 3, 8, 3, TFT_ORANGE);
  } else {
    // Wing down position
    tft.fillRect(birdX + 2, birdDrawY + BIRD_HEIGHT, 8, 3, TFT_ORANGE);
  }
  
  // Draw eye
  tft.fillCircle(birdX + BIRD_WIDTH - 4, birdDrawY + 4, 2, TFT_WHITE);
  tft.fillCircle(birdX + BIRD_WIDTH - 3, birdDrawY + 4, 1, TFT_BLACK);
  
  // Draw beak
  tft.fillTriangle(
    birdX + BIRD_WIDTH, birdDrawY + 5,
    birdX + BIRD_WIDTH + 4, birdDrawY + 7,
    birdX + BIRD_WIDTH, birdDrawY + 9,
    TFT_ORANGE
  );
}

void drawPipes() {
  for (int i = 0; i < MAX_PIPES; i++) {
    // Clear previous pipe position
    if (pipes[i].x >= 0 && pipes[i].x <= TFT_WIDTH) {
      tft.fillRect(pipes[i].x - 2, 0, PIPE_WIDTH + 4, TFT_HEIGHT - GROUND_HEIGHT, TFT_SKYBLUE);
    }
    
    // Draw top pipe
    tft.fillRect(pipes[i].x, 0, PIPE_WIDTH, pipes[i].gapTop, TFT_GREEN);
    tft.fillRect(pipes[i].x - 2, pipes[i].gapTop - 10, PIPE_WIDTH + 4, 10, TFT_GREEN);
    
    // Draw bottom pipe
    int bottomPipeTop = pipes[i].gapTop + PIPE_GAP;
    tft.fillRect(pipes[i].x, bottomPipeTop, PIPE_WIDTH, TFT_HEIGHT - bottomPipeTop - GROUND_HEIGHT, TFT_GREEN);
    tft.fillRect(pipes[i].x - 2, bottomPipeTop, PIPE_WIDTH + 4, 10, TFT_GREEN);
  }
}

void drawGround() {
  // Draw ground
  tft.fillRect(0, TFT_HEIGHT - GROUND_HEIGHT, TFT_WIDTH, GROUND_HEIGHT, TFT_BROWN);
  
  // Draw grass on top of ground
  for (int x = 0; x < TFT_WIDTH; x += 4) {
    tft.drawFastVLine(x, TFT_HEIGHT - GROUND_HEIGHT - 4, 4, TFT_LIGHTGREEN);
  }
}

void drawScore() {
  // Clear score area
  tft.fillRect(TFT_WIDTH / 2 - 20, 20, 40, 20, TFT_SKYBLUE);
  
  // Draw score
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(TFT_WIDTH / 2 - 10, 20);
  if (score < 10) {
    tft.setCursor(TFT_WIDTH / 2 - 6, 20);
  }
  tft.print(score);
}

void drawGameOver() {
  // Draw semi-transparent overlay
  for (int y = 0; y < TFT_HEIGHT - GROUND_HEIGHT; y += 2) {
    tft.drawFastHLine(0, y, TFT_WIDTH, TFT_BLACK);
  }
  
  // Draw game over text
  tft.fillRect(TFT_WIDTH / 2 - 80, TFT_HEIGHT / 2 - 40, 160, 80, TFT_BLACK);
  tft.drawRect(TFT_WIDTH / 2 - 80, TFT_HEIGHT / 2 - 40, 160, 80, TFT_WHITE);
  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(TFT_WIDTH / 2 - 60, TFT_HEIGHT / 2 - 25);
  tft.print("GAME OVER");
  
  tft.setCursor(TFT_WIDTH / 2 - 45, TFT_HEIGHT / 2);
  tft.print("Score: ");
  tft.print(score);
  
  tft.setTextSize(1);
  tft.setCursor(TFT_WIDTH / 2 - 75, TFT_HEIGHT / 2 + 25);
  tft.print("Press button to restart");
}

void drawGame() {
  // Only redraw the necessary elements to minimize flickering
  drawBird();
  drawPipes();
  drawScore();
}