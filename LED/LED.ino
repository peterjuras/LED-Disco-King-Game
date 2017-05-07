#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

int LED_STRIP_PIN = 6;

int BUTTON_PIN_P1 = 10;
int BUTTON_PIN_P2 = 13;

const byte IMPULS_MIN_SIZE = 4; // Minimum length of an impulse

const int PIXELS = 150; // Number of LEDs of a single strip

int Lifes = 100;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, LED_STRIP_PIN, NEO_GRB);

struct RGB {
  byte r;
  byte g;
  byte b;
};


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


// Colors
RGB red;

// Array of impulses
//
// size_impulses is the number of concurrent impulses per strip. The idea behind the calculation is to
// have a number where we would have no overlapping impulses in position and length that can fit across
// a single strip
const int size_impulses = PIXELS / (IMPULS_MIN_SIZE + 1); 
Impuls *impulses[size_impulses];
int current_index = -1;

void createNewImpuls() {
  current_index = (current_index + 1) % size_impulses; // Rolling index, overwriting old impulses

  // Re-initializè the impulse at the current position
  impulses[current_index]->position = 0;
  impulses[current_index]->length = IMPULS_MIN_SIZE;
  impulses[current_index]->color = red;
};

void setup() { // Running once after Arduino boots
  strip.begin();
  
  strip.show(); // Initialize all LEDs to be off
  strip.setBrightness(150);
  
  Serial.begin(9600); // TODO: Remove if we only use Serial for print statements

  // Initialize "red" to be red
  red.r = 255;
  red.g = 0;
  red.b = 0;

  Serial.println(size_impulses);

  // Fill Array with DummyImpulses
  for (int i = 0; i < size_impulses; i++) {
    impulses[i] = new Impuls(-1, IMPULS_MIN_SIZE, red);
  }
}

void loop() { // Should run once every ~33ms to receive 30 frames per second
   // Record the current milliseconds to track how long the logic
   // takes to execute
  int ms= millis();

  // Check whether the first player is pressing her button
  if (digitalRead(BUTTON_PIN_P1) == HIGH) {
    // Create a new impulse if there has never been an impulse created (current_index == -1)
    // or if the last impulse is no longer at the beginning of the strip. (position > 1)
    // TODO: Impulses can currently overlap,
    // we should see whether this can become a problem at some point
    if (
      current_index == -1 ||
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
  }

  // Turn all pixels off
  for (int i = 0; i < PIXELS; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  
  // Loop over all impulses
  for (int i = 0; i < size_impulses; i++) {
    // Check whether the impulse is valid and initialized, otherwise skip this impulse
    if (impulses[i]->position == -1) {
      continue;
    }

    // Colorize all relevant pixels of the strip
    int pos = impulses[i]->position;
    for (int j = 0; j < impulses[i]->length && pos + j < PIXELS; j++) {
      strip.setPixelColor(pos + j, impulses[i]->color.r, impulses[i]->color.g, impulses[i]->color.b);
    }

    // Check whether the impulse is at the other end of the strip. If it is at the other end
    // and the second player is not pressing her button reduce the lifes of this player.
    if (
      pos + impulses[i]->length - 2 >= PIXELS - 1 &&
      digitalRead(BUTTON_PIN_P2)==LOW
    ) {
      Lifes--;
    }

    // If the impulse is at the end of the strip, "disable" it,
    // by setting the position back to the invalid value of -1
    if (pos == PIXELS - 1) {
      impulses[i]->position = -1;
    }
    else {
      // If the impulse is still valid we proceed to the next position
      impulses[i]->position++;
    }
  }

  // Show all pixels
  strip.show();

  // Calculate the time it took to execute this iteration.
  ms = millis() - ms;

  // Wait at least 33 milliseconds until we continue with the next iteration
  delay(17 - ms);
}
