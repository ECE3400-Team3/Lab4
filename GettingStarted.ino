/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"



typedef enum {
  WALL_UP = 1,
  WALL_DOWN = 2,
  WALL_LEFT = 4,
  WALL_RIGHT = 8,
  FINISHED = 128,
} FLAGS;

#define  WALL_UP 1;
#define  WALL_DOWN 2;
#define  WALL_LEFT 4;
#define  WALL_RIGHT 8;
#define  NO_TREASURE 0;
#define  KHZ_7 1;
#define  KHZ_9 2;
#define  KHZ_11 3;
#define  KHZ_13 4;
#define  KHZ_15 5;
#define  KHZ_17 6;
#define  FINISHED 128;

char info[2] ={0};
char testInfo[2] = {0};
bool wallUp = 0;
bool wallDown = 0;
bool wallLeft = 0;
bool wallRight = 0;
uint8_t xPos;
uint8_t yPos;
//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000000CLL, 0x000000000DLL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

unsigned char maze[5][5] =
{
  3, 3, 3, 3, 3,
  3, 1, 1, 1, 3,
  3, 2, 0, 1, 2,
  3, 1, 3, 1, 3,
  3, 0, 3, 1, 0,
};

unsigned char testMaze[5][5] = {0};

unsigned char newInfo[3]={0,0,0};
unsigned char rxInfo[3]={0};

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel  
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

void loop(void)
{
  xPos = 7;
  yPos = 7;
  wallUp = 1;
  wallDown = 1;
  wallLeft = 1;
  wallRight = 1;
  info[1] = 0;
  info[0] = (((xPos << 4) & 0xF0) | (yPos & 0x0F));
  if ( wallUp) info[1] |= WALL_UP;
  if ( wallDown) info[1] |= WALL_DOWN;  
  if ( wallLeft) info[1] |= WALL_LEFT; 
  if ( wallRight) info[1] |= WALL_RIGHT;

  //treasure
  int treasure = KHZ_17;
  if (treasure) info[1] |= (treasure << 4 & 0x70);
  
  //
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
  {
    
    int attempt = 0;
    maze[0][0] += 1;
    while ( attempt < 3 ) { //attempt to send three times
      // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    printf("Now sending %d and %d.",info[0], info[1]);
    //bool ok = radio.write( &time, sizeof(unsigned long) );
    bool ok = radio.write( &info, sizeof(info) );

//     newInfo[0] += 1;
//     newInfo[1] += 1;

     
      
//      bool ok= radio.write( newInfo, sizeof(newInfo) );

    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);

      delay (2000); 
      
      break;
    }

    // Try again 0.5s later
    delay(500);
    
    attempt+=1; // try again.
    }
   }
    

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned long got_time;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        //done = radio.read( &got_time, sizeof(unsigned long) );
        done = radio.read( testInfo, sizeof(testInfo) );
//        done=radio.read(rxInfo,sizeof(rxInfo));
        
        // Spew it
        Serial.println("Recieved");

        Serial.println(testInfo[0], BIN);
        Serial.println(testInfo[1], BIN);

        //if

        /*
        for (int i = 0; i < 5; i++) {
          for (int j = 0; j < 5; j++) {
            Serial.print(testMaze[i][j]); Serial.print(" ");
          }
          Serial.println("");
        }
        */
        
//        printf("X: %d, Y: %d Data:%d",rxInfo[0],rxInfo[1],rxInfo[2]);
//        Serial.println(" ");
        
        // Delay just a little bit to let the other unit
        // make the transition t  o receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &got_time, sizeof(unsigned long) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }

  //
  // Change roles
  //

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");

      // Become the primary receiver (pong back)
      role = role_pong_back;
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
