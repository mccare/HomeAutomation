/*
RFM69 Gateway RFM69 pushing the data to the mosquitto server
by Alexandre Bouillot

License:  CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
Date:  2015-06-12
File: Gateway.c

This sketch receives RFM wireless data and forwards it to Mosquitto relay
It also subscripe to Mosquitto Topics starting with RFM/<network_number> followed  by /<node_id>
The message is parsed and put bak to the same payload structure as the one received from teh nodes


Modifications Needed:
1)  Update encryption string "ENCRYPTKEY"
2)  Adjust network id, node id, frequency and model of RFM
*/

//general --------------------------------
#define SERIAL_BAUD   115200
#ifdef DEBUG
#define DEBUG1(expression)  fprintf(stderr, expression)
#define DEBUG2(expression, arg)  fprintf(stderr, expression, arg)
#define DEBUGLN1(expression)  
#define LOG(...) do { printf(__VA_ARGS__); } while (0)
#define LOG_E(...) do { printf(__VA_ARGS__); } while (0)
#else
#define DEBUG1(expression)
#define DEBUG2(expression, arg)
#define DEBUGLN1(expression)
#define LOG(...)
#define LOG_E(...)
#endif

//RFM69  ----------------------------------
#include "rfm69.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>


#define RFM_FREQUENCY RF69_868MHZ

#ifdef GATEWAY
#define NODE_ID 1
#define GATEWAY_ID 2
#else
#define NODE_ID 2
#define GATEWAY_ID 1
#endif

#define NETWORK_ID 100
#define ENCRYPTION_KEY  "sampleEncryptKey"

RFM69 *rfm69;

typedef struct {
	uint8_t networkId;
	uint8_t nodeId;
	uint8_t frequency; // RF69_433MHZ RF69_868MHZ RF69_915MHZ
	uint8_t keyLength; // set to 0 for no encryption
	char key[16];
	bool isRFM69HW;
	bool promiscuousMode;
}
Config;
Config theConfig;

typedef struct {		
	short           nodeID; 
	short			sensorID;
	unsigned long   var1_usl; 
	float           var2_float; 
	float			var3_float;	
} 
Payload;
Payload theData;


static void die(const char *msg);
static long millis(void);
static void hexDump (char *desc, void *addr, int len, int bloc);

static int initRfm(RFM69 *rfm);
static void send_message();
static int run_loop();


static void uso(void) {
	fprintf(stderr, "Use:\n Simply use it without args :D\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	if (argc != 1) uso();


	//RFM69 ---------------------------
	theConfig.networkId = NETWORK_ID;
	theConfig.nodeId = NODE_ID;
	theConfig.frequency = RFM_FREQUENCY;
	theConfig.keyLength = 16;
	memcpy(theConfig.key, ENCRYPTION_KEY, 16);
	theConfig.isRFM69HW = false;
	theConfig.promiscuousMode = true;
	LOG("NETWORK %d NODE_ID %d FREQUENCY %d\n", theConfig.networkId, theConfig.nodeId, theConfig.frequency);
	
	rfm69 = new RFM69();
	rfm69->initialize(theConfig.frequency,theConfig.nodeId,theConfig.networkId);
	initRfm(rfm69);
	
	LOG("setup complete\n");
	return run_loop();
}

/* Loop until it is explicitly halted or the network is lost, then clean up. */
static int run_loop() {
	int counter = 0;
	for (;;) {
		
		if (rfm69->receiveDone()) {
			LOG("Received something...\n");
			// store the received data localy, so they can be overwited
			// This will allow to send ACK immediately after
			uint8_t data[RF69_MAX_DATA_LEN]; // recv/xmit buf, including header & crc bytes
			uint8_t dataLength = rfm69->DATALEN;
			memcpy(data, (void *)rfm69->DATA, dataLength);
			uint8_t theNodeID = rfm69->SENDERID;
			uint8_t targetID = rfm69->TARGETID; // should match _address
			uint8_t PAYLOADLEN = rfm69->PAYLOADLEN;
			uint8_t ACK_REQUESTED = rfm69->ACK_REQUESTED;
			uint8_t ACK_RECEIVED = rfm69->ACK_RECEIVED; // should be polled immediately after sending a packet with ACK request
			int16_t RSSI = rfm69->RSSI; // most accurate RSSI during reception (closest to the reception)

			if (ACK_REQUESTED  && targetID == theConfig.nodeId) {
				// When a node requests an ACK, respond to the ACK
				// but only if the Node ID is correct
				rfm69->sendACK();
			}//end if radio.ACK_REQESTED
	
			LOG("[%d] to [%d] ", theNodeID, targetID);

			if (dataLength != sizeof(Payload)) {
				LOG("Invalid payload received, not matching Payload struct! %d - %d\r\n", dataLength, sizeof(Payload));
				hexDump(NULL, data, dataLength, 16);		
			} else {
				theData = *(Payload*)data; //assume radio.DATA actually contains our struct and not something else

				LOG("Received Node ID = %d Device ID = %d Time = %d  RSSI = %d var2 = %f var3 = %f\n",
					theData.nodeID,
					theData.sensorID,
					theData.var1_usl,
					RSSI,
					theData.var2_float,
					theData.var3_float
				);
			}  
		} //end if radio.receive
		
		counter = counter + 1;
#ifndef GATEWAY
		if (counter % 40 == 0) {
			LOG("Sending test message\n");
			send_message();
		} else {
			usleep(100*1000);
		}
		if (counter % 100 == 0) {
			rfm69->readAllRegs();
		}
#endif
			
		
	}

}

static int initRfm(RFM69 *rfm) {
	rfm->restart(theConfig.frequency,theConfig.nodeId,theConfig.networkId);
	if (theConfig.isRFM69HW)
		rfm->setHighPower(); //uncomment only for RFM69HW!
	if (theConfig.keyLength)
		rfm->encrypt(theConfig.key);
	rfm->promiscuous(theConfig.promiscuousMode);
	LOG("Listening at %d Mhz...\n", theConfig.frequency==RF69_433MHZ ? 433 : theConfig.frequency==RF69_868MHZ ? 868 : 915);
}

/* Fail with an error message. */
static void die(const char *msg) {
	fprintf(stderr, "%s", msg);
	exit(1);
}

static long millis(void) {
	struct timeval tv;

    gettimeofday(&tv, NULL);

    return ((tv.tv_sec) * 1000 + tv.tv_usec/1000.0) + 0.5;
	}

	
/* Binary Dump utility function */
#define MAX_BLOC 16
const unsigned char hex_asc[] = "0123456789abcdef";
static void hexDump (char *desc, void *addr, int len, int bloc) {
    int i, lx, la, l, line;
	long offset = 0;
    unsigned char hexbuf[MAX_BLOC * 3 + 1];	// Hex part of the data (2 char + 1 space)
	unsigned char ascbuf[MAX_BLOC + 1];	// ASCII part of the data
    unsigned char *pc = (unsigned char*)addr;
	unsigned char ch;
	
	// nothing to output
	if (!len)
		return;

	// Limit the line length to MAX_BLOC
	if (bloc > MAX_BLOC) 
		bloc = MAX_BLOC;
		
	// Output description if given.
    if (desc != NULL)
		LOG("%s:\n", desc);
	
	line = 0;
	do
		{
		l = len - (line * bloc);
		if (l > bloc)
			l = bloc;
	
		for (i=0, lx = 0, la = 0; i < l; i++) {
			ch = pc[i];
			hexbuf[lx++] = hex_asc[((ch) & 0xF0) >> 4];
			hexbuf[lx++] = hex_asc[((ch) & 0xF)];
			hexbuf[lx++] = ' ';
		
			ascbuf[la++]  = (ch > 0x20 && ch < 0x7F) ? ch : '.';
			}
	
		for (; i < bloc; i++) {
			hexbuf[lx++] = ' ';
			hexbuf[lx++] = ' ';
			hexbuf[lx++] = ' ';
		}	
		// nul terminate both buffer
		hexbuf[lx++] = 0;
		ascbuf[la++] = 0;
	
		// output buffers
		LOG("%04x %s %s\n", line * bloc, hexbuf, ascbuf);
		
		line++;
		pc += bloc;
		}
	while (line * bloc < len);
}



static void send_message() {
	Payload data;
	uint8_t network;
	data.nodeID = GATEWAY_ID;
	data.sensorID = 10;
	data.var1_usl = 1000;
	data.var2_float = 99.0;
	data.var3_float = 101.0;
	
	LOG("Will Send message to Node ID = %d Device ID = %d Time = %d  var2 = %f var3 = %f\n",
		data.nodeID,
		data.sensorID,
		data.var1_usl,
		data.var2_float,
		data.var3_float
	);

	if (rfm69->sendWithRetry(data.nodeID,(const void*)(&data),sizeof(data))) {
		LOG("Message sent to node %d ACK\n", data.nodeID);
	}
	else {
		LOG("Message sent to node %d NAK\n", data.nodeID);
	}
	
}

