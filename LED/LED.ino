#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

// Lane 1
const int LED_STRIP_PIN_LANE_1 = 2;
const int BUTTON_PIN_SHOOT_LANE_1 = 8;
const int BUTTON_PIN_DEFEND_LANE_1 = 11;
const int STRIP_START_LANE_1 = 25;
const int STRIP_END_LANE_1 = 125;
// Lane 2
const int LED_STRIP_PIN_LANE_2 = 3;
const int BUTTON_PIN_SHOOT_LANE_2 = 9;
const int BUTTON_PIN_DEFEND_LANE_2 = 10;
const int STRIP_START_LANE_2 = 25;
const int STRIP_END_LANE_2 = 125;
// Lane 3
const int LED_STRIP_PIN_LANE_3 = 4;
const int BUTTON_PIN_SHOOT_LANE_3 = 10;
const int BUTTON_PIN_DEFEND_LANE_3 = 10;
const int STRIP_START_LANE_3 = 25;
const int STRIP_END_LANE_3 = 125;
// Lane 4
const int LED_STRIP_PIN_LANE_4 = 5;
const int BUTTON_PIN_SHOOT_LANE_4 = 11;
const int BUTTON_PIN_DEFEND_LANE_4 = 10;
const int STRIP_START_LANE_4 = 25;
const int STRIP_END_LANE_4 = 125;
// Lane 5
const int LED_STRIP_PIN_LANE_5 = 6;
const int BUTTON_PIN_SHOOT_LANE_5 = 12;
const int BUTTON_PIN_DEFEND_LANE_5 = 10;
const int STRIP_START_LANE_5 = 25;
const int STRIP_END_LANE_5 = 125;

// Life Lane
const int PIXELS_LANE_LIFE = 150;
const int LED_STRIP_PIN_LIFE = 1;

// Number of LEDs of a single strip
const int PIXELS_LANE_1 = 150;
const int PIXELS_LANE_2 = 150;
const int PIXELS_LANE_3 = 150;
const int PIXELS_LANE_4 = 150;
const int PIXELS_LANE_5 = 150;

const byte IMPULS_MIN_SIZE = 4; // Minimum length of an impulse
const int MAX_LIFES = 100;
int Lifes = MAX_LIFES;

struct RGB {
  byte r;
  byte g;
  byte b;
};

// Music and Sounds

char BACKGROUND_MUSIC[] = "$M0";

// Colors structs
RGB red;

class Impuls {
  public:
    int position; // StartPosition of the impulse
    byte length; // Length of the impulse
    RGB color;
    Impuls(int pos, byte len, RGB col);
};

Impuls::Impuls(int pos, byte len, RGB col) {
  position = pos;
  length = len;
  color = col;
};

class Lane {
  public:
    Adafruit_NeoPixel* strip;
    //dynamically allocated array of *Impuls
    int stripStart;
    int stripEnd;
    bool hitSoundArmed;
    Impuls** impulses;
    int current_index;
    Lane(int num_pixels, int led_pin, int button1, int button2, int stripStart, int stripEnd);
    void createNewImpuls();
    void loop();
    
  private:
    int button1;
    int button2;
    int size_impulses_array;
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
    impulses[i] = new Impuls(-1, IMPULS_MIN_SIZE, red);
  }
  current_index=-1;
  button1=but1;
  button2=but2;
  stripStart = sStart;
  stripEnd = sEnd;
  hitSoundArmed = true;
};

//New Impuls Function declaration
void Lane::createNewImpuls() {
  current_index = (current_index + 1) % size_impulses_array; // Rolling index, overwriting old impulses

  // Re-initializè the impulse at the current position
  impulses[current_index]->position = stripStart;
  impulses[current_index]->length = IMPULS_MIN_SIZE;
  impulses[current_index]->color = red;
};

void Lane::loop(){
  // Check whether the first player is pressing her button
  if (digitalRead(button1) == HIGH) {
    // Create a new impulse if there has never been an impulse created (current_index == -1)
    // or if the last impulse is no longer at the beginning of the strip. (position > 1)
    // TODO: Impulses can currently overlap,
    // we should see whether this can become a problem at some point
    // Serial.println("Shoot Button is HIGH");
    if (current_index == -1 ||
      impulses[current_index]->position > 1
    ) {
      createNewImpuls();
    }
    else {
      // We should extend the current impulse, because it is still at the beginning.
      // We reset the position and extend its length to give the impression of a
      // longer impulse.
      impulses[current_index]->position = 0;
      impulses[current_index]->length++;
    }
  } else {
      // Serial.println("Shoot Button is LOW");
  }

  // Turn all pixels off
  for (int i = 0; i < strip->numPixels(); i++) {
    strip->setPixelColor(i, 0, 0, 0);
  }
  
  // Loop over all impulses
  for (int i = 0; i < size_impulses_array; i++) {
    // Check whether the impulse is valid and initialized, otherwise skip this impulse
    if (impulses[i]->position == -1) {
      continue;
    }

    // Colorize all relevant pixels of the strip
    int pos = impulses[i]->position;
    for (int j = 0; j < impulses[i]->length && pos + j < stripEnd; j++) {
      strip->setPixelColor(pos + j, impulses[i]->color.r, impulses[i]->color.g, impulses[i]->color.b);
    }

    // Check whether the impulse is at the other end of the strip. If it is at the other end
    // and the second player is not pressing her button reduce the lifes of this player.
    if (
      pos + impulses[i]->length - 2 >= stripEnd - 1 &&
      digitalRead(button2)==LOW
    ) {
      Lifes--;
      if (hitSoundArmed) {
        hitSoundArmed = false;
        Serial.println("$S0");  
      }
    } else {
      hitSoundArmed = true;
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

  // Show all pixels
  strip->show();
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
  int stripPixels = lifeStrip->numPixels();
  int usableLifes = max(Lifes, 0);
  int pixelThreshold = (usableLifes / MAX_LIFES) * stripPixels;
  
  for (int i = 0; i < stripPixels; i++) {
    if (i >= pixelThreshold) {
      lifeStrip->setPixelColor(i, 0, 0, 0);
    } else {
      lifeStrip->setPixelColor(i, 0, 255, 0);
    }
  }
  lifeStrip->show();
}

void setup() { // Running once after Arduino boots
  Serial.begin(9600);

  // Start music
  Serial.println(BACKGROUND_MUSIC);

  // Initialize "red" to be red
  red.r = 255;
  red.g = 0;
  red.b = 0;

  Lanes_Array[0]=new Lane(PIXELS_LANE_1, LED_STRIP_PIN_LANE_1, BUTTON_PIN_SHOOT_LANE_1, BUTTON_PIN_DEFEND_LANE_1, STRIP_START_LANE_1, STRIP_END_LANE_1);
  Lanes_Array[1]=new Lane(PIXELS_LANE_2, LED_STRIP_PIN_LANE_2, BUTTON_PIN_SHOOT_LANE_2, BUTTON_PIN_DEFEND_LANE_2, STRIP_START_LANE_1, STRIP_END_LANE_1);
  Lanes_Array[2]=new Lane(PIXELS_LANE_3, LED_STRIP_PIN_LANE_3, BUTTON_PIN_SHOOT_LANE_3, BUTTON_PIN_DEFEND_LANE_3, STRIP_START_LANE_1, STRIP_END_LANE_1);
  Lanes_Array[3]=new Lane(PIXELS_LANE_4, LED_STRIP_PIN_LANE_4, BUTTON_PIN_SHOOT_LANE_4, BUTTON_PIN_DEFEND_LANE_4, STRIP_START_LANE_1, STRIP_END_LANE_1);
  Lanes_Array[4]=new Lane(PIXELS_LANE_5, LED_STRIP_PIN_LANE_5, BUTTON_PIN_SHOOT_LANE_5, BUTTON_PIN_DEFEND_LANE_5, STRIP_START_LANE_1, STRIP_END_LANE_1);
}

void loop() { 
   // Should run once every ~33ms to receive 30 frames per second
   // Record the current milliseconds to track how long the logic
   // takes to execute
  int ms= millis();
  for (int i=0; i<NUM_LANES; i++){
      Lanes_Array[i]->loop();
    }

  
  // Calculate the time it took to execute this iteration.
  ms = millis() - ms;
  int delayTime = 17 - ms;
  // Wait at least 33 milliseconds until we continue with the next iteration
  if (delayTime > 0) {
    delay(delayTime);
  }
}
