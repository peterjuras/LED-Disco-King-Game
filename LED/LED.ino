#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

int LED_STRIP_PIN = 6;
int BUTTON_PIN_P1 = 10;
int BUTTON_PIN_P2 = 13;
const byte IMPULS_MIN_SIZE = 4;
const int PIXELS = 150;

int Lifes=100;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, LED_STRIP_PIN, NEO_GRB);

//position im Object= StartPosition

struct RGB {
  byte r;
  byte g;
  byte b;
};


class Impuls {
  public:
    int position;
    byte length;
    RGB color;
    Impuls(int pos, byte len, RGB col);
};

Impuls::Impuls(int pos, byte len, RGB col)
{
  position = pos;
  length = len;
  color = col;

};


//Colors
RGB red;

//Array of impulses
const int size_impulses=PIXELS / (IMPULS_MIN_SIZE + 1);
Impuls *impulses[size_impulses];
int current_index = -1;


void createNewImpuls() {
  current_index = (current_index + 1) % size_impulses;
  impulses[current_index]->position = 0;
  impulses[current_index]->length = IMPULS_MIN_SIZE;
  impulses[current_index]->color = red;
};



void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.show();
  strip.setBrightness(150);
  Serial.begin(9600);
  red.r = 255;
  red.g = 0;
  red.b = 0;

  Serial.println(size_impulses);

  //Fill Array with DummyImpulses
  for (int i = 0; i < size_impulses; i++) {
    impulses[i] = new Impuls(-1, IMPULS_MIN_SIZE, red);
  }

}


void loop() {
  int ms= millis();
  // put your main code here, to run repeatedly:

  if (digitalRead(BUTTON_PIN_P1) == HIGH) {
    if (current_index == -1 || impulses[current_index]->position > 1) {
      createNewImpuls();
      //Können sich überlappen wegen der Länge
    }
    else {
      impulses[current_index]->position = 0;
      impulses[current_index]->length++;
    }
  }

  //Schalte alle Pixel aus
  for (int i = 0; i < PIXELS; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  //Schalte Impuls Pixel an
  for (int i = 0; i < size_impulses; i++) {
    if (impulses[i]->position == -1) {
      continue;
    }
    int pos = impulses[i]->position;
    for (int j = 0; j < impulses[i]->length && pos + j < PIXELS; j++) {
      strip.setPixelColor(pos + j, impulses[i]->color.r, impulses[i]->color.g, impulses[i]->color.b);
    }
    
    if (pos+impulses[i]->length >= PIXELS-1 && digitalRead(BUTTON_PIN_P2)==LOW){
      Lifes--;
      Serial.print("You lost, rest lifes: ");
      Serial.println(Lifes);
    }
    
    if (pos == PIXELS - 1) {
      impulses[i]->position = -1;
    }
    else {
      impulses[i]->position++;
    }
  }
  strip.show();
  ms=millis()-ms;

  delay(33-ms);
  
}
