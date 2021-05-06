#include <inttypes.h>

static const uint32_t cPrescaleFactor = 256;
static const uint32_t cCpuFreq = 16000000; // [Hz]
static const uint32_t cCalibrationFaktor = 8600; // [ticks/liter]
static const uint32_t cTimerFreq = cCpuFreq / cPrescaleFactor; // [Hz]
static const uint32_t cDutyCicle = 10; // [%]

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
  TCCR1B = 0b00011100;

  ICR1 = 2717; // set PWM TOP value -> 23 Hz
  OCR1A = (2717 * cDutyCicle) / 100; // set duty cicle -> 10%

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
  setFuelFlow(Serial.parseInt(), true);
}

/*
 * Update the PWM parameter to simulate the given fuel flow.
 * 
 * @param fuel_flow The fuel flow level the pwm output should simulate in liter/hour
 * @param verbose if set a debug message gets written to the Serial
 */
void setFuelFlow(uint32_t fuel_flow, bool verbose) {
  uint32_t PwmFreq = (fuel_flow * cCalibrationFaktor) / 3600; // [Hz] the needed PMW frequency
  uint16_t top = cTimerFreq / PwmFreq; // PWM TOP value

  ICR1 = top;
  OCR1A = (top * cDutyCicle) / 100;

  if (verbose) {
    String msg = "Set new fuel flow (" + String(fuel_flow) + " liter" + " -> " + String(PwmFreq) + " Hz)";
    Serial.println(msg);
  }
}
