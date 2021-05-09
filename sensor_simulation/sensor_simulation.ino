#include <inttypes.h>

#define CLOCK_SELECT_MASK 0b00000100;
static const uint32_t cPrescaleFactor = 256;
static const uint32_t cCpuFreq = 16000000; // [Hz]
static const uint32_t cCalibrationFaktor = 8600; // [ticks/liter]
static const uint32_t cTimerFreq = cCpuFreq / cPrescaleFactor; // [Hz]
static const float cDutyCicle = 0.10;

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
    float top = cTimerFreq / PwmFreq; // PWM TOP value
  
    ICR1 = (uint16_t)top;
    OCR1A = (uint16_t)(top * cDutyCicle);

    if (pwm_off) {
      TCCR1B |= CLOCK_SELECT_MASK; // set clock sources to start PWM
      pwm_off = false;
    }
  
    if (verbose) {
      String msg = "Set new fuel flow (" + String(fuel_flow) + " liter" + " -> " + String(PwmFreq) + " Hz, PWM: " + ICR1 + "/" + OCR1A + ")";
      Serial.println(msg);
    }
  }
}
