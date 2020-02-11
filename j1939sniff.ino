/*
	A example simple CAN sniffer for the Due or a Teensy 3.6 or
	Teensy 4.0.
	
	You'll want to modify the code to look for the PGNs you're 
	interested in. If you want to print out a lot of data on the
	Due, you'll need to use the native USB port on the Due, and 
	uncomment the #define Serial line below.  The native port is 
	fast enough to easily print out all the traffic that flows 
	across a 250 kbit/s link.

	The Teensy USB port (Serial) is as fast as the Due native
	port.

	Requires the following libraries installed into Arduino Due:

	https://github.com/collin80/can_common
	https://github.com/collin80/due_can

	Requires the following library for the Teensy 3.6 or Teensy
	4.0 (ships with Teensyduino):

	https://github.com/tonton81/FlexCAN_T4
 */

#ifdef ARDUINO_TEENSY40
#define TEENSY 1
#endif
#ifdef ARDUINO_TEENSY36
#define TEENSY 1
#endif

#ifdef TEENSY
#  include <FlexCAN_T4.h>
#else //Due
#  include "variant.h" 
#  include <due_can.h>
//#  define Serial SerialUSB
#endif


#ifdef TEENSY
//Union for parsing CAN bus data messages. Warning:
//Invokes type punning, but this works here because
//CAN bus data is always little-endian (or should be)
//and the ARM processor on these boards is also little
//endian.
typedef union {
    uint64_t uint64;
    uint32_t uint32[2]; 
    uint16_t uint16[4];
    uint8_t  uint8[8];
    int64_t int64;
    int32_t int32[2]; 
    int16_t int16[4];
    int8_t  int8[8];

    //deprecated names used by older code
    uint64_t value;
    struct {
        uint32_t low;
        uint32_t high;
    };
    struct {
        uint16_t s0;
        uint16_t s1;
        uint16_t s2;
        uint16_t s3;
    };
    uint8_t bytes[8];
    uint8_t byte[8]; //alternate name so you can omit the s if you feel it makes more sense
} BytesUnion;

#  ifdef ARDUINO_TEENSY40
//use the first CAN port on the Teensy 4.0
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#  else
//use the only CAN port on the Teensy 3.6
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> Can0;
#  endif
#endif

unsigned long start_time;

static inline void print_hex(uint8_t *data, int len) {
	char temp[4];
	for (int b=0;b < len; b++) {
		sprintf(temp, "%.2x",data[b]);
		Serial.print(temp);
	}
	Serial.println("");
}

void j1939Decode(long ID, unsigned long* PGN, byte* priority, byte* src_addr, byte *dest_addr)
{
	/* decode j1939 fields from 29-bit CAN id */
	*src_addr = 255;
	*dest_addr = 255;

	*priority = (int)((ID & 0x1C000000) >> 26);

	*PGN = ID & 0x00FFFF00;
	*PGN = *PGN >> 8;

	ID = ID & 0x000000FF;
	*src_addr = (int)ID;

	/* decode dest_addr if it's a peer to peer message */
	if( (*PGN > 0 && *PGN <= 0xEFFF) ||
	    (*PGN > 0x10000 && *PGN <= 0x1EFFF) ) {
		*dest_addr = (int)(*PGN & 0xFF);
		*PGN = *PGN & 0x01FF00;
	}
}

#ifdef ARDUINO_TEENSY40
void got_frame_teensy(const CAN_message_t &frame) {
	got_frame(frame.id, frame.flags.extended, frame.len, (BytesUnion *)frame.buf);
}
#else

void got_frame_due(CAN_FRAME *frame) {
	got_frame(frame->id, frame->extended, frame->length, &(frame->data));
}
#endif

void got_frame(uint32_t id, uint8_t extended, uint8_t length, BytesUnion *data) {
	unsigned long PGN;
	byte priority;
	byte srcaddr;
	byte destaddr;

	j1939Decode(id, &PGN, &priority, &srcaddr, &destaddr);

	//could filter out what we want to look at here on any of these
	//variables.

	Serial.print(millis() - start_time);
	Serial.print(": ");
	Serial.print(PGN);
	Serial.print(",");
	Serial.print(priority);
	Serial.print(",");
	Serial.print(srcaddr);
	Serial.print(",");
	Serial.print(destaddr);
	Serial.print("/");
	Serial.print(id);
	Serial.print(",");
	Serial.print(extended);
	Serial.print(" ");
	print_hex(data->bytes, length);

	//example of a decode
	if (PGN == 65267) { //vehicle position message
		//latitude
		Serial.print(data->uint32[0] / 10000000.0 - 210.0);
		Serial.print(",");
		//longitude
		Serial.println(data->uint32[1] / 10000000.0 - 210.0);
	}

	//don't do anything with the frame since we're just sniffing.
}

void setup()
{
	delay(5000);
	Serial.begin(115200);
#ifdef TEENSY
	//Teensy FlexCAN_T4 setup
	Can0.begin();
	Can0.setBaudRate(250000);
	Can0.enableFIFO();
	Can0.enableFIFOInterrupt();
	Can0.onReceive(got_frame_teensy);
	Can0.enableMBInterrupts(FIFO);
	Can0.enableMBInterrupts();
#else
	//Due due_can setup
	Can0.begin(CAN_BPS_250K);
	for (int filter=0;filter <3; filter ++) {
		Can0.setRXFilter(0,0,true);
	}

	Can0.attachCANInterrupt(got_frame_due);
#endif

	start_time = millis();
}

void loop()
{
#ifdef TEENSY
	//process collected frames
	Can0.events();
#endif
}

