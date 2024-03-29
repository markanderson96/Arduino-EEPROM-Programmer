// EEPROM Programmer
// Mark Anderson
// June 4th, 2019
//
// Quick & dirty EEPROM programmer using shift registers
// Reads/Writes to ROM

#define CE 10
#define OE 11
#define WE 12
#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4

#define READMEM 0xFA
#define WRITEMEM 0xFB
#define SUCCESS 0x01

#define ROM_SIZE 8

// Shift register usage
void setAddr(int addr){
	  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (addr >> 8));
	  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, addr);

	  digitalWrite(SHIFT_LATCH, LOW);
	  digitalWrite(SHIFT_LATCH, HIGH);
	  digitalWrite(SHIFT_LATCH, LOW);
}

// Read one byte at a time
byte readEEPROM(int addr){
    DDRC &= ~B00111111;
    DDRD &= ~B11000000;
	
    setAddr(addr);
    
    digitalWrite(CE, LOW);
    digitalWrite(OE, LOW);
	
    byte data = (PIND & B11000000) | PINC;
    
    digitalWrite(CE, HIGH);
    digitalWrite(OE, HIGH);
	
    return data;
}

// Read a block of memory
void readMem(){
    while (Serial.available() < 3) {} // Wait for serial buffer
    
    int addr = (Serial.read() << 8); // High byte address
    addr |= Serial.read(); // Low byte address

    int count = Serial.read();
    for (int i = 0; i < count; i++){
        byte data = readEEPROM(addr);
        Serial.write(data);
        addr ++;
    }
}

// Write to ROM
void writeMem(){
    static int ledState = LOW;

    while (Serial.available() < 3) {} // Wait for buffer

    int addr = (Serial.read() << 8); // High byte address
    addr |= Serial.read(); // Low byte address
    int count = Serial.read();
    byte data;

    while (Serial.available() < count) {} // Wait for buffer

    for (int i = 0; i < count; i++){
        setAddr(addr);
        DDRC |= B00111111;
        DDRD |= B11000000;

        data = Serial.read();
        PORTC = (data & B00111111);
        PORTD &= ~B11000000;
        PORTD |= (data & B11000000);

        digitalWrite(WE, LOW);
        digitalWrite(CE, LOW);
        digitalWrite(WE, HIGH);
        digitalWrite(CE, HIGH);
        
        DDRC &= ~B00111111;
        DDRD &= ~B11000000;
        addr++;
    }

    while (readEEPROM(addr-1) != data) {}

    digitalWrite(13, ledState);
    ledState = (ledState == HIGH) ? LOW : HIGH;

    Serial.write(SUCCESS);
}

void error(){
    while(true){
        digitalWrite(13, HIGH);
        delay(1000);
        digitalWrite(13, LOW);
        delay(1000);
    }
}

void setup(){
	// Set control pins as outputs
    pinMode(CE, OUTPUT);
    pinMode(OE, OUTPUT);
	  pinMode(WE, OUTPUT);
	  pinMode(SHIFT_DATA, OUTPUT);
	  pinMode(SHIFT_CLK, OUTPUT);
	  pinMode(SHIFT_LATCH, OUTPUT);
    
    digitalWrite(CE, HIGH);
    digitalWrite(OE, HIGH);
	  digitalWrite(WE, HIGH);

	// Initialise Serial
    Serial.begin(38400);

    // Status LED
    digitalWrite(13, HIGH);
}

void loop(){
    if (Serial.available() > 0){
        byte in = Serial.read(); // Command byte
        Serial.print(in);
        switch(in){
            case READMEM:
                readMem();
                break;
            case WRITEMEM:
                writeMem();
                break;
            default:
                error();
                break;
         }
    }
}
