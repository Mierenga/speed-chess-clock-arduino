/***********************************************************************
 ***********************************************************************
 ***********************************************************************
 
                              CHESS CLOCK
                        CHESS CLOCK CHESS CLOCK
                      CHESS CLOCK /|\ CHESS CLOCK
                    CHESS CLOCK  / | \  CHESS CLOCK
                  CHESS CLOCK   /  |  \   CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                  CHESS CLOCK   \  |  /   CHESS CLOCK
                    CHESS CLOCK  \ | /  CHESS CLOCK
                      CHESS CLOCK \|/ CHESS CLOCK
                        CHESS CLOCK CHESS CLOCK
                              CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK      ______________________
                CHESS CLOCK     /  _____   _____     / |
                CHESS CLOCK    /  /CLOC/  /INC_/    /  |
                CHESS CLOCK   /____________________/   |
                CHESS CLOCK  |____________________|____|
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK CHESS CLOCK CHESS CLOCK CHESS
                CLOCK CHESS CLOCK CHESS CLOCK CHESS CLOCK
                CHESS CLOCK CHESS CLOCK CHESS CLOCK CHESS
                CLOCK CHESS CLOCK CHESS CLOCK CHESS CLOCK

                              CHESS CLOCK
                        CHESS CLOCK CHESS CLOCK
                      CHESS CLOCK /|\ CHESS CLOCK
                    CHESS CLOCK  / | \  CHESS CLOCK
                  CHESS CLOCK   /  |  \   CHESS CLOCK
                CHESS CLOCK    /   |   \    CHESS CLOCK
                CHESS CLOCK   |    |    |   CHESS CLOCK
                CHESS CLOCK   |    |    |   CHESS CLOCK
                CHESS CLOCK   |    |    |   CHESS CLOCK
                CHESS CLOCK    \   |   /    CHESS CLOCK
                  CHESS CLOCK   \  |  /   CHESS CLOCK
                    CHESS CLOCK  \ | /  CHESS CLOCK
                      CHESS CLOCK \|/ CHESS CLOCK
                        CHESS CLOCK CHESS CLOCK
                              CHESS CLOCK

                              CHESS CLOCK
                        CHESS CLOCK CHESS CLOCK
                      CHESS CLOCK /|\ CHESS CLOCK
                    CHESS CLOCK  / | \  CHESS CLOCK
                  CHESS CLOCK   /  |  \   CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                CHESS CLOCK
                  CHESS CLOCK   \  |  /   CHESS CLOCK
                    CHESS CLOCK  \ | /  CHESS CLOCK
                      CHESS CLOCK \|/ CHESS CLOCK
                        CHESS CLOCK CHESS CLOCK
                              CHESS CLOCK
                              
/***********************************************************************
 ******** HARDWARE PIN CONFIGURATIONS **********************************
 ***********************************************************************
 
  port_0 : shift registers input
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

  port_1 : button 2 (on/off)
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

  port_12 : button1 (on/off)
  port_13 : buzzer
  port_A0 : potentiometer (turn knob)
  port_A3 : center button (momentary contact)
/***********************************************************************
 ***********************************************************************
 ***********************************************************************/

#include <math.h> /* round() */

// Input pins
int potPin = A0;
int button1 = 12;
int button2 = 1;
int centerButton = A3;
int buzzerPin = 13;

// shift register pins
int shiftReg = 0;
int clkPin = 10;
int latPin = 11;

// Player identification
#define PLAYER_ONE 0
#define PLAYER_TWO 1
#define NONE 2

// Input mode for knob
#define TIME true
#define INCR false

enum GameState {
  PREGAME,
  INGAME,
  POSTGAME
};

enum ButtonSelectDuration {
  NO_PRESS,
  SHORT_PRESS,
  LONG_PRESS
};

enum SevSegPhrase {
  SPIN_PHRASE,
  LOSE_PHRASE,
  SET_PHRASE,
  CLOC_PHRASE,
  INC_PHRASE
};

typedef struct Player {
  int name;      // PLAYER_ONE, PLAYER_TWO
  int time;        // time remaining before lose
  int increment;   // add to time after each turn
  int buttonState; // HI, LOW
  int sevSeg[4];   // pins for the sev seg display
} Player;


Player player[] = {{PLAYER_ONE, 0, 0, LOW, {2, 5, 4, 3}},
                  {PLAYER_TWO, 0, 0, LOW, {6, 9, 8, 7}}};

int gameState = PREGAME;
Player *turnPlayer = &player[PLAYER_ONE];
int clockTick, modeLockClock;
bool paused = false;
bool inputMode = TIME;

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
  N_CHAR = 236,
  C_CHAR = 156

} sevSegDigits[] = {
  ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
  E_CHAR, L_CHAR, T_CHAR, EMPTY_CHAR, P_CHAR, N_CHAR, C_CHAR
};

#define L 11
#define O 0 // letter 'o' as zero
#define S 5
#define E 10
#define P 14
#define I 1 // letter 'i' as one
#define N 15
#define C 16
#define T 12
#define BLANK 13

int intervalToTime[] = {0, 3600, 3300, 3000, 2700, 2400, 2100, 1800, 1500, 1200, 1140, 1080, 1020,
                        960, 900, 840, 780, 720, 660, 600, 570, 540, 510, 480, 450, 420, 390, 360,
                        330, 300, 270, 240, 210, 180, 150, 120};
                      
int intervalToIncr[] = {0, 5 , 5 , 5 , 5 , 5 , 4 , 4 , 4 , 4 , 4 , 4 , 3 , 3 , 3 , 3 , 3 , 3 , 2 ,
                        2 , 2 , 2 , 2 , 2 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0};


/*     ______________________
      /  _____   _____     / |
     /  /SET /  /UP  /    /  |
    /____________________/   |
   |____________________|____|
   
*/
void setup() {
  
  pinMode(shiftReg, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(centerButton, INPUT_PULLUP);

  // initialize button states
  player[PLAYER_ONE].buttonState = digitalRead(button1);
  player[PLAYER_TWO].buttonState = digitalRead(button2);

  for (int i = 0; i < 4; i++) {
    pinMode(player[PLAYER_ONE].sevSeg[i], OUTPUT);
    pinMode(player[PLAYER_TWO].sevSeg[i], OUTPUT);

    // initialize sevseg states
    digitalWrite(player[PLAYER_ONE].sevSeg[i], HIGH); // LOW selects digit
    digitalWrite(player[PLAYER_TWO].sevSeg[i], HIGH); // LOW selects digit
  }

  pinMode(clkPin, OUTPUT);
  pinMode(latPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  int clockTick = millis();
  int modeLockClock = millis();
}
/*     ______________________
      /  _____   _____     / |
     /  /MAIN/  /LOOP/    /  |
    /____________________/   |
   |____________________|____|
   
*/
void loop() {
  
  switch (gameState) {
    
    case PREGAME :
      pregame_loop();
      break;
      
    case INGAME :
      ingame_loop();
      break;

    case POSTGAME :
      postgame_loop();
      break;

  }
}
/*--------------------------------------------------------------------*\
   The function that is called repeatedly during the PREGAME
\*--------------------------------------------------------------------*/
void pregame_loop(void) {
  
  int change, center, input;

  // Get the user selected start time
  if (inputMode == TIME) {
    input = get_player_times(read_potentiometer());
    display_number_for_start_time(input);
    player[PLAYER_ONE].time = player[PLAYER_TWO].time = input;

  } else {
    input = get_player_increments(read_potentiometer());
    display_number_for_increment(input);
    player[PLAYER_ONE].increment = player[PLAYER_TWO].increment = input;
  }

  change = button_changed();
  if (change == PLAYER_ONE || change == PLAYER_TWO ) {
    turnPlayer = (turnPlayer->name == PLAYER_TWO) ? (&player[PLAYER_ONE]) : (&player[PLAYER_TWO]);
    gameState = INGAME;
    
  } else if (check_center_button() == SHORT_PRESS) {
    
    if (millis() - modeLockClock > 500) {
      inputMode = !(inputMode);
      modeLockClock = millis();
    }
  }
  
}
/*--------------------------------------------------------------------*\
   The function that is called repeatedly during the INGAME
\*--------------------------------------------------------------------*/
void ingame_loop(void) {

      int change;
      
      checkForPauseOrReset();

      if (!paused && one_second_has_passed()) {
        decrement_turn_player_time();
      }
      
      display_number_in_game();
      
      if (out_of_time()) {
        gameState = POSTGAME;
      }

      // See if the player's turn has ended
      if (!paused) {
        change = button_changed();
        if (change == turnPlayer->name) {
          next_player_turn();
        }
      }
}
/*--------------------------------------------------------------------*\
   The function that is called repeatedly during the POSTGAME
\*--------------------------------------------------------------------*/
void postgame_loop(void) {

    int center;

    display_number_in_game();
    buzz_loop(1);

    if (button_changed() != NONE || check_center_button() != NO_PRESS) {
      gameState = PREGAME;
    }
}
/*--------------------------------------------------------------------*\
   Check if the player has pressed or held the center button,
   changing the gameState accordingly.
\*--------------------------------------------------------------------*/
void checkForPauseOrReset(void) {
  
   int center = check_center_button();
   
   if (center == SHORT_PRESS) {
     paused != paused;
   } else if (center == LONG_PRESS) {
     gameState = PREGAME;
   }
}
/*--------------------------------------------------------------------*\
   Intended for use when called to turn on for a one time beep.
   @period : the length of time to keep on
\*--------------------------------------------------------------------*/
void buzz_noise(int duration) {
  digitalWrite(buzzerPin, HIGH);
  delay(duration);
  digitalWrite(buzzerPin, LOW);
}
/*--------------------------------------------------------------------*\
   Intended for use when called in a loop.
   @period : the length of time between each toggle on or off
\*--------------------------------------------------------------------*/
void buzz_loop(int period) {
  static bool buzzState = false;
  digitalWrite(buzzerPin, buzzState);
  delay(period);
  buzzState = !(buzzState);
}
/*--------------------------------------------------------------------*\
   Check if the center button has been pressed or held down
   @return : NO_PRESS, SHORT_PRESS, LONG_PRESS
\*--------------------------------------------------------------------*/
int check_center_button(void) {

  static int lastHigh = millis();
  int val = digitalRead(centerButton);

  if (val == LOW) {

    if ((millis() - lastHigh) > 2000) {
      lastHigh = millis();
      return LONG_PRESS;
    }

    if ((millis() - lastHigh) > 2) {
      lastHigh = millis();
      return SHORT_PRESS;
    }

  } else { // val == HGGH
    lastHigh = millis();
  }

  return NO_PRESS;
}
/*--------------------------------------------------------------------*\
   Check if a second has passed since we last checked
   @return : true if one second or more has gone by
\*--------------------------------------------------------------------*/
boolean one_second_has_passed(void) {

  int current = millis();

  if ( (current - clockTick) > 999) {
    clockTick = current;
    return true;
  }

  return false;
}
/*--------------------------------------------------------------------*\
   Adjust the turn player's time by the amount passed in
\*--------------------------------------------------------------------*/
void decrement_turn_player_time(void) {

  turnPlayer->time--;
}
/*--------------------------------------------------------------------*\
   Check if either player has run out of time
   @return : true if a player is out of time
\*--------------------------------------------------------------------*/
boolean out_of_time(void) {

  if (turnPlayer->time <= 0)
    return true;

  return false;
}
/*--------------------------------------------------------------------*\
   Change the state of the game to be the other player's turn
\*--------------------------------------------------------------------*/
void next_player_turn(void) {

    turnPlayer->time += turnPlayer->increment;
    turnPlayer = (turnPlayer->name == PLAYER_TWO) ?
          (&player[PLAYER_ONE])
          : (&player[PLAYER_TWO]);

    clockTick = millis();

}
/*--------------------------------------------------------------------*\
   Check if one or both of the player's buttons has been pressed
   @return : NONE, PLAYER_ONE, PLAYER_TWO, or BOTH_PLAYERS
\*--------------------------------------------------------------------*/
int button_changed(void) {

  if ( digitalRead(button1) != player[PLAYER_ONE].buttonState) {
    player[PLAYER_ONE].buttonState = digitalRead(button1);
    return PLAYER_ONE;
  }
  if ( digitalRead(button2) != player[PLAYER_TWO].buttonState) {
    player[PLAYER_TWO].buttonState = digitalRead(button2);
    return PLAYER_TWO;
  }

  return NONE;
}
/*--------------------------------------------------------------------*\
   Set up the values to send to the displays during or after the game
   and send it to the displays
\*--------------------------------------------------------------------*/
void display_number_in_game(void) {

  int digit[8];

  separate_digits_in_time(player[PLAYER_ONE].time, &digit[0]);
  separate_digits_in_time(player[PLAYER_TWO].time, &digit[4]);

  // Replace digits with encoded values
  for (int n = 0; n < 8; n++ ) {
    digit[n] = sevSegDigits[digit[n]];
  }

  send_data_to_displays(&digit[0]);

}
/*--------------------------------------------------------------------*\
   Send the date int the incoming array to the displays
   @arr : the 8 sevseg digits to send to the displays
\*--------------------------------------------------------------------*/
void send_data_to_displays(int *arr) {
  for (int i = 0; i < 4; i++ ) {
    digitalWrite(player[PLAYER_ONE].sevSeg[i], LOW);
    shiftOut(shiftReg, clkPin, LSBFIRST, arr[i + 4]);
    digitalWrite(player[PLAYER_ONE].sevSeg[i], HIGH);

    digitalWrite(player[PLAYER_TWO].sevSeg[i], LOW);
    shiftOut(shiftReg, clkPin, LSBFIRST, arr[i]); 
    digitalWrite(player[PLAYER_TWO].sevSeg[i], HIGH);

    digitalWrite(latPin, LOW);
    digitalWrite(latPin, HIGH);
  }

}
/*--------------------------------------------------------------------*\
   Transforms a decimal integer up to four digits into 4 values, one
   representing each digit
   @time : the decimal integer to transform
   @arr : the four int wide array to fill with the values
\*--------------------------------------------------------------------*/
void separate_digits_in_time(int time, int *arr)
{
  if (time > 0) {
    arr[0] = (time % 10);
    arr[1] = (time % 100) / 10;
    arr[2] = (time % 1000) / 100;
    arr[3] = (time % 10000) / 1000;
  } else {
    convert_to_phrase(LOSE_PHRASE, arr);
  }

}
/*--------------------------------------------------------------------*\
   Set up the data for the displays during the pregame when in time
   input mode and send it to the displays
   @time : the start time value to send to the displays
\*--------------------------------------------------------------------*/
void display_number_for_start_time(int time) {

  int digit[8];

  if (time == 0) {
    convert_to_phrase(CLOC_PHRASE, &digit[0]);
    convert_to_phrase(SET_PHRASE, &digit[4]);
  } else {
    separate_digits_in_time(time, &digit[0]);
    convert_to_phrase(CLOC_PHRASE, &digit[4]);
  }

  // Replace digits with encoded values
  for (int n = 0; n < 8; n++ ) {
    digit[n] = sevSegDigits[digit[n]];
  }

  send_data_to_displays(&digit[0]);
}
/*--------------------------------------------------------------------*\
   Set up the data for the displays during the pregame when in increment
   input mode and send it to the displays
   @incr : the increment value to send to the displays
\*--------------------------------------------------------------------*/
void display_number_for_increment(int incr) {

  int digit[8];

  separate_digits_in_time(incr, &digit[0]);
  convert_to_phrase(INC_PHRASE, &digit[4]);

  // Replace digits with encoded values
  for (int n = 0; n < 8; n++ ) {
    digit[n] = sevSegDigits[digit[n]];
  }

  send_data_to_displays(&digit[0]);

}
/*--------------------------------------------------------------------*\
   Override the characters to be sent to the display
   @phrase : the id for the selected phrase
   @arr : the array that holds the 4 character bytes
\*--------------------------------------------------------------------*/
void convert_to_phrase(int phrase, int* arr) {

  switch (phrase) {

    case SPIN_PHRASE :
      arr[3] = S;
      arr[2] = P;
      arr[1] = I;
      arr[0] = N;
      break;

    case LOSE_PHRASE :
      arr[3] = L;
      arr[2] = O;
      arr[1] = S;
      arr[0] = E;
      break;

    case SET_PHRASE :
      arr[3] = S;
      arr[2] = E;
      arr[1] = T;
      arr[0] = BLANK;
      break;

    case CLOC_PHRASE :
      arr[3] = C;
      arr[2] = L;
      arr[1] = O;
      arr[0] = C;
      break;

    case INC_PHRASE :
      arr[3] = I;
      arr[2] = N;
      arr[1] = C;
      arr[0] = BLANK;
      break;
  }
}
/*--------------------------------------------------------------------*\
   See what the reading on the knob is and map it to an interval
   @return : the interval mapped by the knob position (1-36)
\*--------------------------------------------------------------------*/
int read_potentiometer(void) {

  int reading = analogRead(potPin);

  int interval;

  for ( int i = 1; i < 36; i++ ) {

    interval = i + 36 - (2 * i);
    interval = round(13.86 * (interval + 1));

    if ( reading > interval ) {
      return i;
    }
  }

}
/*--------------------------------------------------------------------*\
   Get the start time that corresponds to an interval
   @interval : the interval from the knob position (1-36)
   @return : the time corresponding to the interval
\*--------------------------------------------------------------------*/
int get_player_times(int interval)
{

  return (interval <0) ? (0) : 
         ((interval > 35) ? (intervalToTime[35]) :
         (intervalToTime[interval]));
         
}
/*--------------------------------------------------------------------*\
   Get the increment time that correspond to an interval
   @interval : the interval from the knob position (1-36)
   @return : the increment corresponding to the intervalf
\*--------------------------------------------------------------------*/
int get_player_increments(int interval)
{
  return (interval <0) ? (0) : 
         ((interval > 35) ? (intervalToIncr[35]) :
         (intervalToIncr[interval]));
}


