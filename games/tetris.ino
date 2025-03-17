// tetris_tft_with_joystick_uno.ino

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
#define TFT_MAGENTA 0xF81F
#define TFT_NAVY 0x000F
#define TFT_RED 0xF800
#define TFT_GREEN 0x07E0
#define TFT_BLUE 0x001F
#define TFT_CYAN 0x07FF
#define TFT_YELLOW 0xFFE0
#define TFT_ORANGE 0xFD20
#define TFT_PURPLE 0x8010

// Joystick pins (analog)
#define JOYSTICK_Y_PIN A0  // Y-axis for up/down movement
#define JOYSTICK_X_PIN A1  // X-axis for left/right movement

// Game constants
#define BLOCK_SIZE 10
#define GRID_WIDTH 12
#define GRID_HEIGHT 20
#define GRID_X_OFFSET 100
#define GRID_Y_OFFSET 30

// Game variables
uint8_t grid[GRID_HEIGHT][GRID_WIDTH];
uint8_t currentPiece[4][4];
uint8_t nextPiece[4][4];
int currentX, currentY;
int currentRotation;
int currentPieceType;
int nextPieceType;
unsigned long lastMoveTime;
unsigned long lastInputTime;
int level = 1;
int score = 0;
int linesCleared = 0;
boolean gameOver = false;

// Define tetromino shapes (I, J, L, O, S, T, Z)
const uint8_t PROGMEM tetrominos[7][4][4] = {
  { // I piece
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // J piece
    {2, 0, 0, 0},
    {2, 2, 2, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // L piece
    {0, 0, 3, 0},
    {3, 3, 3, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // O piece
    {0, 4, 4, 0},
    {0, 4, 4, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // S piece
    {0, 5, 5, 0},
    {5, 5, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // T piece
    {0, 6, 0, 0},
    {6, 6, 6, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // Z piece
    {7, 7, 0, 0},
    {0, 7, 7, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  }
};

// Colors for each tetromino type
const uint16_t blockColors[] = {
  TFT_BLACK,  // Empty
  TFT_CYAN,   // I piece
  TFT_BLUE,   // J piece
  TFT_ORANGE, // L piece
  TFT_YELLOW, // O piece
  TFT_GREEN,  // S piece
  TFT_PURPLE, // T piece
  TFT_RED     // Z piece
};

void setup() {
  Serial.begin(9600);
  
  // Initialize display
  tft.begin();
  tft.setRotation(0); // Portrait orientation
  tft.fillScreen(TFT_BLACK);
  
  Serial.println("Tetris Game Initialized!");
  Serial.print("Screen dimensions: ");
  Serial.print(TFT_WIDTH);
  Serial.print("x");
  Serial.println(TFT_HEIGHT);
  
  // Initialize game
  resetGame();
  drawGameScreen();
}

void loop() {
  // Update top info bar
  updateInfoBar();
  
  if (!gameOver) {
    // Handle joystick input
    handleJoystickInput();
    
    // Move piece down at regular intervals
    if (millis() - lastMoveTime > (1000 - (level * 50))) {
      lastMoveTime = millis();
      if (!movePieceDown()) {
        placePiece();
        checkLines();
        spawnNewPiece();
      }
    }
  } else {
    // Display game over and wait for reset
    if (millis() - lastMoveTime > 2000) {
      lastMoveTime = millis();
      tft.fillRect(50, 100, 220, 60, TFT_BLACK);
      tft.drawRect(50, 100, 220, 60, TFT_WHITE);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.setCursor(80, 110);
      tft.print("GAME OVER");
      tft.setCursor(70, 130);
      tft.print("Score: ");
      tft.print(score);
    }
    
    // Check for joystick press to restart
    int xVal = analogRead(JOYSTICK_X_PIN);
    if (xVal > 700 || xVal < 300) {
      resetGame();
      drawGameScreen();
    }
  }
}

void resetGame() {
  // Clear grid
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      grid[y][x] = 0;
    }
  }
  
  // Reset game variables
  level = 1;
  score = 0;
  linesCleared = 0;
  gameOver = false;
  
  // Spawn first pieces
  nextPieceType = random(7);
  loadPiece(nextPiece, nextPieceType);
  spawnNewPiece();
  
  lastMoveTime = millis();
  lastInputTime = millis();
}

void drawGameScreen() {
  // Clear screen
  tft.fillScreen(TFT_BLACK);
  
  // Draw game border
  tft.drawRect(GRID_X_OFFSET - 1, GRID_Y_OFFSET - 1, 
               GRID_WIDTH * BLOCK_SIZE + 2, 
               GRID_HEIGHT * BLOCK_SIZE + 2, 
               TFT_WHITE);
  
  // Draw "Next" box
  tft.drawRect(GRID_X_OFFSET + GRID_WIDTH * BLOCK_SIZE + 10, 
               GRID_Y_OFFSET, 
               6 * BLOCK_SIZE, 
               6 * BLOCK_SIZE, 
               TFT_WHITE);
  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(GRID_X_OFFSET + GRID_WIDTH * BLOCK_SIZE + 15, GRID_Y_OFFSET - 15);
  tft.print("NEXT");
  
  // Draw score and level info
  tft.setCursor(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 20);
  tft.print("SCORE:");
  
  tft.setCursor(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 40);
  tft.print("LEVEL:");
  
  tft.setCursor(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 60);
  tft.print("LINES:");
  
  updateScoreDisplay();
  drawNextPiece();
  drawGrid();
}

void updateInfoBar() {
  static unsigned long prmillis2 = 0;
  if (millis() - prmillis2 >= 1000) {
    prmillis2 = millis();
    
    // Clear header area
    tft.fillRect(0, 0, TFT_WIDTH, 15, TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    
    // Left-aligned text
    tft.setCursor(0, 2);
    tft.print("TETRIS");
    
    // Center-aligned text for time
    char timeStr[6];
    unsigned long gameTime = millis() / 1000;
    sprintf(timeStr, "%02d:%02d", (gameTime / 60) % 60, gameTime % 60);
    tft.setCursor((TFT_WIDTH / 2) - 15, 2);
    tft.print(timeStr);
    
    // Right-aligned text for version
    tft.setCursor(TFT_WIDTH - 25, 2);
    tft.print("v1.0");
  }
}

void updateScoreDisplay() {
  // Clear score area
  tft.fillRect(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 30, 75, 50, TFT_BLACK);
  
  // Display score, level and lines
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  
  tft.setCursor(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 30);
  tft.print(score);
  
  tft.setCursor(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 50);
  tft.print(level);
  
  tft.setCursor(GRID_X_OFFSET - 80, GRID_Y_OFFSET + 70);
  tft.print(linesCleared);
}

void drawBlock(int x, int y, uint8_t blockType) {
  uint16_t color = blockColors[blockType];
  
  // Calculate screen position
  int screenX = GRID_X_OFFSET + (x * BLOCK_SIZE);
  int screenY = GRID_Y_OFFSET + (y * BLOCK_SIZE);
  
  // Draw filled block
  tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, color);
  
  // Draw highlight (lighter inner edge)
  tft.drawFastHLine(screenX, screenY, BLOCK_SIZE - 1, color + 0x1084);
  tft.drawFastVLine(screenX, screenY, BLOCK_SIZE - 1, color + 0x1084);
  
  // Draw shadow (darker outer edge)
  tft.drawFastHLine(screenX, screenY + BLOCK_SIZE - 1, BLOCK_SIZE, color - 0x1084);
  tft.drawFastVLine(screenX + BLOCK_SIZE - 1, screenY, BLOCK_SIZE, color - 0x1084);
}

void drawGrid() {
  // Draw placed blocks
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] > 0) {
        drawBlock(x, y, grid[y][x]);
      } else {
        // Draw empty cell
        tft.fillRect(
          GRID_X_OFFSET + (x * BLOCK_SIZE),
          GRID_Y_OFFSET + (y * BLOCK_SIZE),
          BLOCK_SIZE, BLOCK_SIZE, TFT_BLACK);
      }
    }
  }
  
  // Draw current piece
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentPiece[y][x] > 0) {
        int gridX = currentX + x;
        int gridY = currentY + y;
        if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
          drawBlock(gridX, gridY, currentPiece[y][x]);
        }
      }
    }
  }
}

void drawNextPiece() {
  // Clear next piece area
  tft.fillRect(
    GRID_X_OFFSET + GRID_WIDTH * BLOCK_SIZE + 11,
    GRID_Y_OFFSET + 1,
    6 * BLOCK_SIZE - 2,
    6 * BLOCK_SIZE - 2,
    TFT_BLACK);
  
  // Draw next piece
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (nextPiece[y][x] > 0) {
        int screenX = GRID_X_OFFSET + GRID_WIDTH * BLOCK_SIZE + 20 + (x * BLOCK_SIZE);
        int screenY = GRID_Y_OFFSET + 20 + (y * BLOCK_SIZE);
        
        uint16_t color = blockColors[nextPiece[y][x]];
        
        // Draw filled block
        tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, color);
        
        // Draw highlight (lighter inner edge)
        tft.drawFastHLine(screenX, screenY, BLOCK_SIZE - 1, color + 0x1084);
        tft.drawFastVLine(screenX, screenY, BLOCK_SIZE - 1, color + 0x1084);
        
        // Draw shadow (darker outer edge)
        tft.drawFastHLine(screenX, screenY + BLOCK_SIZE - 1, BLOCK_SIZE, color - 0x1084);
        tft.drawFastVLine(screenX + BLOCK_SIZE - 1, screenY, BLOCK_SIZE, color - 0x1084);
      }
    }
  }
}

void loadPiece(uint8_t piece[4][4], int pieceType) {
  // Copy tetromino shape from progmem to piece array
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      piece[y][x] = pgm_read_byte(&(tetrominos[pieceType][y][x]));
    }
  }
}

void spawnNewPiece() {
  // Move next piece to current piece
  currentPieceType = nextPieceType;
  memcpy(currentPiece, nextPiece, sizeof(currentPiece));
  
  // Generate new next piece
  nextPieceType = random(7);
  loadPiece(nextPiece, nextPieceType);
  
  // Set starting position
  currentX = GRID_WIDTH / 2 - 2;
  currentY = 0;
  currentRotation = 0;
  
  // Check if game is over (collision at spawn)
  if (checkCollision(currentX, currentY, currentPiece)) {
    gameOver = true;
  }
  
  drawNextPiece();
  drawGrid();
}

boolean checkCollision(int x, int y, uint8_t piece[4][4]) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (piece[py][px] > 0) {
        int gridX = x + px;
        int gridY = y + py;
        
        // Check boundaries
        if (gridX < 0 || gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) {
          return true;
        }
        
        // Check for collisions with existing blocks (only if within grid)
        if (gridY >= 0 && grid[gridY][gridX] > 0) {
          return true;
        }
      }
    }
  }
  return false;
}

void rotatePiece() {
  // Create temporary piece for rotation
  uint8_t tempPiece[4][4] = {0};
  
  // Special case for O piece (doesn't rotate)
  if (currentPieceType == 3) {
    return;
  }
  
  // Rotate 90 degrees clockwise
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      tempPiece[x][3 - y] = currentPiece[y][x];
    }
  }
  
  // Check if rotation is valid
  if (!checkCollision(currentX, currentY, tempPiece)) {
    // Apply rotation
    memcpy(currentPiece, tempPiece, sizeof(currentPiece));
    currentRotation = (currentRotation + 1) % 4;
    drawGrid();
  }
}

boolean movePieceLeft() {
  if (!checkCollision(currentX - 1, currentY, currentPiece)) {
    currentX--;
    drawGrid();
    return true;
  }
  return false;
}

boolean movePieceRight() {
  if (!checkCollision(currentX + 1, currentY, currentPiece)) {
    currentX++;
    drawGrid();
    return true;
  }
  return false;
}

boolean movePieceDown() {
  if (!checkCollision(currentX, currentY + 1, currentPiece)) {
    currentY++;
    drawGrid();
    return true;
  }
  return false;
}

void dropPiece() {
  while (movePieceDown()) {
    // Keep moving down until collision
  }
  placePiece();
  checkLines();
  spawnNewPiece();
}

void placePiece() {
  // Add current piece to the grid
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentPiece[y][x] > 0) {
        int gridX = currentX + x;
        int gridY = currentY + y;
        if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
          grid[gridY][gridX] = currentPiece[y][x];
        }
      }
    }
  }
}

void checkLines() {
  int linesCompleted = 0;
  
  for (int y = 0; y < GRID_HEIGHT; y++) {
    boolean lineComplete = true;
    
    // Check if this line is complete
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == 0) {
        lineComplete = false;
        break;
      }
    }
    
    if (lineComplete) {
      linesCompleted++;
      
      // Flash the line
      for (int i = 0; i < 3; i++) {
        // Fill line with white
        for (int x = 0; x < GRID_WIDTH; x++) {
          tft.fillRect(
            GRID_X_OFFSET + (x * BLOCK_SIZE),
            GRID_Y_OFFSET + (y * BLOCK_SIZE),
            BLOCK_SIZE, BLOCK_SIZE, TFT_WHITE);
        }
        delay(50);
        
        // Fill line with black
        for (int x = 0; x < GRID_WIDTH; x++) {
          tft.fillRect(
            GRID_X_OFFSET + (x * BLOCK_SIZE),
            GRID_Y_OFFSET + (y * BLOCK_SIZE),
            BLOCK_SIZE, BLOCK_SIZE, TFT_BLACK);
        }
        delay(50);
      }
      
      // Move all lines above down
      for (int moveY = y; moveY > 0; moveY--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          grid[moveY][x] = grid[moveY - 1][x];
        }
      }
      
      // Clear top line
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[0][x] = 0;
      }
    }
  }
  
  // Update score and level
  if (linesCompleted > 0) {
    // Classic Tetris scoring system
    switch (linesCompleted) {
      case 1:
        score += 40 * level;
        break;
      case 2:
        score += 100 * level;
        break;
      case 3:
        score += 300 * level;
        break;
      case 4:
        score += 1200 * level;
        break;
    }
    
    linesCleared += linesCompleted;
    
    // Level up every 10 lines
    level = 1 + (linesCleared / 10);
    
    updateScoreDisplay();
    drawGrid();
  }
}

void handleJoystickInput() {
  int xVal = analogRead(JOYSTICK_X_PIN);
  int yVal = analogRead(JOYSTICK_Y_PIN);
  
  // Debounce
  if (millis() - lastInputTime < 150) return;
  
  // Arduino Uno analog range is 0-1023 (10-bit)
  if (xVal < 300) { // Left movement
    lastInputTime = millis();
    movePieceLeft();
  }
  else if (xVal > 700) { // Right movement
    lastInputTime = millis();
    movePieceRight();
  }
  
  if (yVal < 300) { // Up movement (rotate)
    lastInputTime = millis();
    rotatePiece();
  }
  else if (yVal > 700) { // Down movement (faster drop)
    lastInputTime = millis();
    movePieceDown();
  }
  
  // Quick drop (center press) - simulated by simultaneous extreme values
  if ((xVal > 700 && yVal > 700) || (xVal < 300 && yVal < 300)) {
    dropPiece();
  }
}