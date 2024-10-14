#define PULSE_DURATION              5  // us
#define PULSE_DELAY_DIFF_NOZZLE     1  // us
#define PULSE_DELAY_SAME_NOZZLE   800  // us

enum CommandMode {
  INSTRUCTION,
  MASK1,
  MASK2,
};

CommandMode cmdMode = INSTRUCTION;

struct PinInfo {
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  volatile uint8_t* port;
  uint8_t bit;
};

// Nozzle Number       1  2  3  4  5  6  7  8  9   10  11  12 
const int nozzles[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15};

const PinInfo nozzlePins[] = {
//                           Register  Pin Nozzle
  {&DDRD, &PIND, &PORTD, 2}, // D2     2   1
  {&DDRD, &PIND, &PORTD, 3}, // D3     3   2
  {&DDRD, &PIND, &PORTD, 4}, // D4     4   3
  {&DDRD, &PIND, &PORTD, 5}, // D5     5   4
  {&DDRD, &PIND, &PORTD, 6}, // D6     6   5
  {&DDRD, &PIND, &PORTD, 7}, // D7     7   6
  {&DDRB, &PINB, &PORTB, 0}, // B0     8   7
  {&DDRB, &PINB, &PORTB, 1}, // B1     9   8
  {&DDRB, &PINB, &PORTB, 2}, // B2    10   9
  {&DDRB, &PINB, &PORTB, 3}, // B3    11   10
  {&DDRB, &PINB, &PORTB, 4}, // B4    12   11 
  {&DDRC, &PINC, &PORTC, 1}, // C0    14   12
};

/**
 * Set the pin for a nozzle HIGH or LOW
 * @param info The register info for the nozzle pin to modify
 * @param state 1 (HIGH) to set pin to 5V, 0 (LOW) to set pin to 0V
 */
void writePin(const PinInfo& info, bool state) {
  volatile uint8_t portRegister = *info.port;

  if (state) {
    portRegister |= 1 << info.bit;
  } else {
    portRegister &= ~(1 << info.bit);
  }
  
  *info.port = portRegister;
}

/**
 * Initiate a 5V pulse on a single nozzle
 *
 * For HP6602, A pulse lasts for 5us, after which follows a delay. 
 * For the same nozzle, an 800us is required for successive pulses. 
 * For different nozzles, a 0.5us delay is required. 
 * @param info The register info for nozzle pin to set or clear
 */
void pulse(const PinInfo& info){
  writePin(info, HIGH); 
  delayMicroseconds(PULSE_DURATION);
  writePin(info, LOW);
}

/**
 * Test all nozzle pins successively
 */
void pulseTestSuccessive(){
  for (int i = 0; i < 12; i++){
    pulse(nozzlePins[i]);
    delayMicroseconds(PULSE_DELAY_DIFF_NOZZLE);
  }
  delayMicroseconds(PULSE_DELAY_SAME_NOZZLE);
}

// move stepper x 1 step (dot width) to the right
// keep track of steps moved so we can return to the begin of line
void step() {
  // TODO
}

// goto the begin of current line
void gotoBeginLine() {
  // TODO
}

// move stepper y 1 line above (dot height) * nozzleCount
void gotoNextLine() {
  // TODO
}

// dispense ink based on bitmask
// when last mask done, call step() and change cmdMode to INSTRUCTION
void dispense(unsigned char mask) {
  if (cmdMode == MASK1) {
    /* TODO: dispense first set of nozzles */
    for (int i = 0; i < 8; i++){
      // pulse nozzle at index `i` if its bit is set HIGH in mask
      if (mask & (1 << i)){
        pulse(nozzlePins[i]);
      }
    }
    cmdMode = MASK2;
  }
  if (cmdMode == MASK2) {
    /* TODO: dispense second set of nozzles */
    for (int i = 8; i < 12; i++){
      // pulse nozzle at index `i` if its bit is set HIGH in mask
      if (mask & (1 << i)){
        pulse(nozzlePins[i]);
      }
    }
    cmdMode = INSTRUCTION;
    step();
  }
}

void setup() {
  Serial.begin(115200);
  while (Serial.available() <= 0) {
    Serial.print('X');
    delay(100);
  }

  // Nozzle pin setup
  for(int i = 0; i < 12; i++){
    pinMode(nozzles[i], OUTPUT);
  }
}

void loop() {
  if (Serial.available() > 0) {
    unsigned char cmd = Serial.read();
    Serial.print('R'); // let host know byte has been received
    if (cmdMode == INSTRUCTION) {
      switch (cmd) {
        case 'D':
          cmdMode = MASK1;
          break;
        case 'R':
          gotoBeginLine();
          break;
        case 'N':
          gotoNextLine();
          break;
        case 'T': // Potential INSTRUCTION to test nozzles
          pulseTestSuccessive();
          break;
        default:
          break;
      }
    } else {
      dispense(cmd);
    }
  }
}
