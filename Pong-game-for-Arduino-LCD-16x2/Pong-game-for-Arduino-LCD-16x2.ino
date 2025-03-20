/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// select pins for buttons
const int startButton = A0, leftButton = A1, rightButton = A2;

// buttons work different in menu and in game to make game 100% fair
// those variables are used for menu actions
int lastReadingStart = LOW, stableStart = LOW;
int lastReadingLeft = LOW, stableLeft = LOW;
int lastReadingRight = LOW, stableRight = LOW;

unsigned long lastDebounceTimeStart = 0, lastDebounceTimeLeft = 0, lastDebounceTimeRight = 0;
const unsigned long debounceDelay = 50;

// game settings ( Players: <1,2>, PC DIfficulty: <1,9> )
int playersNumber = 1, pcDifficulty = 5;

// Define game modes using an enum
enum GameMode {
  WELCOME,
  MENU,
  MENU2,
  GAME
};

// Initialize the gamemode as WELCOME at the start
GameMode gamemode = WELCOME;

// game info
const int defaultGameDelay = 200, scoredDelay = 1000; // delay in ms
int pointsLeft = 0, pointsRight = 0, gameDelay = defaultGameDelay;

// print blinking text function
// every time [s] change state
void blinkText(String text, int column, int row, int time) {
    lcd.setCursor(column, row);
    // every second time [s] passes clear message
    if (millis() / time % 2) {
        for (int i = 0; i < text.length(); i++) {
          // Print i long blank space where i is text length
          lcd.print(" ");
        }
    } else {
      // print text again after time [s] passed
      lcd.print(text);
    }
}

// function to print is here so less spaghetti is bellow in loop
void setupWelcomeScreen() {
  lcd.setCursor(0,0);
  lcd.print("Welcome to Pong!");
  lcd.setCursor(0,1);
  lcd.print("press start...  ");
}

// function to print is here so less spaghetti is bellow in loop
void setupMenuScreen() {
  lcd.setCursor(0,0);
  lcd.print("LB|SETTINGS  |RB");
  lcd.setCursor(0,1);
  lcd.print("  |          |  ");
}

// print blinking arrows to suggest player may use menu with buttons
void updateMenuScreen(){
  blinkText("<<", 0, 1, 1000); // left arrows
  blinkText(">>", 14, 1, 1000); // right arrows
}

// THIS BUTTON DETECTION IS ONLY FOR MENU! game uses different input detection for fair gameplay
bool isButtonPressedInMenu(int buttonPin, int &lastReading, int &stableState, unsigned long &lastDebounceTime) {
  int reading = digitalRead(buttonPin);
  if (reading != lastReading) {
    lastDebounceTime = millis();
    lastReading = reading;
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == HIGH) {
        return true;
      }
    }
  }
  return false;
}

// Welcome screen
void welcomeLoop() {
  Serial.println("================");
  Serial.println("WELCOME TO PONG!");
  Serial.println("================");
  // loop with limited actions
  while (gamemode == WELCOME) { 
    // isMenuOppened = isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart);
    if (isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart)) {
      gamemode = MENU;
    }
    blinkText("Press start...", 0, 1, 1000); // blinking text at column 0, row 1, 1000ms seen / 1000ms hidden
  }
}

// Menu screen
void menuLoop(){
  Serial.println("SETTINGS        ");
  Serial.print("Players: ");
  setupMenuScreen();

  // loop with limited actions
  while (gamemode == MENU) {
    updateMenuScreen();
    lcd.setCursor(3, 1);
    lcd.print("Players: ");
    lcd.print(playersNumber);

    // if left button is pressed then show previous option
    if (isButtonPressedInMenu(leftButton, lastReadingLeft, stableLeft, lastDebounceTimeLeft)) {
      playersNumber--;
      if (playersNumber < 1) {
        playersNumber = 1;
      }
    }
    
    // if right button is pressed then show next option
    if (isButtonPressedInMenu(rightButton, lastReadingRight, stableRight, lastDebounceTimeRight)) {
      playersNumber++;
      if (playersNumber > 2){
        playersNumber = 2;
      }
    }

    // if start button is pressed then select option and
    if (isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart)) {
      if (playersNumber == 1) {
        // select pcPower
        gamemode = MENU2;
        Serial.println(playersNumber);
        Serial.print("PC lvl: ");
        setupMenuScreen();
      } else if (playersNumber == 2) {
        // play PvP
        gamemode = GAME;
        Serial.println(playersNumber);
        Serial.println("================");
        Serial.println("");
      }
    }
    
  }

  // if playersNumber == 1 player may select difficulty
  while (gamemode == MENU2) {
    updateMenuScreen();

      lcd.setCursor(3, 1);
      lcd.print("PC LVL: ");
      lcd.print(pcDifficulty);

    // if left button is pressed then show previous option
    if (isButtonPressedInMenu(leftButton, lastReadingLeft, stableLeft, lastDebounceTimeLeft)) {
      pcDifficulty--;
      if (pcDifficulty < 1){
        pcDifficulty = 1;
      }
    }
    
    // if right button is pressed then show next option
    if (isButtonPressedInMenu(rightButton, lastReadingRight, stableRight, lastDebounceTimeRight)) {
      pcDifficulty++;
      if (pcDifficulty > 9){
        pcDifficulty = 9;
      }
    }

    // if player hits start then its P1vPC game
    if (isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart)){
      gamemode = GAME;
        Serial.println(pcDifficulty);
        Serial.println("================");
        Serial.println("");
    }
    
  }
}

bool isLeftButtonPressed() {
  // if button is pressed print paddle in top lane and bottom lane
  if (digitalRead(leftButton) == HIGH) {
    lcd.setCursor(2, 1);
    lcd.print(" ");
    lcd.setCursor(2, 0);
    lcd.print("|");
    return true;
  } else {
    // print paddle in bottom lane and clear top lane
    lcd.setCursor(2, 0);
    lcd.print(" ");
    lcd.setCursor(2, 1);
    lcd.print("|");
    return false;
  }
}

bool isRightButtonPressed(){
  // if button is pressed print paddle in top lane and bottom lane
  if (digitalRead(rightButton) == HIGH) {
    lcd.setCursor(13, 1);
    lcd.print(" ");
    lcd.setCursor(13, 0);
    lcd.print("|");
    return true;
  } else {
    // print paddle in bottom lane and clear top lane
    lcd.setCursor(13, 0);
    lcd.print(" ");
    lcd.setCursor(13, 1);
    lcd.print("|");
    return false;
  }
}

void leftPlayerScored() {
  // when points contain one nuber print points far right
  if (++pointsLeft <= 9) {
    lcd.setCursor(1, 1);
  } else if (pointsLeft <= 99) {
    lcd.setCursor(0, 1);
  } else {
    // points are beyond max visible value of 99
    pointsLeft = 1;
    lcd.setCursor(0, 1);
    lcd.print("00");
    lcd.setCursor(1, 1);
  }
  
  lcd.print(pointsLeft);
  gameDelay = defaultGameDelay;

  // print P1 scored message
  lcd.setCursor(3, 0);  
  lcd.print("P1 SCORED!");
  lcd.setCursor(3, 1);  
  lcd.print("          ");
  Serial.println("P1 scored: " + String(pointsLeft) + " : " + String(pointsRight));
  delay(scoredDelay);
  lcd.setCursor(3, 0);  
  lcd.print("          ");
}

void rightPlayerScored() {
  // when points contain one nuber print points far right
  if (++pointsRight <= 9) {
    lcd.setCursor(15, 1);
  } else if (pointsRight <= 99) {
    lcd.setCursor(14, 1);
  } else {
    // points are beyond max visible value of 99
    pointsRight = 1;
    lcd.setCursor(14, 1);
    lcd.print("00");
    lcd.setCursor(15, 1);
  }

  lcd.print(pointsRight);
  gameDelay = defaultGameDelay;

  // print P2 scored message
  lcd.setCursor(3, 0);  
  lcd.print("P2 SCORED!");
  lcd.setCursor(3, 1);  
  lcd.print("          ");
  Serial.println("P2 scored: " + String(pointsLeft) + " : " + String(pointsRight));
  delay(scoredDelay);
  lcd.setCursor(3, 0);  
  lcd.print("          ");
}

// PC logic
bool pcLost() {
  int pcLost = random(1, pcDifficulty * 5);
  if (pcLost == 1) {
    return true;
  } else {
    return false;
  }
}

// ball is traveling from left to right
void pathRight(int row) {
  // clear screen from old ball
  lcd.setCursor(3, 0);
  lcd.print(" ");
  lcd.setCursor(3, 1);
  lcd.print(" ");

  for (int column = 4; column <= 12; column++) {
    lcd.setCursor(column, row);
    lcd.print("O");
    delay(gameDelay);
    lcd.setCursor(column, row);
    lcd.print(" ");
    
    // those functions works only as display 
    isLeftButtonPressed();
    if (playersNumber == 2) {
      isRightButtonPressed();
    }

  }

  if (playersNumber == 1) {
    if (pcLost() == true) {
      leftPlayerScored();    
      // print paddle wrong (in bottom lane and clear top lane)
      if (row == 0) {
        lcd.setCursor(13, 0);
        lcd.print(" ");
        lcd.setCursor(13, 1);
        lcd.print("|");
      } else {
        // print paddle wrong (in top lane and clear bottom lane)
        lcd.setCursor(13, 1);
        lcd.print(" ");
        lcd.setCursor(13, 0);
        lcd.print("|");
      }
    } else {
      // print paddle right (in top lane and clear bottom lane)
      if (row == 0) {
        lcd.setCursor(13, 1);
        lcd.print(" ");
        lcd.setCursor(13, 0);
        lcd.print("|");
      } else {
        // print paddle right (in bottom lane and clear top lane)
        lcd.setCursor(13, 0);
        lcd.print(" ");
        lcd.setCursor(13, 1);
        lcd.print("|");
      }
    }

  } else {
    if (row == 0) {
      if (isRightButtonPressed() == false) {
        leftPlayerScored();
      }
    } else {
      if (isRightButtonPressed() == true) {
        leftPlayerScored();
      }
    }
  }
}

// ball is traveling from right to left
void pathLeft(int row) {
  // clear screen from old ball
  lcd.setCursor(12, 0);
  lcd.print(" ");
  lcd.setCursor(12, 1);
  lcd.print(" ");

  for (int column = 11; column >= 3; column--) {
    lcd.setCursor(column, row);
    lcd.print("O");
    delay(gameDelay);
    lcd.setCursor(column, row);
    lcd.print(" ");

    isLeftButtonPressed();

    if (playersNumber == 2){
      isRightButtonPressed();
    }

  }

  if (row == 0) {
    if (isLeftButtonPressed() == false) {
      rightPlayerScored();
    }
  } else {
    if (isLeftButtonPressed() == true) {
      rightPlayerScored();
    }
  }
}

// gamemode == game
void gameLoop() {
  // setup game screen
  Serial.println("In game");
  lcd.clear();
  lcd.setCursor(0, 0);

  if (playersNumber == 1) {
    // print first paddle for pc
    lcd.print("P1           |PC");
  } else {
    lcd.print("P1            P2");
  }

  lcd.setCursor(0, 1);
  lcd.print("00            00");

  while (gamemode == GAME) {
    // random row: 0 is top line and 1 is bottom line
    pathLeft(random(0,2));
    // increase ball speed randomly
    gameDelay = random(gameDelay - 10, gameDelay);
    pathRight(random(0,2));
    gameDelay = random(gameDelay - 10, gameDelay);

  }
}

void setup() {
  Serial.begin(9600);

  pinMode(startButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop() {
  // Print a welcome message to the LCD.
  setupWelcomeScreen();

  welcomeLoop();
  menuLoop();
  gameLoop();

  // I save this for later (menu button detection to copy):

  // isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart);
  // isButtonPressedInMenu(leftButton, lastReadingLeft, stableLeft, lastDebounceTimeLeft);
  // isButtonPressedInMenu(rightButton, lastReadingRight, stableRight, lastDebounceTimeRight);

}