#define DEF_OFFSET 50
#define EXPO 400  // how long the LED is enabled in microseconds
#define PERIOD 10000  // period of flickering in microseconds
#define SET_TIMER 500
#define ADC_PRESCALER 4  
#define FIXED_OSC_PERIOD 300000  

#define M1A 5
#define M1B 6
#define M2A 9
#define M2B 10
#define POT_PIN A0

volatile bool solenoidState = false;
volatile bool ledState = false;
volatile uint16_t flashDelay = PERIOD * 2 + DEF_OFFSET;
uint32_t lastPotRead = 0;

void setADCprescaler(uint8_t prescaler) {
  ADC0.CTRLA &= ~ADC_ENABLE_bm;  
  ADC0.CTRLA = (ADC0.CTRLA & ~ADC_PRESC_gm) | prescaler;  
  ADC0.CTRLA |= ADC_ENABLE_bm; 
}

uint16_t readADC(uint8_t channel) {
  ADC0.MUXPOS = channel;      
  ADC0.CTRLA |= ADC_STARTEI_bm;   
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) {}  
  ADC0.INTFLAGS = ADC_RESRDY_bm;  
  return ADC0.RES;                
}

void setup() {
  cli();

  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  digitalWrite(M1A, LOW);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, LOW);

  setADCprescaler(ADC_PRESCALER); 

  // Set up TCA0 (16-bit Timer) for solenoid control 
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm;  // Set prescaler to 64 and enable timer
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // CTC mode (Single Slope PWM)
  TCA0.SINGLE.PER = (FIXED_OSC_PERIOD / 64) - 1; // Set the period (adjusted for zero-indexed count)
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;

  // Set up TCB0 (8-bit Timer) for LED control
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
  TCB0.CTRLB = TCB_CNTMODE_INT_gc;
  TCB0.CCMP = EXPO / 64;
  TCB0.INTCTRL = TCB_CAPT_bm;

  Serial.begin(115200);
  sei();
}

void loop() {
  if (millis() - lastPotRead >= SET_TIMER) {
    lastPotRead = millis();
    uint16_t potValue = readADC(POT_PIN); 
    Serial.println(potValue);
    float oscFrequency = map(potValue, 0, 1023, 10 * 10, 12 * 10) / 10.0;
    noInterrupts();
    flashDelay = 1000000 / (oscFrequency * 2);
    TCB0.CCMP = EXPO / 64;  
    interrupts();
  }
}

ISR(TCA0_OVF_vect) {
//**** SOLENOID CONTROL ****
  solenoidState = !solenoidState;  
  if (solenoidState) {
    digitalWrite(M1A, HIGH);
    digitalWrite(M1B, LOW);  
  } else {
    digitalWrite(M1A, LOW);
    digitalWrite(M1B, HIGH);  
  }
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;  // Clear the interrupt flag
}

ISR(TCB0_INT_vect) {
//**** LED CONTROL ****
  static bool flashState = false;
  flashState = !flashState;
  if (flashState) {
    digitalWrite(M2A, LOW);
    digitalWrite(M2B, LOW);
    TCB0.CCMP = EXPO / 64;
  } else {
    digitalWrite(M2A, LOW);
    digitalWrite(M2B, HIGH);
    TCB0.CCMP = (flashDelay - EXPO) / 64;
  }
  TCB0.INTFLAGS = TCB_CAPT_bm;  // Clear the interrupt flag
}
