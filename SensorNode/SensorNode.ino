/*
 Author: Alexandre Bouillot
 Based on work from: Eric Tsai
 License: CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
 Date: 2014/04/13
 File: SensorNode.ino
 This sketch is for a wired Arduino w/ RFM69 wireless transceiver
 Sends sensor data (temp/humidity) back  to gateway.  
 Receive sensor messages from the gateway
 */


/* sensor
 node = 13
 device ID
 2 = 1222 = smoke or not
 3 = 1232 = flame detected or not
 4 = 1242 = human motion present or not
 5 = 1252 = barking or not
 6 = 1262, 1263 = temperature, humidity
 
 */
  


//general --------------------------------
#define SERIAL_BAUD   115200
#if 1
#define DEBUG1(expression)  Serial.print(expression)
#define DEBUG2(expression, arg)  Serial.print(expression, arg)
#define DEBUGLN1(expression)  Serial.println(expression)
#else
#define DEBUG1(expression)
#define DEBUG2(expression, arg)
#define DEBUGLN1(expression)
#endif


//RFM69  --------------------------------------------------------------------------------------------------
#include <RFM69.h>
#include <SPI.h>
#define NODEID        200    //unique for each node on same network
#define NETWORKID     101  //the same on all nodes that talk to each other
#define GATEWAYID     1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define LED           13  // Moteinos have LEDs on D9
#define SERIAL_BAUD   115200  //must be 9600 for GPS, use whatever if no GPS

// If you are using Adafruit Trinket Pro, please uncomment the next TWO lines. On Adafruit Trinket Pro you cannot use the INT0 (and pin 2) so you need to use pin 3 and INT1
#define RF69_INTERRUPT_PIN 3
#define RF69_INTERRUPT_NUMBER 1

#ifdef RF69_INTERRUPT_PIN
RFM69 radio(RF69_SPI_CS, RF69_INTERRUPT_PIN, false, RF69_INTERRUPT_NUMBER) ;
#else
RFM69 radio;
#endif


boolean debug = 0;

//struct for wireless data transmission
typedef struct {		
  int       nodeID; 		//node ID (1xx, 2xx, 3xx);  1xx = basement, 2xx = main floor, 3xx = outside
  int       deviceID;		//sensor ID (2, 3, 4, 5)
  unsigned long   var1_usl; 		//uptime in ms
  float     var2_float;   	//sensor data?
  float     var3_float;		//battery condition?
} Payload;
Payload theData;

char buff[20];
byte sendSize=0;
boolean requestACK = true;

//end RFM69 ------------------------------------------


// timings
unsigned long temperature_time;

unsigned long frameSent = 0;
unsigned long ackMissed = 0;
unsigned long ackReceived = 0;
boolean statOut;

// Anarduino led is on pin D9
int led = 13;

void setup()
{
  Serial.begin(SERIAL_BAUD);          //  setup serial
  DEBUG1("starting");

  //RFM69-------------------------------------------
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGLN1(buff);
  theData.nodeID = NODEID;  //this node id should be the same for all devices in this node
  //end RFM--------------------------------------------
  
  pinMode(led, OUTPUT);
  radio.promiscuous(true);
}

long blinkInterval = 3000;
long blinkNext = 0;
bool high = false;

void loop()
{
  if (millis() > blinkNext)
  {
      if (high) {
       digitalWrite(led, LOW);   
      } else {
        digitalWrite(led, HIGH);
      }
      high = ! high;
      blinkNext = millis() + blinkInterval;
  }
  
  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    if (radio.DATALEN == 8) { // ACK TEST
      for (byte i = 0; i < radio.DATALEN; i++)
        DEBUG1((char)radio.DATA[i]);
    }
    else if(radio.DATALEN == sizeof(Payload)) {
      for (byte i = 0; i < radio.DATALEN; i++) {
        DEBUG2((char)radio.DATA[i], HEX);
        DEBUG1(".");
      }
      DEBUGLN1();
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else

      DEBUG1("Received Device ID = ");
      DEBUGLN1(theData.deviceID);  
      DEBUG1 ("    Time = ");
      DEBUGLN1 (theData.var1_usl);
      DEBUG1 ("    var2_float ");
      DEBUGLN1 (theData.var2_float);
      DEBUG1 ("    var3_float ");
      DEBUGLN1 (theData.var3_float);
      
    }
    else {
      Serial.print("Invalid data ");
      for (byte i = 0; i < radio.DATALEN; i++) {
        DEBUG2((char)radio.DATA[i], HEX);
        DEBUG1(".");
      }
    }

    DEBUG1("   [RX_RSSI:");DEBUG1(radio.RSSI);DEBUG1("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      DEBUG1(" - ACK sent");
    }
    DEBUGLN1();
  }
  
  if (false && frameSent % 20 == 0 ) {
    //send data
    theData.deviceID = 1;
    theData.var1_usl = millis();
    theData.var2_float = frameSent;
    theData.var3_float = ackMissed;
    frameSent++;
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData), 1, 300)) {
      ackReceived++;
      DEBUGLN1("ACK received");
    } else {
      ackMissed++;
    }
  }

  if ( frameSent % 10 == 0 ) {
    if ( statOut == 0 ) {
      statOut = 1;
      DEBUG1("Frames ");
      DEBUG1(frameSent);
      DEBUG1(" missed: ");
      DEBUG1(ackMissed);
      DEBUG1(" ACKnowledge: ");
      DEBUGLN1(ackReceived);
    }
  } else { statOut = 0; }
  
  unsigned long time_passed = 0;


  //send data
  if (false && millis() > blinkNext) {
    theData.deviceID = 6;
    theData.var1_usl = millis();
    theData.var2_float = 10;
    theData.var3_float = 20;
    frameSent++;
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData), 1, 300)) {
      ackReceived++;
      DEBUGLN1(" ACK received");
    } else {
      ackMissed++;
    }
  }
  
}//end loop








