/*
 A simple Pong game for lcd 16x2
 Original code at:
 https://github.com/BROUQEN/Pong-game-for-Arduino-LCD-16x2.git

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
const int START_BUTTON = A0, LEFT_BUTTON = A1, RIGHT_BUTTON = A2;

// buttons work different in menu and in game to make game 100% fair
// those variables are used for menu actions
int lastReadingStart = LOW, stableStart = LOW;
int lastReadingLeft = LOW, stableLeft = LOW;
int lastReadingRight = LOW, stableRight = LOW;

unsigned long lastDebounceTimeStart = 0, lastDebounceTimeLeft = 0, lastDebounceTimeRight = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// game settings ( Players: <1,2>, PC DIfficulty: <1,9> )
int playersNumber = 1, pcDifficulty = 5;

// game info
const int DEFAULT_GAME_DELAY = 200, SCORED_DELAY = 1000; // delay in ms
int pointsLeft = 0, pointsRight = 0, gameRound = 0, bounces = 0, gameDelay = DEFAULT_GAME_DELAY;

// Define game modes using an enum
enum GameMode {
  WELCOME,
  MENU,
  MENU2,
  GAME
};

// Initialize the gamemode as WELCOME at the start
GameMode gamemode = WELCOME;

// print blinking text function. every time [s] change state
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

// print blinking arrows to suggest player may use menu with buttons
void updateMenuScreen(int &value){
  blinkText("<<", 0, 1, 1000); // left arrows
  blinkText(">>", 14, 1, 1000); // right arrows
  lcd.setCursor(12, 1);
  lcd.print(value);
}

// THIS BUTTON DETECTION IS ONLY FOR MENU! game uses different input detection for fair gameplay
bool isButtonPressedInMenu(int buttonPin, int &lastReading, int &stableState, unsigned long &lastDebounceTime) {
  int reading = digitalRead(buttonPin);
  if (reading != lastReading) {
    lastDebounceTime = millis();
    lastReading = reading;
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == HIGH) {
        return true;
      }
    }
  }
  return false;
}

// if left button: value-- if right button: value++
void updateValue(int &value, int minVal, int maxVal) {
  if (isButtonPressedInMenu(LEFT_BUTTON, lastReadingLeft, stableLeft, lastDebounceTimeLeft)) {
    value = max(minVal, value - 1);
  }

  if (isButtonPressedInMenu(RIGHT_BUTTON, lastReadingRight, stableRight, lastDebounceTimeRight)) {
    value = min(maxVal, value + 1);
  }
}

bool isLeftButtonPressed() {
  // if button is pressed print paddle in top lane and bottom lane
  if (digitalRead(LEFT_BUTTON) == HIGH) {
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
  if (digitalRead(RIGHT_BUTTON) == HIGH) {
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
  gameDelay = DEFAULT_GAME_DELAY;
  bounces = 0;
  gameRound++;

  // print P1 scored message
  lcd.setCursor(3, 0);  
  lcd.print("P1 SCORED!");
  lcd.setCursor(3, 1);  
  lcd.print("          ");
  Serial.println("P1 scored: " + String(pointsLeft) + " : " + String(pointsRight));
  delay(SCORED_DELAY);
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
  gameDelay = DEFAULT_GAME_DELAY;
  bounces = 0;
  gameRound++;

  // print P2 scored message
  lcd.setCursor(3, 0);  
  lcd.print("P2 SCORED!");
  lcd.setCursor(3, 1);  
  lcd.print("          ");
  Serial.println("P2 scored: " + String(pointsLeft) + " : " + String(pointsRight));
  delay(SCORED_DELAY);
  lcd.setCursor(3, 0);  
  lcd.print("          ");
}

// PC logic
bool pcLost() {
  if ((bounces*2) >= pcDifficulty) {
    int pcLost = random(1, pcDifficulty * 3);
    if (pcLost == 1) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}

// ball is traveling from left to right
void pathRight(int row) {
  for (int column = 4; column <= 12; column++) {
    lcd.setCursor(column, row);
    lcd.print("O");
    // those functions works only as display 
    isLeftButtonPressed();
    if (playersNumber == 2) {
      isRightButtonPressed();
    }

    delay(gameDelay);
    lcd.setCursor(column, row);
    lcd.print(" ");
  }

  if (playersNumber == 1) {
    if (pcLost() == true) {   
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
      // after printing the paddle notice about score change
      leftPlayerScored();  
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
  for (int column = 11; column >= 3; column--) {
    lcd.setCursor(column, row);
    lcd.print("O");
    // those functions works only as display 
    isLeftButtonPressed();
    if (playersNumber == 2){
      isRightButtonPressed();
    }

    delay(gameDelay);
    lcd.setCursor(column, row);
    lcd.print(" ");
  }

  if (row == 0) {
    if (isLeftButtonPressed() == false) {
      rightPlayerScored();
    } else {
      bounces++;
    }
  } else {
    if (isLeftButtonPressed() == true) {
      rightPlayerScored();
    } else {
      bounces++;
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(START_BUTTON, INPUT);
  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop() {
  switch (gamemode) {
    case WELCOME:
      // setup welcome screen
      lcd.setCursor(0,0);
      lcd.print("Welcome to Pong!");
      lcd.setCursor(0,1);
      lcd.print("press start...  ");
      Serial.println("WELCOME TO PONG!");

      // loop with limited actions
      while (gamemode == WELCOME) { 
        // isMenuOppened = isButtonPressedInMenu(START_BUTTON, lastReadingStart, stableStart, lastDebounceTimeStart);
        if (isButtonPressedInMenu(START_BUTTON, lastReadingStart, stableStart, lastDebounceTimeStart)) {
          gamemode = MENU;
        }
        blinkText("Press start...", 0, 1, 1000); // blinking text at column 0, row 1, 1000ms seen / 1000ms hidden
      }
      break;
      
    case MENU:
      // setup menu screen 
      lcd.setCursor(0,0);
      lcd.print("LB|SETTINGS  |RB");
      lcd.setCursor(0,1);
      lcd.print("  |          |  ");
      Serial.println("= SETTINGS =====");
      Serial.print("Players: ");
      lcd.setCursor(3, 1);
      lcd.print("Players: ");

      // loop with limited actions
      while (gamemode == MENU) {
        updateValue(playersNumber, 1, 2); // variable, min, max
        updateMenuScreen(playersNumber);

        // if start button is pressed then select option and
        if (isButtonPressedInMenu(START_BUTTON, lastReadingStart, stableStart, lastDebounceTimeStart)) {
          // print settings info
          Serial.println(playersNumber);

          if (playersNumber == 1) {
            // select pcPower
            gamemode = MENU2;
          } else if (playersNumber == 2) {
            // play PvP
            gamemode = GAME;
          }
        }
      }
      break;
      
    case MENU2:
      // setup menu2 screen 
      lcd.setCursor(3, 1);
      lcd.print("          ");
      lcd.setCursor(3, 1);
      lcd.print("PC LVL: ");
      // print settings info
      Serial.print("PC LVL: ");

      while (gamemode == MENU2) {
        updateValue(pcDifficulty, 1, 9); // variable, min, max
        updateMenuScreen(pcDifficulty);

        if (isButtonPressedInMenu(START_BUTTON, lastReadingStart, stableStart, lastDebounceTimeStart)){
          // print settings info
          Serial.println(pcDifficulty);
          // Play PvPC
          gamemode = GAME;
        }
      }
      break;
      
    case GAME:
      // setup game screen
      Serial.println("= IN GAME ======");
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
      // game loop
      while (gamemode == GAME) {
        // random row: 0 is top line and 1 is bottom line
        pathLeft(random(0,2));
        // increase ball speed randomly
        gameDelay = random(gameDelay - 10, gameDelay);
        pathRight(random(0,2));
        gameDelay = random(gameDelay - 10, gameDelay);

      }
      break;
  }

  // I save this for later (menu button detection to copy):

  // isButtonPressedInMenu(START_BUTTON, lastReadingStart, stableStart, lastDebounceTimeStart);
  // isButtonPressedInMenu(LEFT_BUTTON, lastReadingLeft, stableLeft, lastDebounceTimeLeft);
  // isButtonPressedInMenu(RIGHT_BUTTON, lastReadingRight, stableRight, lastDebounceTimeRight);

}