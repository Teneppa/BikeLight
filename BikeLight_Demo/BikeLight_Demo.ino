#include <Adafruit_NeoPixel.h>

#define NEO_PIN 1
#define NEO_LED_COUNT 7
#define BRAKE_PIN 12
#define LEFT_BLINKER_PIN 2
#define RIGHT_BLINKER_PIN 3

enum brake_states {
    NOT_BRAKING,
    BRAKING
};
enum blinking_states {
    NOT_BLINKING,
    BLINKING_LEFT,
    BLINKING_RIGHT
};

class BikeLight {
    private:
        Adafruit_NeoPixel np;
        uint32_t red;
        uint32_t yellow;
        uint32_t dim_red;
        uint16_t blink_timer = NULL;
        brake_states braking = NOT_BRAKING;
        blinking_states blinking = NOT_BLINKING;

    public:
        BikeLight(Adafruit_NeoPixel neo_pixel){
            this -> np = neo_pixel;
            this -> red = neo_pixel.Color(255, 0, 0);
            this -> yellow = neo_pixel.Color(255, 150, 0);
            this -> dim_red = neo_pixel.Color(40, 0, 0);
        }

    void init() {
        /*
            Calls NeoPixel's begin() -method and displays starting animation.
        */
        np.begin();
        for(int i=0; i<=NEO_LED_COUNT; i++) {
            np.setPixelColor(i, red);
            np.show();
            delay(100);
        }
        np.clear();
    }

    void brake() {
        /*
            Braking, set every pixel to red.
        */
        np.fill(red);
    }

    void idle() {
        /*
            Idle state, set every other pixel's color to dim_red.
        */
        for(int i=0; i<NEO_LED_COUNT; i+=2){
            np.setPixelColor(i, dim_red);
        }
    }

    void blink() {
        /*
            Displays blinking animation on the correct side.
            Calculates the state of blinking cycle by the
            time since blinker was activated (blink_timer).
        */
        int blink_cycle_now = (millis() - blink_timer) / 500 % 3 + 1;
        if(blinking == BLINKING_LEFT) {
            np.fill(yellow, 3 - blink_cycle_now, blink_cycle_now);
        }
        else if(blinking == BLINKING_RIGHT) {
            np.fill(yellow, NEO_LED_COUNT - 3, blink_cycle_now);
        }
    }

    void update_states() {
        /*
            Check if user is braking or blinking and set states accordingly.
            digitalRead() returns true when it's not activated by user.
            Update blink_timer or set it to NULL if user is not blinking.
        */

        braking = digitalRead(BRAKE_PIN) ? NOT_BRAKING : BRAKING;
        blinking = digitalRead(LEFT_BLINKER_PIN) ?
                   digitalRead(RIGHT_BLINKER_PIN) ? NOT_BLINKING : BLINKING_RIGHT :
                   BLINKING_LEFT;
        blink_timer = blinking ? blink_timer == NULL ? millis() : blink_timer : NULL;
    }

    void update_leds() {
        /*
            Clears all pixel colors before setting new.
            Updates leds' colors and calls NeoPixel's show() -method
            to show updated colors.
        */
        np.clear();
        
        if(braking) {
            this -> brake();
        } else {
            this -> idle();
        }

        if(blinking) {
            this -> blink();
        }
        np.show();
    }

};

Adafruit_NeoPixel neo_pixel = Adafruit_NeoPixel(NEO_LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
BikeLight bl = BikeLight(neo_pixel);

void setup() {
    pinMode(BRAKE_PIN, INPUT_PULLUP);
    pinMode(LEFT_BLINKER_PIN, INPUT_PULLUP);
    pinMode(RIGHT_BLINKER_PIN, INPUT_PULLUP);
    bl.init();

}

void loop() {
    bl.update_states();
    bl.update_leds();
}
