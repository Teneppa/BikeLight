// This demo is coded by Teemu Laurila @ Teukkaniikka
// Date: 09.10.2020
//
// Use as you wish, but don't claim that you coded this, because
// you obviously didn't, and you would get a lot of crap for showing
// something this buggy to someone else.

/*
 * This thing works with Neopixels (aka. WS2812B LEDs)
 */

/************************ Afafruit neopixel object **************************/

#include <Adafruit_NeoPixel.h>

#define PIN 4         // Neopixel DIN-pin goes here
#define LED_COUNT 7   // How many leds do you have?

/***************************** Blink variables ******************************/

bool brake_state        = false;      // Is the brake activated?
bool left_blink_state   = false;      // Is left blinker activated?
bool right_blink_state  = false;      // Is right blinker activated?
bool blinkInit          = true;       // This will reset the blink animation
                                      // if it has stopped in the middle of it.

int8_t blink_state      = 0;          // 0 = no blink, -1 = left, 1 = right
uint16_t blink_delay    = 400;        // This is the delay between on/off states
                                      // of the indicator in ms

long oldBlink           = 0;          // Last time that the
                                      // blink state has changed.

/************************ COLORS & NEOPIXEL SETTINGS ************************/

Adafruit_NeoPixel led = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

const uint32_t white    = led.Color(255, 255, 255); // White
const uint32_t dim_red  = led.Color(40, 0, 0);      // Dark red
const uint32_t red      = led.Color(255, 0, 0);     // Red
const uint32_t yellow   = led.Color(255, 150, 0);   // Yellow
//const uint32_t dim_red  = led.Color(50,0,20);     // Dark Purple
//const uint32_t red      = led.Color(255,0,80);    // Purple

/****************************** SWEEP SETTINGS ******************************/

// Is the sweep still going on?
bool sweepBusy = false;

// In which pixel are we in the animation? (index)
int8_t sweepIndex = 0;

// Last animation update in ms
unsigned long oldUpdate = 0;

// How long to wait between animation steps in ms
uint16_t sweepUpdateInterval = 80; // 80

/*********************************** SETUP **********************************/

void setup() {
  // Setup all of the inputs (these are probably
  //                          used only for testing)
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

  // Setup the neopixel library
  led.begin();

  // Lower the sweepUpdateInterval to
  // speed up the startup animation.
  sweepUpdateInterval = 40;

  // Initialize the color of the boot animation
  uint32_t color = dim_red;

  // Set the sweepBusy to true to find out if it's running
  // FIXME: This shouldn't be needed, but the code doesn't work
  //        without this.
  sweepBusy = true;

  // Startup animation
  while (true) {
    // Set the color according to the brake status
    if (not digitalRead(12)) {
      color = red;
    } else {
      color = dim_red;
    }

    // Do a sweep
    sweep(0, LED_COUNT, true, color);

    // If the sweep is finished, break the while loop
    if (not sweepBusy) {
      break;
    }
  }

  // Set the sweep interval back to normal
  sweepUpdateInterval = 80;
}

/*********************************** LOOP **********************************/

void loop() {

  // If indicating to left
  if (digitalRead(2) == LOW) {
    if (blink_state == 0) {
      blink_state = -1;
    }
  }

  // If indicating to right
  if (digitalRead(3) == LOW) {
    if (blink_state == 0) {
      blink_state = 1;
    }
  }

  // Brake status
  brake_state = not digitalRead(12);

  // If both indicators are off
  if (digitalRead(2) == HIGH && digitalRead(3) == HIGH) {
    blink_state = 0;
  }

  // If we are not braking or indicating
  if (blink_state == 0 && brake_state == 0) {
    led.setPixelColor(0, dim_red);
    led.setPixelColor(1, dim_red);
    led.setPixelColor(2, dim_red);
    led.setPixelColor(3, dim_red);
    led.setPixelColor(4, dim_red);
    led.setPixelColor(5, dim_red);
    led.setPixelColor(6, dim_red);
  }

  // Braking without blinkers
  if (blink_state == 0 && brake_state == 1) {
    led.setPixelColor(0, red);
    led.setPixelColor(1, red);
    led.setPixelColor(2, red);
    led.setPixelColor(3, red);
    led.setPixelColor(4, red);
    led.setPixelColor(5, red);
    led.setPixelColor(6, red);
  }

  // If indicators are activated
  if (blink_state != 0) {
    uint32_t redColor = 0;

    // Handle the braking color
    if (brake_state) {
      redColor = red;
    } else {
      redColor = dim_red;
    }

    // If indicating left
    if (left_blink_state) {
      sweep(0, 3, false, yellow);
    } else {
      led.setPixelColor(0, redColor);
      led.setPixelColor(1, redColor);
      led.setPixelColor(2, redColor);
    }

    // Set the middle led always to the redColor
    led.setPixelColor(3, redColor);

    // If indicating right
    if (right_blink_state) {
      sweep(4, 3, true, yellow);
    } else {
      led.setPixelColor(4, redColor);
      led.setPixelColor(5, redColor);
      led.setPixelColor(6, redColor);
    }

    // If indicators are disabled, reset the sweep animation
    if (not left_blink_state && not right_blink_state) {
      sweepIndex = 0;
      sweepBusy = false;
    }

    // Handle the blink status
    if (millis() - oldBlink > blink_delay || blinkInit) {
      if (blink_state == -1) {
        left_blink_state = not left_blink_state;
      } else {
        right_blink_state = not right_blink_state;
      }

      blinkInit = false;
      oldBlink = millis();
    }
  } else {
    left_blink_state = false;
    right_blink_state = false;
    blinkInit = true;
    oldBlink = millis();
  }

  led.show();
}


// Do a sweep that starts from a specific index
// dir = True -> right
bool sweep(uint8_t startFrom, uint8_t count, bool dir, uint32_t color) {

  // Keep the refresh rate stable
  if (millis() - oldUpdate > sweepUpdateInterval) {

    // If we are starting the sweep
    if (not sweepBusy) {
      // Set all of the target leds to black
      for (uint8_t i = startFrom; i < startFrom + count; i++) {
        led.setPixelColor(i, 0);
      }
      led.show();

      // Set the starting index
      if (dir == true) {
        sweepIndex = startFrom;
      } else {
        sweepIndex = startFrom + count - 1;
      }

      sweepBusy = true;

    } else {
      // If direction is right
      if (dir == true) {
        // While there's LEDs left, do the sweep animation
        if (sweepIndex < startFrom + count) {
          led.setPixelColor(sweepIndex, color);

          /*
            // This can be used to clear the trail of the sweep animation
            // Currently disabled.
            if (sweepIndex > 0) {
            led.setPixelColor(sweepIndex - 1, 0);
            }
          */

          led.show();

          sweepIndex += 1;
        } else {
          /*
            // This can be used to clear all of the leds after the animation is
            // done. We don't need for now.
            for (uint8_t i = startFrom; i < startFrom + count; i++) {
            led.setPixelColor(i, 0);
            }
            led.show();
          */

          sweepBusy = false;
        }
        // If the direction is left
      } else {
        // While there's LEDs to sweep
        if (sweepIndex >= startFrom) {
          led.setPixelColor(sweepIndex, color);

          /*
            // This is also for clearing the trail
            if (sweepIndex < startFrom + count) {
            led.setPixelColor(sweepIndex + 1, 0);
            }
            led.show();
          */

          sweepIndex -= 1;


        } else {
          /*
            // This can be used to clear all of the leds after the animation is
            // done. We don't need for now.
            for (uint8_t i = startFrom; i < startFrom + count; i++) {
            led.setPixelColor(i, 0);
            }
            led.show();
          */

          sweepBusy = false;
        }
      }
    }

    // Save the update time
    oldUpdate = millis();
  }

  // Return true if the animation isn't finished
  return sweepBusy;
}
