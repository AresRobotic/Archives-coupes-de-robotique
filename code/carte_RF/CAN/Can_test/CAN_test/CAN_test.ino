#include <SPI.h>
#include <SoftwareSerial.h>
#include "CAN.h"

#define BUS_SPEED 125
#define RX 1
#define TX 0

int state;

#define GUESS_MIN		0b00000001
#define GUESS_MAX		0b00001111

CANClass CAN;

void setup() {           
	Serial.begin(9600);
	CAN.begin();
	CAN.baudConfig(BUS_SPEED);	
	CAN.resetFiltersAndMasks();
	CAN.setMaskOrFilter(MASK_0,   0b11111111, 0b11100000, 0b00000000, 0b00000000);
	CAN.setMaskOrFilter(MASK_1,   0b11111111, 0b11100000, 0b00000000, 0b00000000); 
	CAN.setMaskOrFilter(FILTER_2, 0b10101010, 0b10100000, 0b00000000, 0b00000000);

	//I had two nodes, they had different CPUS, so I could make one a slave and one a master
	//based on these cpus... you may well have to compile two different apps.
	
	#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
		state = TX; //TX = guessing
		Serial.print("This node will start off guessing...");
	#else
		state = RX; //RX = I set a number.
		Serial.print("This node will be waiting for the correct guess...");
	#endif 
	delay(1000);
	
	randomSeed(analogRead(0)); //random input from a non-connected pin.
}

byte guess = -1;
byte guess_count = 0;

byte length,rx_status,i;
uint32_t frame_id;
byte frame_data[8]; 
byte filter,ext;

void loop() {
	switch(state) {
		case TX: //I'm guessing!, try a random number.
			guess = random(GUESS_MIN, GUESS_MAX);
			guess_count++;
			
			frame_data[0] = guess;
			frame_data[1] = guess_count;
			frame_id = guess << 3;  
			length = 1;
	  
			CAN.load_ff_0(length,&frame_id,frame_data,false);
			 
			Serial.print(guess);
			Serial.print(",");
			delay(250);
			
			
			if (CAN.buffer1DataWaiting()) {
				CAN.readDATA_ff_1(&length,frame_data,&frame_id,&ext,&filter);
				Serial.print("I think I got it?:");
				Serial.println(frame_data[0]);
				state = RX;
				guess_count = 0;
			} else {
				frame_data[0] = guess;
				frame_data[1] = guess_count;  
				frame_id = 0x555;
				length = 2;
				CAN.load_ff_0(length,&frame_id,frame_data,false);
				delay(750);
			}
			break;
		case RX: //SET the mask and let the other guess.
			guess = random(GUESS_MIN, GUESS_MAX);
			CAN.setMaskOrFilter(FILTER_0, guess,      0b00000000, 0b00000000, 0b00000000);
			Serial.print("Guess has been set at: ");
			Serial.println(guess);
			
			while (state == RX) {
				rx_status = CAN.readStatus();
				if (CAN.buffer0DataWaiting()) {
					CAN.readDATA_ff_0(&length,frame_data,&frame_id,&ext,&filter);
					Serial.print("{#");
					Serial.print(frame_data[1]);
					Serial.print("}:");
					Serial.print(frame_data[0]);
					Serial.println("!");
					Serial.print("Other party got it on filter: ");
					Serial.println(filter);
					
					frame_data[0] = 1;
					frame_id = 0x555;
					length = 1;
					CAN.load_ff_0(length,&frame_id,frame_data,false);
					
					state = TX;
					delay(2000);
					Serial.println("\nSender:");
					
				}

                                if (CAN.buffer1DataWaiting()) {
					CAN.readDATA_ff_1(&length,frame_data,&frame_id,&ext,&filter);
					Serial.print("[#");
					Serial.print(frame_data[1]);
					Serial.print("]:");
					Serial.print(frame_data[0]);
					Serial.print(", ");
				}
			}
			break;
	}
}

