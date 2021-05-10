#include <inttypes.h>

#define CLOCK_SELECT_MASK 0b00000100;
static const uint32_t cPrescaleFactor = 256;
static const uint32_t cCpuFreq = 16000000; // [Hz]
static const uint32_t cCalibrationFaktor = 8600; // [ticks/liter]
static const uint32_t cTimerFreq = cCpuFreq / cPrescaleFactor; // [Hz]
static const float cDutyCicle = 0.10;

volatile uint16_t top; // PWM TOP value
volatile uint16_t compare; // PWM compare value

/*
 * Setup routine, executed once at startup
 */
void setup() {
  /*
   * configure 16-bit Timer/Counter1 for PWM output on pin PB2 (OC1B)
   */
  pinMode(10, OUTPUT);

  /*
   * - COM1B[1-0] clear on compare match, set at BOTTOM (non-inverting mode)
   * - WGM1[3-0] Fast PWM with TOP at OCR1A
   * - CS1[2-0] from prescaler: clk/256 (update cPrescaleFactor on change)
   */
  TCCR1A = 0b00100011;
  TCCR1B = 0b00011000 | CLOCK_SELECT_MASK;

  OCR1A = 2717; // set PWM TOP value -> 23 Hz
  OCR1B = (uint16_t)(2717 * cDutyCicle); // set duty cicle -> 10%

  /*
   * init Serial
   */
  Serial.begin(9600);
  Serial.println("Hello World!");

  sei(); // global interrupt enable
}

/*
 * Main loop
 */
void loop() {
  /*
   * nothing to do here. PWM is running in hardware and get updated through Serial messages caught in serialEvent()-function.
   */
}

/*
 * function that gets called after received a Serial message
 */
void serialEvent() {
  setFuelFlow(Serial.parseFloat(), true);
}

/*
 * Interrupt Service Routine for the Timer/Counter1 Compare Match B
 * 
 * In this ISR the PWM values get set.
 */
ISR(TIMER1_COMPB_vect) {
  OCR1A = top;
  OCR1B = compare;

  TIMSK1 &= 0b11111011; // clear OCIE1B (Timer/Counter1, Output Compare B Match Interrupt Enable) bit
  // the setFuelFlow() function sets the OCIE1B bit (enabling this ISR) again if the values have changed
}

/*
 * Update the PWM parameter to simulate the given fuel flow.
 * 
 * @param fuel_flow The fuel flow level the pwm output should simulate in liter/hour
 * @param verbose_out if set a debug message gets written to the Serial
 */
void setFuelFlow(float fuel_flow, bool verbose_out) {
  static bool pwm_off = false;
  
  if (fuel_flow == 0) {
    TCCR1B &= 0b11111000; // clear CS (clock Source) bits to stop PWM
    pwm_off = true;

    if (verbose_out) {
      Serial.println("Turn fuel flow off");
    }
  } else if (fuel_flow < 0.4) { // the top value does not fit into 16-bit anymore
    Serial.println("Error: Can't set so low fuel flow!");
  } else {
    float PwmFreq = (fuel_flow * cCalibrationFaktor) / 3600; // [Hz] the needed PMW frequency
    float top_f = cTimerFreq / PwmFreq; // PWM TOP value
  
  	top = (uint16_t)top_f;
  	compare = (uint16_t)(top_f * cDutyCicle);
  
  	if (pwm_off) {
        OCR1A = top;
        OCR1B = compare;
  
        TCCR1B |= CLOCK_SELECT_MASK; // set clock sources to start PWM
        pwm_off = false;
  	} else {
        TIMSK1 |= 0b00000100; // set OCIE1B bit to write the new values after the next compare (->ISR)
  	}
    
    if (verbose_out) {
      String msg = "Set new fuel flow (" + String(fuel_flow) + " liter" + " -> " + String(PwmFreq) + " Hz, PWM: " + top + "/" + compare + ")";
      Serial.println(msg);
    }
  }
}
