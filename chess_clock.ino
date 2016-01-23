
#include <math.h> // round()
/* 

port_0 :
  595 #1 data
  ------
    Q0 : A (11)
    Q1 : B (7)
    Q2 : C (4)
    Q3 : D (2)
    Q4 : E (1)
    Q5 : F (10)
    Q6 : G (5)
    Q7 :DP (3)
  
  595 #2 data
  ------
    Q0 : A (11)
    Q1 : B (7)
    Q2 : C (4)
    Q3 : D (2)
    Q4 : E (1)
    Q5 : F (10)
    Q6 : G (5)
    Q7 :DP (3)
    
port_2 : sev #1 dig_1 (12)
port_3 : sev #1 dig_2 (9)
port_4 : sev #1 dig_3 (8)
port_5 : sev #1 dig_4 (6)

port_6 : sev #2 dig_1 (12)
port_7 : sev #2 dig_2 (9)
port_8 : sev #2 dig_3 (8)
port_9 : sev #2 dig_4 (6)

port_10 : clock (11)
port_11 : latch (12)

port_12 : 
port_13 : 

*/

// pins
int potPin = A0; // A0
int button1 = 12;
int button2 = 1;
int buzzerPin = 13;
int led1 = A4;
int led2 = A5;

// shift register pins
int shiftReg = 0;
int clkPin = 10;
int latPin = 11;
int centerButton = A3;

#define NONE 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2
#define BOTH_PLAYERS 3

typedef enum GAME_STATE {
 PREGAME,
 INGAME,
 POSTGAME 
} GAME_STATE;

int gameState = PREGAME;

typedef struct PLAYER {
  int player;
  int time;
  int prevTime;
  int buttonState;
  int sevSeg[4]; // pins for the sev seg display
} PLAYER;

PLAYER playerOne = {PLAYER_ONE, 0, 0, LOW, {2,5,4,3}};
PLAYER playerTwo = {PLAYER_TWO, 0, 0, LOW, {6,9,8,7}};

int turnPlayer = PLAYER_ONE;

enum DIGITS {
  
  ZERO = 252,
  ONE = 96,
  TWO = 218,
  THREE = 242,
  FOUR = 102,
  FIVE = 182,
  SIX = 190,
  SEVEN = 224,
  EIGHT = 254,
  NINE = 246,
  E_CHAR = 158,
  L_CHAR = 28,
  T_CHAR = 30,
  EMPTY_CHAR = 0,
  P_CHAR = 206,
  N_CHAR = 236
  
} sevSegDigits[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, E_CHAR, L_CHAR, T_CHAR, EMPTY_CHAR, P_CHAR, N_CHAR};

#define E 10
#define L 11
#define T 12
#define O 0 // letter 'o' as zero
#define S 5
#define P 14
#define I 1
#define N 15
#define BLANK 13

typedef enum BUTTON_SELECT_DURATION {
    NO_PRESS,
    SHORT_PRESS,
    LONG_PRESS
} BUTTON_SELECT_DURATION;
int clockTick = 0;

void setup() {
  
  ///////////////////////////////
  // set up all the digital pins
  ///////////////////////////////
  pinMode(shiftReg, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(centerButton, INPUT_PULLUP);
  
  // initialize button states
  playerOne.buttonState = digitalRead(button1);
  playerTwo.buttonState = digitalRead(button2);
  
  for (int i = 0; i < 4; i++) {
    pinMode(playerOne.sevSeg[i], OUTPUT);
    pinMode(playerTwo.sevSeg[i], OUTPUT);
    // initialize sevseg states
    digitalWrite(playerOne.sevSeg[i], HIGH); // LOW selects digit
    digitalWrite(playerTwo.sevSeg[i], HIGH); // LOW selects digit
  }
  pinMode(clkPin, OUTPUT);
  pinMode(latPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  
  int clockTick = millis();
}

void loop() {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  
  int centerVal = checkCenterButton();
  switch (centerVal)
  {
    case SHORT_PRESS:
      break;
    case LONG_PRESS:
      digitalWrite(buzzerPin, HIGH);
      break;
    default:
      digitalWrite(buzzerPin, LOW);
      break;
  }
  
  int change;
  switch (gameState) {
    case PREGAME :
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      // Get the corresponding time for knob value
      playerOne.time = playerTwo.time = readPotentiometer();
      // Send it to each display
      displayNumber();
      // Check for a button push
      change = buttonChanged();
      if (change == PLAYER_ONE) {
        turnPlayer = PLAYER_TWO;
        gameState = INGAME;
      } else if (change == PLAYER_TWO) {
        turnPlayer = PLAYER_ONE;
        gameState = INGAME;
      }
      
      break;
      
    case INGAME :
    
      // Check for reset
      if (checkCenterButton() == LONG_PRESS) {
        gameState = PREGAME;
      }
    
      // Decrement clock if necessary
      if (oneSecondHasPassed()) {
        decrementTurnPlayerTime();
      }
      displayNumber();
      // See if the turn player has lost
      if (checkForTimeout()) {
        gameState = POSTGAME;
      }
      // See if the player's turn has ended
      change = buttonChanged();
      if (change == turnPlayer || change == BOTH_PLAYERS ) {
        nextPlayerTurn();
      }
      break;
      
    case POSTGAME :
      displayNumber();
      digitalWrite(buzzerPin, HIGH);
      delayMicroseconds(1000);
      digitalWrite(buzzerPin, LOW);
      delayMicroseconds(1000);
      
      if (buttonChanged() != NONE) {
        gameState = PREGAME;
      }
      break;
      
  }
}

int checkCenterButton(void) {
  static int lastLow = millis();
  
  int val = digitalRead(centerButton);
  if (val == HIGH) {
    if ((millis() - lastLow) > 2000) {
        return LONG_PRESS;
    }
    if ((millis() - lastLow) > 20) {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      return SHORT_PRESS;
    }
  } else {  
      lastLow = millis();
  }
  
  return NO_PRESS;
  
  
}

boolean oneSecondHasPassed(void) {
  
  int current = millis();
  
  if ( (current - clockTick) > 999) {
    clockTick = current;
    return true;
  }
  return false;
}

void decrementTurnPlayerTime(void) {
  switch (turnPlayer) {
    case PLAYER_ONE :
      playerOne.time--;
      break;
    case PLAYER_TWO :
      playerTwo.time--;
      break;
  }
}

boolean checkForTimeout(void) {
  if (playerOne.time <= 0) {
    return true;
  } else if (playerTwo.time <= 0 ) {
    return true;    
  }
  return false;
}

void nextPlayerTurn(void) {
  
  clockTick = millis();
  
  switch (turnPlayer) {
    case PLAYER_ONE :
      turnPlayer = PLAYER_TWO;
      break;
    case PLAYER_TWO :
      turnPlayer = PLAYER_ONE;
      break;
    }
}

int buttonChanged(void) {
  int changed = NONE;
  
  if ( digitalRead(button1) != playerOne.buttonState) {
      playerOne.buttonState = digitalRead(button1);
      changed = PLAYER_ONE;
  }
  if ( digitalRead(button2) != playerTwo.buttonState) {
     playerTwo.buttonState = digitalRead(button2);
     changed += PLAYER_TWO;
  }
   
  return changed;
}

void displayNumber(void) {
  
  int digit[8];
  // player one
  if (playerOne.time > 0) {
    digit[0] = playerOne.time % 10;
    digit[1] = (playerOne.time % 100) / 10;
    digit[2] = (playerOne.time % 1000) / 100;
    digit[3] = (playerOne.time % 10000) / 1000;
  } else {
    //digitalWrite(led2, HIGH);
    convertZeroToPhrase(&digit[0]);
  }
  // player two
  if (playerTwo.time > 0) {
    digit[4] = playerTwo.time % 10;
    digit[5] = (playerTwo.time % 100) / 10;
    digit[6] = (playerTwo.time % 1000) / 100;
    digit[7] = (playerTwo.time % 10000) / 1000;
  } else {
    convertZeroToPhrase(&digit[4]);
    //digitalWrite(led1, HIGH);
  }
  
    for (int n = 0; n < 8; n++ ) {

      digit[n] = sevSegDigits[digit[n]];
    }
  
  for (int i = 0; i < 4; i++ ) {
    
    digitalWrite(playerOne.sevSeg[i], LOW);
    shiftOut(shiftReg, clkPin, LSBFIRST, digit[i+4]);
    digitalWrite(playerOne.sevSeg[i], HIGH);
    
    digitalWrite(playerTwo.sevSeg[i], LOW);
    shiftOut(shiftReg, clkPin, LSBFIRST, digit[i]);
    digitalWrite(playerTwo.sevSeg[i], HIGH);
    
    digitalWrite(latPin, LOW);
    digitalWrite(latPin, HIGH);    
  }
  
}

void convertZeroToPhrase(int* arr) {
  switch (gameState) {
    case PREGAME :
      // SET
      arr[3] = S;
      arr[2] = P;
      arr[1] = I;
      arr[0] = N;
      break;
    case POSTGAME :
      // LOSE
      arr[3] = L;
      arr[2] = O;
      arr[1] = S;
      arr[0] = E;
      break;
  }
}

int readPotentiometer() {
  
  int val = analogRead(potPin);
  int seconds;
  int interval;
  
  for ( int i = 1; i < 36; i++ ) {

    interval = i+36-(2*i);
    interval = round(13.86 * (interval + 1));
    
    if ( val > interval ) {
      if (i < 18) {
        seconds = i * 30 + 90;
      } else if (i < 28) {
        seconds = ((i-17) * 60 ) + 600;
      } else {
        seconds = ((i-27) * 300) + 1200;
      }
      
      return seconds;
    }
  }
  
}


