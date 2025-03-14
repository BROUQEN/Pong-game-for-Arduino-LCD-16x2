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

// game settings ( Players: <1,2>, AI DIfficulty: <1,9> )
int playersNumber = 1, aiDifficulty = 5;

String gamemode = "welcome";
// gamemodes: welcome, menu, menu2, game

// print blinking text function
void blinkText(String text, int column, int row, int time) {
    lcd.setCursor(column, row);
    if (millis() / time % 2) {
        for (int i = 0; i < text.length(); i++) {
            lcd.print(" "); // Print i long blank space where i is text length
        }
    } else {
        lcd.print(text); // Print text
    }
}

// function to print is here so less spaghetti is bellow in loop
void setupWelcomeScreen(){
  lcd.setCursor(0,0);
  lcd.print("Welcome to Pong!");
  lcd.setCursor(0,1);
  lcd.print("press start...  ");
}

// function to print is here so less spaghetti is bellow in loop
void setupMenuScreen(){
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
void welcomeLoop(){
  Serial.println("================");
  Serial.println("WELCOME IN PONG!");
  Serial.println("================");
  // loop with limited actions
  while (gamemode == "welcome"){ 
    // isMenuOppened = isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart);
    if (isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart)){
      gamemode = "menu";
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
  while (gamemode == "menu"){
    updateMenuScreen();
    lcd.setCursor(3, 1);
    lcd.print("Players: ");
    lcd.print(playersNumber);

    if (isButtonPressedInMenu(leftButton, lastReadingLeft, stableLeft, lastDebounceTimeLeft)){
      playersNumber--;
      if (playersNumber < 1){
        playersNumber = 1;
      }
    }
    
    if (isButtonPressedInMenu(rightButton, lastReadingRight, stableRight, lastDebounceTimeRight)){
      playersNumber++;
      if (playersNumber > 2){
        playersNumber = 2;
      }
    }

    if (isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart)){
      if (playersNumber == 1){
        gamemode = "menu2";
        Serial.println(playersNumber);
        Serial.print("AI power: ");
        setupMenuScreen();
      } else {
        gamemode = "game";
        Serial.println(playersNumber);
        Serial.println("================");
        Serial.println("");
      }
    }
    
  }

  // if playersNumber == 1 player may select difficulty
  while (gamemode == "menu2"){
    updateMenuScreen();

      lcd.setCursor(3, 1);
      lcd.print("AI LVL: ");
      lcd.print(aiDifficulty);

    if (isButtonPressedInMenu(leftButton, lastReadingLeft, stableLeft, lastDebounceTimeLeft)){
      aiDifficulty--;
      if (aiDifficulty < 1){
        aiDifficulty = 1;
      }
    }
    
    if (isButtonPressedInMenu(rightButton, lastReadingRight, stableRight, lastDebounceTimeRight)){
      aiDifficulty++;
      if (aiDifficulty > 9){
        aiDifficulty = 9;
      }
    }

    if (isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart)){
      gamemode = "game";
        Serial.println(aiDifficulty);
        Serial.println("================");
        Serial.println("");
    }
    
  }
}

// gamemode == game
void gameLoop(){
  Serial.println("In game");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("here i will");
  lcd.setCursor(0, 1);
  lcd.print("rewrite the game");
  while (gamemode == "game"){

  }
}

void setup() {
  Serial.begin(9600);

  pinMode(startButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print a welcome message to the LCD.
  setupWelcomeScreen();
}

void loop() {

  welcomeLoop();
  menuLoop();
  gameLoop();

  // I save this for later (menu button detection to copy):

  // isButtonPressedInMenu(startButton, lastReadingStart, stableStart, lastDebounceTimeStart);
  // isButtonPressedInMenu(leftButton, lastReadingLeft, stableLeft, lastDebounceTimeLeft);
  // isButtonPressedInMenu(rightButton, lastReadingRight, stableRight, lastDebounceTimeRight);

}

