/* Light game for Christmas tree v1
*  made by Bob Clagett at I Like To Make Stuff
*  
*  This is PROTOTYPE code, not meant to be fully optimized or even correct.
*  I tried to leave comments for a while, then ran out of time.
*  
*  We cannot provide any support for this code, but feel free to use it as a 
*/

#include <EasyButton.h>
#include <dmtimer.h>
#include <FastLED.h>

#define BUTTON_ONE_PIN 10 //left
#define BUTTON_ONE_LED_PIN 9//right
#define BUTTON_TWO_LED_PIN 11 //left
#define BUTTON_TWO_PIN 12//right
EasyButton button1(BUTTON_ONE_PIN);
EasyButton button2(BUTTON_TWO_PIN);

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

//setup constants and variables for gameplay
const int initGameSpeed = 500;
const bool renderGameAsText = false;

#define NUM_STRIPS 7
#define NUM_LEDS_PER_STRIP 50

bool screenSaverToggle = false;
int screenSaverTimeOut = 0;

//define gameboard matrix
const int rows = NUM_LEDS_PER_STRIP;
const int columns = NUM_STRIPS;
int gameArray[columns][rows];

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

bool queueNewGame = false;
bool gameOn = false;
int screenSaverSelector = 1;
int playerX = 3;
int gameLevel = 0;
int levelStepper = 30;  
int levelIncrease = 3; //how many clicks to condense next drop
int numToDrop = 3; //initial # to drop at once (random position, potential overlap)
int countUntilDrop = 0;
int waitForDrop = 0;
int waitForDropIncrease = 0;
int waitForDropThreshold = 5; 
int increaseDropThreshold = 35;
DMTimer myTimer(initGameSpeed);

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:

currentPalette = CloudColors_p;
    currentBlending = LINEARBLEND;
    
pinMode(BUTTON_ONE_LED_PIN, OUTPUT);
pinMode(BUTTON_TWO_LED_PIN, OUTPUT);
digitalWrite(BUTTON_ONE_LED_PIN, HIGH);
digitalWrite(BUTTON_TWO_LED_PIN, HIGH);
  //setup LEDs
   // tell FastLED there's XX NEOPIXEL leds on pin YY
  FastLED.addLeds<WS2811, 2>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 3>(leds[1], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 4>(leds[2], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 5>(leds[3], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 6>(leds[4], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 7>(leds[5], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, 8>(leds[6], NUM_LEDS_PER_STRIP);

 // Initialize the button1
  button1.begin();
  // Initialize the button2
  button2.begin();
  // Add the callback function to be called when the button1 is pressed.
  button1.onPressed(onButton1Pressed);
  // Add the callback function to be called when the button2 is pressed.
  button2.onPressed(onButton2Pressed);
  Serial.println("Ready......");
}

void loop() {
  if(myTimer.isTimeReached()){ //check if execution time has been reached
    tickTock();
  }
}

void onButton1Pressed() {
  if(queueNewGame == false){
    if(gameOn == true){
      Serial.println("left");
      if(playerX>0){
        playerX--;
      }
   } else {
    //start a new game!!!!
      Serial.println("clicked, start a new one");
      queueNewGame = true;  
    }
  }
}

void onButton2Pressed() {
  if(queueNewGame == false){
    if(gameOn == true){
      Serial.println("right");
        if(playerX<columns-1){
          playerX++;
        }
    } else {
      //start a new game!!!!
        Serial.println("clicked, start a new one");
        queueNewGame = true;  
      }
  }
}

void tickTock(){
  button1.read();
  button2.read();
  if(queueNewGame == true){
    startNewGame(); 
  }

  switch(gameOn){
    case false:
      //screensaver mode  
      stepScreensaver();
      break;
    case true:
      stepGame();
      break;
   }
  renderGameBoard();
}
void startNewGame() {
  Serial.println("HERE WE GO!");
playerX = 3;
gameLevel = 0;
levelStepper = 30;  
levelIncrease = 3; //how many clicks to condense next drop
numToDrop = 3; //initial # to drop at once (random position, potential overlap)
countUntilDrop = 0;
waitForDrop = 0;
waitForDropIncrease = 0;
waitForDropThreshold = 5; 
increaseDropThreshold = 35;

  //clear gameboard
  for (int i = columns-1; i >=0; i--) {
        for (int j = rows-1; j >= 0; j--) {
          gameArray[i][j] = 0;
        }
   }
  //reset all light colors;
  // play startup sequence
  gameOn = true;
  queueNewGame = false;
}

void stepGame(){
  //Serial.println("stepGame");
    countUntilDrop--;

    if(countUntilDrop <= 0){
        dropPiece();
    }

    for (int i = columns-1; i >=0; i--) {
      for (int j = rows-1; j >=0; j--) {
        int locValue = gameArray[i][j];
        int above = j-1;
        if(above>=0){
          
          int ceilingValue = gameArray[i][above];
          if(ceilingValue==1){
            gameArray[i][j] = ceilingValue; //inherit value from location above
            gameArray[i][above] = 0; //clear location above
            locValue = gameArray[i][j];
          } else {
            locValue = gameArray[i][j] = 0; //empties bottom row when nothing is above it 
          }
          
        }
        if(j==rows-1 && i==playerX){
          locValue = 2;//show player position
        }
         switch (locValue) {
          case 2://player
            switch(gameLevel){
              case 0:
              case 1:
              case 2:
                leds[i][j] = CRGB::Blue;
                break;
              case 3:
              case 4:
              case 5:
                leds[i][j] = CRGB::Yellow;
                break;
              case 6:
              case 7:
              case 8:
                leds[i][j] = CRGB::Violet;
                break;
              case 9:
              case 10:
              case 11:
                leds[i][j] = CRGB::Green;
                break;
              case 12:
              case 13:
              case 14:
                leds[i][j] = CRGB::Navy;
                break;
               default:
                leds[i][j] = CRGB::Red;
            }
            
            break;
          case 1:
            leds[i][j] = CRGB::White;
            break;
          case 0:
            leds[i][j] = CRGB::Black;
            break;
        }

        //check for player collision
        if(j == rows-1){
          if(gameArray[i][j]==1){
            checkCollision(i);
          }
        }
      }
    }  
    //Serial.println("tick complete");
}

void renderGameBoard() {
  if(renderGameAsText == false){
    //Serial.println("renderGameBoard");
    FastLED.show();
    delay(30);
    
  } else {
    Serial.println("----------------------");
      // This outer loop will go over each strip, one at a time
    for (int i = columns-1; i >=0; i--) {
        for (int j = rows-1; j >= 0; j--) {
        int locValue = gameArray[i][j];
        if(j==0 && i==playerX){
           Serial.print("X");
        } else {
          if(locValue == 0){
            Serial.print(" ");
          } else{
            Serial.print(locValue);
          }
        }
      }
      Serial.println("|");
    }
  }
}
void stepScreensaver() {
  //Serial.println("stepScreensaver ");
  //TODO: make this more interesting
  static uint8_t colorIndex = 0;
  screenSaverTimeOut+=1000;
 screenSaverToggle = !screenSaverToggle;
  if(screenSaverTimeOut > 10000){
     screenSaverTimeOut = 0;
     uint16_t clr = CRGB::Black;
      for(int i = 0; i < columns; i++) { 
        // This inner loop will go over each led in the current strip, one at a time
        for(int j = 0; j < rows; j++) {
          switch(screenSaverSelector){
            case 0:
              if(screenSaverToggle){
                leds[i][j] = CRGB::Red;
              } else {
                leds[i][j] = CRGB::Green;  
              }
            break;
            case 1:
            colorIndex +=3;
              leds[i][j] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
            break;
            }
          
        }
        delay(30);
      }  
  }
}

void checkCollision(int k) {
    if(playerX == k){
      //Collision found
      gameOn = false;
      playLoseAnim();
    }
  }

void dropPiece() {
  // set random top row location
  countUntilDrop = levelStepper;
  waitForDrop++;
  if(waitForDrop==waitForDropThreshold && levelStepper>=0){
    waitForDrop = 0;
    levelStepper-=levelIncrease; 
    gameLevel++; 
  }
  waitForDropIncrease++;
  if(waitForDropIncrease == increaseDropThreshold){
    waitForDropIncrease = 0;
    numToDrop++; 
  }
  if( numToDrop >5){
    numToDrop = 5;
  }
  for( int z=0;z<numToDrop;z++){
    int ranX = random(0, columns);
    gameArray[ranX][0] = 1;
  }
}

void playLoseAnim() {
  
  for(int k = 0; k < 3; k++) {
    Serial.println("YOU LOSE!");
    for(int i = 0; i < columns; i++) {
      for(int j = 0; j < rows; j++) {
        leds[i][j] = CRGB::Red; 
      }
    }
    FastLED.show();
    delay(250);
    for(int i = 0; i < columns; i++) {
      for(int j = 0; j < rows; j++) {
        leds[i][j] = CRGB::Black;
      }
    }
    FastLED.show();
    delay(250);
    //switch scrrensaver
    screenSaverSelector++;
    if (screenSaverSelector > 1){
      screenSaverSelector = 0;  
    }
  }
}
