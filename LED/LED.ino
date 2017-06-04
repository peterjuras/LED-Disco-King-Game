#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

// Lane 1
const int LED_STRIP_PIN_LANE_1 = 5;
const int BUTTON_PIN_SHOOT_LANE_1 = 10;
const int BUTTON_PIN_DEFEND_LANE_1 = 11;
// Lane 2
const int LED_STRIP_PIN_LANE_2 = 6;
const int BUTTON_PIN_SHOOT_LANE_2 = 11;
const int BUTTON_PIN_DEFEND_LANE_2 = 10;

// Number of LEDs of a single strip
const int PIXELS_LANE_1 = 50; 
const int PIXELS_LANE_2 = 50;

const byte IMPULS_MIN_SIZE = 4; // Minimum length of an impulse
int Lifes = 100;

struct RGB {
  byte r;
  byte g;
  byte b;
};

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
    Impuls** impulses;
    int current_index;
    Lane(int num_pixels, int led_pin, int button1, int button2);
    void createNewImpuls();
    void loop();
    
  private:
    int button1;
    int button2;
    int size_impulses_array;
};

//Lane Constructor
Lane::Lane(int num_pixels, int led_pin, int but1, int but2){
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
};

//New Impuls Function declaration
void Lane::createNewImpuls() {
  current_index = (current_index + 1) % size_impulses_array; // Rolling index, overwriting old impulses

  // Re-initializè the impulse at the current position
  impulses[current_index]->position = 0;
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
    Serial.println("Shoot Button is HIGH");
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
      Serial.println("Shoot Button is LOW");
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
    for (int j = 0; j < impulses[i]->length && pos + j < strip->numPixels(); j++) {
      strip->setPixelColor(pos + j, impulses[i]->color.r, impulses[i]->color.g, impulses[i]->color.b);
    }

    // Check whether the impulse is at the other end of the strip. If it is at the other end
    // and the second player is not pressing her button reduce the lifes of this player.
    if (
      pos + impulses[i]->length - 2 >= strip->numPixels() - 1 &&
      digitalRead(button2)==LOW
    ) {
      Lifes--;
    }

    // If the impulse is at the end of the strip, "disable" it,
    // by setting the position back to the invalid value of -1
    if (pos == strip->numPixels() - 1) {
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

const int NUM_LANES = 2;
Lane* Lanes_Array[NUM_LANES];

void setup() { // Running once after Arduino boots
  Serial.begin(9600); // TODO: Remove if we only use Serial for print statements

  // Initialize "red" to be red
  red.r = 255;
  red.g = 0;
  red.b = 0;

  Lanes_Array[0]=new Lane(PIXELS_LANE_1, LED_STRIP_PIN_LANE_1, BUTTON_PIN_SHOOT_LANE_1, BUTTON_PIN_DEFEND_LANE_1);
  Lanes_Array[1]=new Lane(PIXELS_LANE_2, LED_STRIP_PIN_LANE_2, BUTTON_PIN_SHOOT_LANE_2, BUTTON_PIN_DEFEND_LANE_2);
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
