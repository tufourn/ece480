enum CommandMode {
  INSTRUCTION,
  MASK1,
  MASK2,
};

CommandMode cmdMode = INSTRUCTION;

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
    cmdMode = MASK2;
  }
  if (cmdMode == MASK2) {
    /* TODO: dispense second set of nozzles */
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
        default:
          break;
      }
    } else {
      dispense(cmd);
    }
  }
}
