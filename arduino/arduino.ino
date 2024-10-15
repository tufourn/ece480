#define NOZZLE_COUNT               12
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
const int nozzles[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14};

const PinInfo unoPins[] = {
//                            GPIO    PIN
  {&DDRD, &PIND, &PORTD, 0}, // D0     0   Rx     
  {&DDRD, &PIND, &PORTD, 1}, // D1     1   Tx
  {&DDRD, &PIND, &PORTD, 2}, // D2     2   
  {&DDRD, &PIND, &PORTD, 3}, // D3     3   
  {&DDRD, &PIND, &PORTD, 4}, // D4     4   
  {&DDRD, &PIND, &PORTD, 5}, // D5     5   
  {&DDRD, &PIND, &PORTD, 6}, // D6     6   
  {&DDRD, &PIND, &PORTD, 7}, // D7     7   
  {&DDRB, &PINB, &PORTB, 0}, // B0     8   
  {&DDRB, &PINB, &PORTB, 1}, // B1     9   
  {&DDRB, &PINB, &PORTB, 2}, // B2     10   
  {&DDRB, &PINB, &PORTB, 3}, // B3     11   
  {&DDRB, &PINB, &PORTB, 4}, // B4     12  
  {&DDRB, &PINB, &PORTB, 5}, // B5     13  on-board LED    
  {&DDRC, &PINC, &PORTC, 0}, // C0     14
  {&DDRC, &PINC, &PORTC, 1}, // C1     15   
  {&DDRC, &PINC, &PORTC, 2}, // C2     16   
  {&DDRC, &PINC, &PORTC, 3}, // C3     17   
  {&DDRC, &PINC, &PORTC, 4}, // C4     18   
  {&DDRC, &PINC, &PORTC, 5}, // C5     19   
};

/**
 * Write to a pin using direct port manipulation for low latency
 * @param pin The pin to write
 * @param state 1 (HIGH) to set pin to 5V, 0 (LOW) to set pin to 0V
 */
void digitalWriteFast(const PinInfo& pin, bool state) {
  volatile uint8_t portRegister = *pin.port;

  if (state) {
    portRegister |= 1 << pin.bit;
  } else {
    portRegister &= ~(1 << pin.bit);
  }
  
  *pin.port = portRegister;
}

/**
 * Pulse specified pin for PULSE_DURATION us
 * @param pin The pin to pulse
 */
void pulse(const PinInfo& pin) {
  digitalWriteFast(pin, HIGH); 
  delayMicroseconds(PULSE_DURATION);
  digitalWriteFast(pin, LOW);
}

/**
 * Initiate a pulse on all nozzles sequentially
 *
 * For HP6602, A pulse lasts for 5us, after which follows a delay. 
 * To successively pulse the same nozzle, an ~800us is required. 
 * For different nozzles, a ~0.5us delay is required. 1us is used. See
 * Inkshield theory: http://nicholasclewis.com/projects/inkshield/theory/ 
 */
void pulseTestSuccessive() {
  for (int i = 0; i < NOZZLE_COUNT; i++) {
    pulse(unoPins[nozzles[i]]);
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
    for (int i = 0; i < 8; i++) {
      // pulse nozzle at index `i` if its bit is set HIGH in mask
      if (mask & (1 << i)) {
        pulse(unoPins[nozzles[i]]);
        delayMicroseconds(PULSE_DELAY_DIFF_NOZZLE);
      }
    }
    cmdMode = MASK2;
  }
  if (cmdMode == MASK2) {
    for (int i = 0; i < 4; i++) {
      if (mask & (1 << i)) {
        // pulse nozzle at index `i + 8` for remaining 4 nozzles
        pulse(unoPins[nozzles[i + 8]]);
        delayMicroseconds(PULSE_DELAY_DIFF_NOZZLE);
      }
    }
    delayMicroseconds(PULSE_DELAY_SAME_NOZZLE);

    cmdMode = INSTRUCTION;
    step();
  }
}

void setup() {
  // Nozzle pin setup
  for(int i = 0; i < NOZZLE_COUNT; i++) {
    pinMode(nozzles[i], OUTPUT);
  }

  Serial.begin(115200);
  while (Serial.available() <= 0) {
    Serial.print('X');
    delay(100);
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
        case 'T': 
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
