#define DEF_OFFSET 50
#define EXPO 300  // LED on-time in microseconds
#define PERIOD 5000  // Flickering period in microseconds
#define ADC_PRESCALER 32
#define FIXED_OSC_PERIOD 14000  

#define M1A 5
#define M1B 6
#define M2A 9
#define M2B 10
#define POT_PIN A0

uint16_t setTimer, ledTimer, solenoidTimer;
volatile bool solenoidState = false;
volatile bool ledState = false;
volatile uint16_t flashDelay = PERIOD * 2 + DEF_OFFSET;
uint32_t lastPotRead = 0;
uint32_t lastSolenoidToggle = 0;
uint32_t lastLEDToggle = 0;

void setup() {
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  digitalWrite(M1A, LOW);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, LOW);

  setADCrate(2);
}

void loop() {
  uint32_t currentTime = micros();
  
  if (micros() - lastPotRead >= setTimer) {
    lastPotRead = micros();
    uint16_t potValue = analogRead(POT_PIN);
    flashDelay = PERIOD * 2 + potValue - 512;
  }

  // LED Control
  if (currentTime - lastLEDToggle >= (ledState ? EXPO : (flashDelay - EXPO))) {
    lastLEDToggle = currentTime;
    ledState = !ledState;
    digitalWrite(M2A, LOW);
    digitalWrite(M2B, ledState ? HIGH : LOW);
  }

  // Solenoid Control
  if (currentTime - lastSolenoidToggle >= FIXED_OSC_PERIOD) {
    lastSolenoidToggle = currentTime;
    solenoidState = !solenoidState;
    digitalWrite(M1A, solenoidState);
    digitalWrite(M1B, !solenoidState);
  }

}
void setADCrate(uint8_t mode) {
  // Disable ADC before changing settings
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  // Set prescaler based on mode
  switch (mode) {
    case 0:  ADC0.CTRLC = ADC_PRESC_DIV2_gc;   break; // 8 MHz
    case 1:  ADC0.CTRLC = ADC_PRESC_DIV4_gc;   break; // 4 MHz
    case 2:  ADC0.CTRLC = ADC_PRESC_DIV8_gc;   break; // 2 MHz
    case 3:  ADC0.CTRLC = ADC_PRESC_DIV16_gc;  break; // 1 MHz (safe)
    case 4:  ADC0.CTRLC = ADC_PRESC_DIV32_gc;  break; // 500 kHz
    case 5:  ADC0.CTRLC = ADC_PRESC_DIV64_gc;  break; // 250 kHz (default)
    case 6:  ADC0.CTRLC = ADC_PRESC_DIV128_gc; break; // 125 kHz
    default: ADC0.CTRLC = ADC_PRESC_DIV64_gc;  break; // Default to safe mode
  }

  // Re-enable ADC
  ADC0.CTRLA |= ADC_ENABLE_bm;
}
