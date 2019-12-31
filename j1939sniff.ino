/*
	A example simple CAN sniffer for the Due.  You'll want to 
	modify the code to look for the PGNs you're interested in.
	If you 	want to print out a lot of data, you'll need to use 
	the native USB port on the Due, and uncomment the #define
	below.  The native port is fast enough to easily print out
	all the traffic that flows across a 250 kbit/s link.

	Requires the following libraries installed into Arduino:

	https://github.com/collin80/can_common
	https://github.com/collin80/due_can
 */

#include "variant.h" 
#include <due_can.h>

//#define Serial SerialUSB

static inline void print_hex(uint8_t *data, int len) {
	char temp[4];
	for (int b=0;b < len; b++) {
		sprintf(temp, "%.2x",data[b]);
		Serial.print(temp);
	}
	Serial.println("");
}

bool j1939PeerToPeer(long lPGN)
{
	// Check the PGN 
	if(lPGN > 0 && lPGN <= 0xEFFF)
		return true;

	if(lPGN > 0x10000 && lPGN <= 0x1EFFF)
		return true;

	return false;

}

void j1939Decode(long lID, unsigned long* lPGN, byte* nPriority, byte* nSrcAddr, byte *nDestAddr)
{
	/* decode j1939 fields from 29-bit CAN id */
	byte nRetCode = 1;

	*nSrcAddr = 255;
	*nDestAddr = 255;

	long lPriority = lID & 0x1C000000;
	*nPriority = (int)(lPriority >> 26);

	*lPGN = lID & 0x00FFFF00;
	*lPGN = *lPGN >> 8;

	lID = lID & 0x000000FF;
	*nSrcAddr = (int)lID;

	if(j1939PeerToPeer(*lPGN) == true)
	{
		*nDestAddr = (int)(*lPGN & 0xFF);
		*lPGN = *lPGN & 0x01FF00;
	}
}

void got_frame(CAN_FRAME *frame) {
	unsigned long PGN;
	byte priority;
	byte srcaddr;
	byte destaddr;

	j1939Decode(frame->id, &PGN, &priority, &srcaddr, &destaddr);

	//could filter out what we want to look at here on any of these
	//variables.
	Serial.print(PGN);
	Serial.print(",");
	Serial.print(priority);
	Serial.print(",");
	Serial.print(srcaddr);
	Serial.print(",");
	Serial.print(destaddr);
	Serial.print(",");
	print_hex(frame->data.bytes, frame->length);

	//example of a decode
	if (PGN == 65267) { //vehicle position message
		//latitude
		Serial.print(frame->data.uint32[0] / 10000000.0 - 210.0);
		Serial.print(",");
		//longitude
		Serial.println(frame->data.uint32[1] / 10000000.0 - 210.0);
	}

	//don't do anything with the frame since we're just sniffing.
}

void setup()
{
	Serial.begin(115200);
	Can0.begin(CAN_BPS_250K);

	for (int filter=0;filter <3; filter ++) {
		Can0.setRXFilter(0,0,true);
	}

	Can0.attachCANInterrupt(got_frame);
}

void loop()
{
}

