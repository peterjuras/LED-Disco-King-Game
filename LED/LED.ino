#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

// Lane 1
const int LED_STRIP_PIN_LANE_1 = 23;
const int BUTTON_PIN_SHOOT_LANE_1 = 6;
const int BUTTON_PIN_DEFEND_LANE_1 = 11;
const int STRIP_START_LANE_1 = 25;
const int STRIP_END_LANE_1 = 125;
// Lane 2
const int LED_STRIP_PIN_LANE_2 = 30;
const int BUTTON_PIN_SHOOT_LANE_2 = 2;
const int BUTTON_PIN_DEFEND_LANE_2 = 10;
const int STRIP_START_LANE_2 = 25;
const int STRIP_END_LANE_2 = 125;
// Lane 3
const int LED_STRIP_PIN_LANE_3 = 22;
const int BUTTON_PIN_SHOOT_LANE_3 = 3;
const int BUTTON_PIN_DEFEND_LANE_3 = 9;
const int STRIP_START_LANE_3 = 25;
const int STRIP_END_LANE_3 = 125;
// Lane 4
const int LED_STRIP_PIN_LANE_4 = 26;
const int BUTTON_PIN_SHOOT_LANE_4 = 4;
const int BUTTON_PIN_DEFEND_LANE_4 = 8;
const int STRIP_START_LANE_4 = 25;
const int STRIP_END_LANE_4 = 125;
// Lane 5
const int LED_STRIP_PIN_LANE_5 = 27;
const int BUTTON_PIN_SHOOT_LANE_5 = 5;
const int BUTTON_PIN_DEFEND_LANE_5 = 7;
const int STRIP_START_LANE_5 = 25;
const int STRIP_END_LANE_5 = 125;

// Life Lane
const int LED_STRIP_PIN_LIFE = 40;

bool idle = true;

// Number of LEDs of a single strip
const int PIXELS_LANE_1 = 150;
const int PIXELS_LANE_2 = 150;
const int PIXELS_LANE_3 = 150;
const int PIXELS_LANE_4 = 150;
const int PIXELS_LANE_5 = 150;
const int PIXELS_LANE_LIFE = 150;

const byte IMPULS_MIN_SIZE = 4; // Minimum length of an impulse
const int MAX_LIFES = 100;
int Lifes = MAX_LIFES;
const int DEFEND_BUFFER_SIZE = 15;

unsigned long lastRunTime = 0;

int lastActions[10];
int currentAction = 0;
bool superShot = false;

bool swapped = true;

struct RGB {
  byte r;
  byte g;
  byte b;
};

// Music and Sounds

const char GAME_OVER_MUSIC[] = "$M-1";
const char BACKGROUND_MUSIC[] = "$M";
const int NUMBER_OF_MUSIC_TRACKS = 1;
int currentMusicTrack = 0;


const char PLAYER_SUPERSHOT_SOUND[] = "$S0";
const char PLAYER_SHOOT_SOUND[] = "$S1";
const char PLAYER_HIT_SOUND[] = "$S2";

// Colors structs
RGB red;
RGB blue;
RGB yellow;

class Impuls {
  public:
    int position; // StartPosition of the impulse
    byte length; // Length of the impulse
    bool hitSoundPlayed;
    RGB color;
    Impuls(int pos, byte len, RGB col);
    void playHitSound();
    bool supershot;
};

Impuls::Impuls(int pos, byte len, RGB col) {
  position = pos;
  length = len;
  color = col;

  hitSoundPlayed = false;
  supershot = false;
};

void Impuls::playHitSound() {
  if (!hitSoundPlayed) {
    hitSoundPlayed = true;
    if (supershot) {
      Serial.println(PLAYER_SUPERSHOT_SOUND);
    } else {
      Serial.println(PLAYER_HIT_SOUND);
    }
  }
}

class Lane {
  public:
    Adafruit_NeoPixel* strip;
    //dynamically allocated array of *Impuls
    int stripStart;
    int stripEnd;
    Impuls** impulses;
    int current_index;
    Lane(int num_pixels, int led_pin, int button1, int button2, int stripStart, int stripEnd);
    void createNewImpuls();
    void createSupershot();
    void loop(int index);
    void setLaneColor(int r, int g, int b);
    void resetImpulses();
       
  private:
    int button1;
    int button2;  
    int size_impulses_array;
    int defendBuffer;
};

//Lane Constructor
Lane::Lane(int num_pixels, int led_pin, int but1, int but2, int sStart, int sEnd){
  strip= new Adafruit_NeoPixel(num_pixels, led_pin, NEO_GRB);
  strip->begin();
  strip->show(); // Initialize all LEDs to be off
  strip->setBrightness(150);
  // Array of impulses
  // size_impulses is the number of concurrent impulses per strip. The idea behind the calculation is to
  // have a number where we would have no overlapping impulses in position and length that can fit across
  // a single strip
  size_impulses_array = num_pixels / (IMPULS_MIN_SIZE + 1); 
//  impulses=(Impuls**)malloc(size_impulses_array*sizeof(Impuls*));
  impulses = new Impuls*[size_impulses_array];
  // Fill Array with DummyImpulses
  for (int i = 0; i < size_impulses_array; i++) {
    impulses[i] = new Impuls(-1, IMPULS_MIN_SIZE, blue);
  }
  current_index=-1;
  button1=but1;
  button2=but2;
  stripStart = sStart;
  stripEnd = sEnd;
  defendBuffer = 0;
};

void Lane::resetImpulses() {
  for (int i = 0; i < size_impulses_array; i++) {
    impulses[i]->position = -1;
    impulses[i]->length = IMPULS_MIN_SIZE;
    impulses[i]->color = blue;
    impulses[i]->hitSoundPlayed = false;
    impulses[i]->supershot = false;
  }
}

//New Impuls Function declaration
void Lane::createNewImpuls() {
  current_index = (current_index + 1) % size_impulses_array; // Rolling index, overwriting old impulses

  // Re-initializè the impulse at the current position
  impulses[current_index]->position = swapped ? stripEnd : stripStart;
  impulses[current_index]->length = IMPULS_MIN_SIZE;
  impulses[current_index]->color = blue;
  impulses[current_index]->hitSoundPlayed = false;
  impulses[current_index]->supershot = false;

  Serial.println(PLAYER_SHOOT_SOUND);
};

//New Impuls Function declaration
void Lane::createSupershot() {
  current_index = (current_index + 1) % size_impulses_array; // Rolling index, overwriting old impulses

  // Re-initializè the impulse at the current position
  impulses[current_index]->position = swapped ? stripEnd : stripStart;
  impulses[current_index]->length = IMPULS_MIN_SIZE * 6;
  impulses[current_index]->color = yellow;
  impulses[current_index]->hitSoundPlayed = false;
  impulses[current_index]->supershot = true;

  Serial.println(PLAYER_SHOOT_SOUND);
};

void Lane::setLaneColor(int r, int g, int b) {
  for (int i = 0; i < strip->numPixels(); i++) {
    strip->setPixelColor(i, r, g, b);
  }
  strip->show();
}

void saveLastAction(int index) {
  if (lastActions[(currentAction - 1) % 10] == index) {
    return;
  } 
  lastActions[currentAction] = index;
  currentAction = (currentAction + 1) % 10;
}

void Lane::loop(int index){
  // Check whether the first player is pressing her button

  int shootButton = swapped ? button2 : button1;
  bool shootButtonPressed = digitalRead(shootButton) == HIGH;

  if (superShot) {
    createSupershot();
  } else if (shootButtonPressed) {
    // Create a new impulse if there has never been an impulse created (current_index == -1)
    // or if the last impulse is no longer at the beginning of the strip. (position > 1)
    // TODO: Impulses can currently overlap,
    // we should see whether this can become a problem at some point
    if (current_index == -1 ||
      impulses[current_index]->position > (swapped ? stripEnd - 1 : stripStart + 1)
    ) {
      createNewImpuls();
    }
    else {
      // We should extend the current impulse, because it is still at the beginning.
      // We reset the position and extend its length to give the impression of a
      // longer impulse.
      impulses[current_index]->position = swapped ? stripEnd : stripStart;
      impulses[current_index]->length++;
    }
    saveLastAction(index);
  }

  // Turn all pixels off
  for (int i = 0; i < strip->numPixels(); i++) {
    strip->setPixelColor(i, 0, 0, 0);
  }

  // Colorize plates
  RGB shootPlateColor;
  shootPlateColor.r = 0;
  shootPlateColor.g = shootButtonPressed ? 255 : 0;
  shootPlateColor.b = 0;

  if (swapped) {
    for (int i = stripEnd; i < strip->numPixels(); i++) {
      strip->setPixelColor(i, shootPlateColor.r, shootPlateColor.g, shootPlateColor.b);
    }
  } else {
    for (int i = 0; i < stripStart; i++) {
      strip->setPixelColor(i, shootPlateColor.r, shootPlateColor.g, shootPlateColor.b);
    }
  }
  
  // Loop over all impulses

  RGB defendPlateColor;
  defendPlateColor.r = 0;
  defendPlateColor.g = 0;
  defendPlateColor.b = 0;

  int defendButton = swapped ? button1 : button2;
  if (digitalRead(defendButton) == HIGH) {
    defendBuffer = DEFEND_BUFFER_SIZE;  
  }
  
  for (int i = 0; i < size_impulses_array; i++) {
    // Check whether the impulse is valid and initialized, otherwise skip this impulse
    if (impulses[i]->position == -1) {
      continue;
    }

    if (swapped) {      
      // Colorize all relevant pixels of the strip
      int pos = impulses[i]->position;
      for (int j = pos - impulses[i]->length; j <= pos; j++) {
        strip->setPixelColor(j, impulses[i]->color.r, impulses[i]->color.g, impulses[i]->color.b);
      }
  
      // Check whether the impulse is at the other end of the strip. If it is at the other end
      // and the second player is not pressing her button reduce the lifes of this player.
      if (pos - impulses[i]->length - 2 <= stripStart) {
        if (defendBuffer > 0) {
          defendPlateColor.g = 255;
        } else {
          Lifes--;
          defendPlateColor.r = 255;
          impulses[i]->playHitSound();
        }
      }
  
      // If the impulse is at the end of the strip, "disable" it,
      // by setting the position back to the invalid value of -1
      if (pos == stripStart) {
        impulses[i]->position = -1;
      }
      else {
        // If the impulse is still valid we proceed to the next position
        impulses[i]->position--;
      }
    } else {
      // Colorize all relevant pixels of the strip
      int pos = impulses[i]->position;
      for (int j = 0; j < impulses[i]->length && pos + j < stripEnd; j++) {
        strip->setPixelColor(pos + j, impulses[i]->color.r, impulses[i]->color.g, impulses[i]->color.b);
      }
  
      // Check whether the impulse is at the other end of the strip. If it is at the other end
      // and the second player is not pressing her button reduce the lifes of this player.
      if (pos + impulses[i]->length - 2 >= stripEnd - 1) {
        if (defendBuffer > 0) {
          defendPlateColor.g = 255;
        } else {
          Lifes--;
          defendPlateColor.r = 255;
          impulses[i]->playHitSound();
        }
      }
  
      // If the impulse is at the end of the strip, "disable" it,
      // by setting the position back to the invalid value of -1
      if (pos == stripEnd - 1) {
        impulses[i]->position = -1;
      }
      else {
        // If the impulse is still valid we proceed to the next position
        impulses[i]->position++;
      }
    }
  }

  if (swapped) {
    for (int i = 0; i < stripStart; i++) {;
      strip->setPixelColor(i, defendPlateColor.r, defendPlateColor.g, defendPlateColor.b);
    }
  } else {
    for (int i = stripEnd; i < strip->numPixels(); i++) {;
      strip->setPixelColor(i, defendPlateColor.r, defendPlateColor.g, defendPlateColor.b);
    }
  }

  // Show all pixels
  strip->show();
  
  defendBuffer--;
};

const int NUM_LANES = 5;
Lane* Lanes_Array[NUM_LANES];

Adafruit_NeoPixel* lifeStrip = new Adafruit_NeoPixel(PIXELS_LANE_LIFE, LED_STRIP_PIN_LIFE, NEO_GRB);

void setupLifeDisplay() {
  lifeStrip->begin();
  lifeStrip->show();
  lifeStrip->setBrightness(150);
}

void updateLifeDisplay() {
  float stripPixels = lifeStrip->numPixels();
  float usableLifes = max(Lifes, 0);
  float lifePercentage = (usableLifes / MAX_LIFES) * stripPixels;
  float lifeThreshold = stripPixels - lifePercentage;
  
  for (float i = 0; i < stripPixels; i++) {
    if (swapped) {
      if (i <= lifeThreshold) {
        lifeStrip->setPixelColor(i, 0, 255, 0);
      } else {
        lifeStrip->setPixelColor(i, 0, 0, 0);
      }
    } else {
      if (i >= lifeThreshold) {
        lifeStrip->setPixelColor(i, 0, 255, 0);
      } else {
        lifeStrip->setPixelColor(i, 0, 0, 0);
      }
    }
  }
  lifeStrip->show();
}

void setupNewGame() {
  // Start music
  Serial.println(BACKGROUND_MUSIC + currentMusicTrack);
  currentMusicTrack = (currentMusicTrack + 1) % NUMBER_OF_MUSIC_TRACKS;
  
  Lifes = MAX_LIFES;

  for (int i=0; i<NUM_LANES; i++) {
    Lanes_Array[i]->resetImpulses();
  }

  swapped = !swapped;

  if (!swapped) {
    idle = true;
  }
}

void blinkAllLanes(int r, int g, int b, int ms) {
  for (int i=0; i<NUM_LANES; i++) {
    Lanes_Array[i]->setLaneColor(r, g, b);
  }
  delay(ms / 2);
  for (int i=0; i<NUM_LANES; i++) {
    Lanes_Array[i]->setLaneColor(0, 0, 0);
  }
  delay(ms / 2);
}

void blinkWinner(bool playerTwo, int ms) {
  RGB firstColor;
  firstColor.r = playerTwo ? 255 : 0;
  firstColor.g = playerTwo ? 0 : 255;
  firstColor.b = 0;

  RGB secondColor;
  secondColor.r = playerTwo ? 0 : 255;
  secondColor.g = playerTwo ? 255 : 0;
  secondColor.b = 0;

  for (int i=0; i<NUM_LANES / 2; i++) {
    int stripPixels = Lanes_Array[i]->strip->numPixels();
    for (int j = 0; j < stripPixels / 2; j++) {
      Lanes_Array[i]->strip->setPixelColor(j, firstColor.r, firstColor.g, firstColor.b);
    }
    for (int j = stripPixels / 2; j < stripPixels; j++) {
      Lanes_Array[i]->strip->setPixelColor(j, secondColor.r, secondColor.g, secondColor.b);
    }
  }
  delay(ms / 2);
  for (int i=0; i<NUM_LANES; i++) {
    Lanes_Array[i]->setLaneColor(0, 0, 0);
  }
  delay(ms / 2);
}

void endGame() {
  unsigned long runTime = millis();
  Serial.println(GAME_OVER_MUSIC);

  if (swapped) {
    // Game has ended, check which player has won
    if (runTime < lastRunTime) {
      // Player 2 has won
      blinkWinner(true, 1000);
      blinkWinner(true, 1000);
      blinkWinner(true, 1000);
      blinkWinner(true, 1000);
    } else {
      // Player 1 has won
      blinkWinner(false, 1000);
      blinkWinner(false, 1000);
      blinkWinner(false, 1000);
      blinkWinner(false, 1000);
    }
  } else {
    blinkAllLanes(255, 0, 0, 1000);
    blinkAllLanes(255, 0, 0, 1000);
    blinkAllLanes(255, 0, 0, 1000);
    blinkAllLanes(255, 0, 0, 1000);
    blinkAllLanes(255, 0, 0, 1000);
    blinkAllLanes(0, 255, 0, 1000);
  }
  lastRunTime = runTime;
}

bool interrupted() {
  return
    digitalRead(BUTTON_PIN_SHOOT_LANE_1) == HIGH ||
    digitalRead(BUTTON_PIN_DEFEND_LANE_1) == HIGH ||
    
    digitalRead(BUTTON_PIN_SHOOT_LANE_2) == HIGH ||
    digitalRead(BUTTON_PIN_DEFEND_LANE_2) == HIGH ||
    
    digitalRead(BUTTON_PIN_SHOOT_LANE_3) == HIGH ||
    digitalRead(BUTTON_PIN_DEFEND_LANE_3) == HIGH ||
    
    digitalRead(BUTTON_PIN_SHOOT_LANE_4) == HIGH ||
    digitalRead(BUTTON_PIN_DEFEND_LANE_4) == HIGH ||
    
    digitalRead(BUTTON_PIN_SHOOT_LANE_5) == HIGH ||
    digitalRead(BUTTON_PIN_DEFEND_LANE_5) == HIGH;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return Lanes_Array[0]->strip->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return Lanes_Array[0]->strip->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Lanes_Array[0]->strip->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int n=0; n<NUM_LANES; n++) {
        int stripNumPixels = Lanes_Array[n]->strip->numPixels();
        
        for(int i=0; i< stripNumPixels; i = i + 3) {
          Lanes_Array[n]->strip->setPixelColor(i+q, Wheel( (i+j) % 255));
        }
        
        Lanes_Array[n]->strip->show();
      }
    
      if (interrupted()) {
        idle = false;
        return;
      }

      delay(wait);

      for (int n=0; n<NUM_LANES; n++) {
        int stripNumPixels = Lanes_Array[n]->strip->numPixels();
        
        for(int i=0; i< stripNumPixels; i = i + 3) {
          Lanes_Array[n]->strip->setPixelColor(i+q, 0);
        }
      }
    }
  }
}

void checkCheatCode() {
  if (
    lastActions[(currentAction - 1) % 10] == 2 &&
    lastActions[(currentAction - 2) % 10] == 3 &&
    lastActions[(currentAction - 3) % 10] == 1 &&
    lastActions[(currentAction - 4) % 10] == 4 &&
    lastActions[(currentAction - 5) % 10] == 0
  ) {
    superShot = true;
  }
}

void setup() { // Running once after Arduino boots
  Serial.begin(9600);

  // Initialize "red" to be red
  red.r = 255;
  red.g = 0;
  red.b = 0;

  blue.r = 0;
  blue.g = 0;
  blue.b = 255;

  yellow.r = 255;
  yellow.g = 255;
  yellow.b = 255;

  Lanes_Array[0]=new Lane(PIXELS_LANE_1, LED_STRIP_PIN_LANE_1, BUTTON_PIN_SHOOT_LANE_1, BUTTON_PIN_DEFEND_LANE_1, STRIP_START_LANE_1, STRIP_END_LANE_1);
  Lanes_Array[1]=new Lane(PIXELS_LANE_2, LED_STRIP_PIN_LANE_2, BUTTON_PIN_SHOOT_LANE_2, BUTTON_PIN_DEFEND_LANE_2, STRIP_START_LANE_2, STRIP_END_LANE_2);
  Lanes_Array[2]=new Lane(PIXELS_LANE_3, LED_STRIP_PIN_LANE_3, BUTTON_PIN_SHOOT_LANE_3, BUTTON_PIN_DEFEND_LANE_3, STRIP_START_LANE_3, STRIP_END_LANE_3);
  Lanes_Array[3]=new Lane(PIXELS_LANE_4, LED_STRIP_PIN_LANE_4, BUTTON_PIN_SHOOT_LANE_4, BUTTON_PIN_DEFEND_LANE_4, STRIP_START_LANE_4, STRIP_END_LANE_4);
  Lanes_Array[4]=new Lane(PIXELS_LANE_5, LED_STRIP_PIN_LANE_5, BUTTON_PIN_SHOOT_LANE_5, BUTTON_PIN_DEFEND_LANE_5, STRIP_START_LANE_5, STRIP_END_LANE_5);

  setupLifeDisplay();

  setupNewGame();
}

void loop() { 
  // Should run once every ~33ms to receive 30 frames per second
  // Record the current milliseconds to track how long the logic
  // takes to execute

  checkCheatCode();

  if (idle) {
    // Idle animation check buttons
    theaterChaseRainbow(50);
    return;
  }

  if (Lifes < 0) {
    endGame();
    setupNewGame();
  }
   
  int ms = millis();
  
  for (int i=0; i<NUM_LANES; i++) {
    Lanes_Array[i]->loop(i);
  }

  if (superShot) {
    superShot = false;
    saveLastAction(-1);
  }

  updateLifeDisplay();
  
  // Calculate the time it took to execute this iteration.
  ms = millis() - ms;
  int delayTime = 17 - ms;
  
  // Wait at least 33 milliseconds until we continue with the next iteration
  if (delayTime > 0) {
    delay(delayTime);
  }
}
