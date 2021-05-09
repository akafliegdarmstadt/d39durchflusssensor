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
   * configure 16-bit Timer/Counter1 for PWM output on pin PB1 (OC1A)
   */
  pinMode(9, OUTPUT);

  /*
   * - COM1A[1-0] clear on compare match, set at BOTTOM (non-inverting mode)
   * - WGM1[3-0] Fast PWM with TOP at ICR1
   * - CS1[2-0] from prescaler: clk/256 (update cPrescaleFactor on change)
   */
  TCCR1A = 0b10000010;
  TCCR1B = 0b00011000 | CLOCK_SELECT_MASK;

  ICR1 = 2717; // set PWM TOP value -> 23 Hz
  OCR1A = 2717 * cDutyCicle; // set duty cicle -> 10%

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
 * Interrupt Service Routine for the Timer/Counter1 Overflow
 * 
 * In this ISR the PWM values get set.
 */
ISR(TIMER1_OVF_vect, ISR_BLOCK) {
  ICR1 = top;
  OCR1A = compare;

  TIMSK1 &= 0b11111110; // clear TOIE1 (Timer/Counter1, Overflow Interrupt Enable) bit
  // the setFuelFlow() function sets the TOIE1 bit (enabling this ISR) again if the values have changed
}

/*
 * Update the PWM parameter to simulate the given fuel flow.
 * 
 * @param fuel_flow The fuel flow level the pwm output should simulate in liter/hour
 * @param verbose if set a debug message gets written to the Serial
 */
void setFuelFlow(float fuel_flow, bool verbose) {
  static bool pwm_off = false;
  
  if (fuel_flow == 0) {
    TCCR1B &= 0b11111000; // clear CS (clock Source) bits to stop PWM
    pwm_off = true;

    if (verbose) {
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
        ICR1 = top;
        OCR1A = compare;
  
        TCCR1B |= CLOCK_SELECT_MASK; // set clock sources to start PWM
        pwm_off = false;
  	} else {
        TIMSK1 |= 0b00000001; // set TOIE bit to write the new values after the next TC overflow (->ISR)
  	}
    
    if (verbose) {
      String msg = "Set new fuel flow (" + String(fuel_flow) + " liter" + " -> " + String(PwmFreq) + " Hz, PWM: " + top + "/" + compare + ")";
      Serial.println(msg);
    }
  }
}
