#define dirPin_x 2
#define stepPin_x 3
#define dirPin_y 4
#define stepPin_y 5

enum CommandMode {
  INSTRUCTION,
  MASK1,
  MASK2,
};

CommandMode cmdMode = INSTRUCTION;

// move stepper x 1 step (dot width) to the right
// keep track of steps moved so we can return to the begin of line
void step() {
  // 21 microsteps per drop
  digitalWrite(dirPin_x, LOW);
  for(int i = 0; i < 21; i++){
    digitalWrite(stepPin_x, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin_x, LOW);
    delayMicroseconds(1000);
  }
}

// goto the begin of current line
void gotoBeginLine(int steps) {
  digitalWrite(dirPin_x, LOW);
  for(int i = 0; i < 21*steps; i++){
    digitalWrite(stepPin_x, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin_x, LOW);
    delayMicroseconds(1000);
  }
}

// move stepper y 1 line above (dot height) * nozzleCount
void gotoNextLine() {
  nozzleCount = 1;
  digitalWrite(dirPin_y, LOW);
  for(int i = 0; i < 21*nozzleCount; i++){
    digitalWrite(stepPin_y, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin_y, LOW);
    delayMicroseconds(1000);
  }
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
  pinMode(stepPin_x, OUTPUT);
  pinMode(dirPin_x, OUTPUT);
  pinMode(stepPin_y, OUTPUT);
  pinMode(dirPin_y, OUTPUT);
  
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
