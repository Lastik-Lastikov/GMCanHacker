#include <SPI.h>
#include <Wire.h>

#define P_SD      10
#define LED        8

//-------------------------
// MCP2515 Pin Defines
#define P_CS       10
#define P_SCK     13
#define P_MOSI   11
#define P_MISO   12
#define P_INT     3
#define rxPin     7
#define txPin     6

//-------------------------
//MCP2515
#define CNF3            0x28
#define CNF2            0x29
#define CNF1            0x2A
#define CANINTE  0x2B
#define CANINTF  0x2C
#define RXB0CTRL        0x60
#define RXB1CTRL        0x70
#define RXM0SIDH        0x20
#define RXM0SIDL        0x21
#define RXM0EID8        0x22
#define RXM0EID0        0x23
#define RXM1SIDH        0x24
#define RXM1SIDL        0x25
#define RXM1EID8        0x26
#define RXM1EID0        0x27
#define CANCTRL  0x0F
#define TXB0SIDH        0x31
#define TXB0SIDL        0x32
#define TXB0DLC  0x35
#define TXB0D0    0x36
#define TXB0CTRL        0x30

//-------------------------
//RX Filter Bytes...
#define RXF0SIDH        0x00
#define RXF1SIDH        0x04
#define RXF2SIDH        0x08
#define RXF3SIDH        0x10
#define RXF4SIDH        0x14
#define RXF5SIDH        0x18

//-------------------------
// MCP2515 defs
#define EXIDE     (1<<3)
#define EXMASK   0x1FFFFFFF
#define MCP_DLC_MASK            0x0F    /* 4 LSBits */
#define MCP_SIDH                0
#define MCP_SIDL                1
#define MCP_EID8                2
#define MCP_EID0                3

byte extID;
unsigned long heady;
byte datalength;
byte message[8];
int ConnStatus=0; //connection status

//long id;
byte byData[8]; // Allocate some space for the Bytes
char ch;
String str;
String inString = "";


void setup()
{

  Serial.begin(115200);
  Serial1.begin(115200);
  // set up pins for MCP2515
  pinMode(P_CS, OUTPUT);
  pinMode(P_SCK, OUTPUT);
  pinMode(P_MOSI, OUTPUT);
  pinMode(P_MISO, INPUT);

  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();

  delay(1000);

  //  Serial1.println("Init MCP2515");
  mcp2515_init(33, 0x00);  //Iinitialize MCP2515: 1Mbit/s, 500, 125 Kbits/s, 33kps with filters enabled.
  mcp2515_write_register(CANCTRL, 0x00);
  delay(250);
}

void ReadSerial() {
  if (Serial1.available() > 0)
  {

    ch = Serial1.read(); // Read a Byte
    if (ch == 'T')  //Find "T" flag
    {
      for (int i=0; i<8; i++) 
      {
        ch = WaitAndRead();
        str += ch;
      }
      long idData = strtol( &str[0], NULL, 16);//convert ASCII HEX to INT
      int inChar = Serial1.read();   //read data length 
      if (isDigit(inChar)) {        //read data length
        inString += (char)inChar;
      }

      int length=inString.toInt(); // convert length  from char to int in bytes
      for (int i=0; i<length; i++) //add data bytes to array
      {
        byData[i]=SerialReadHexByte();
      }

      if (can_send_29bit_message(idData, sizeof(byData), byData)) //send to can
      {
        Serial.println("suceeded transmit");
      }
      else
      {
        Serial.println("failed");
      }
      //clear variables
      str="";
      inString = ""; 
      inChar=0;
      memset(byData, 0, sizeof(byData));
      idData =0;
    }
  }
}

void renderMsg(uint8_t address)
{
  mcp2515_read_can_id( address , &extID, &heady );
  mcp2515_read_canMsg( address, &datalength, &message[0]);
  processMessage();
}


void readCanAndSendToSerial() {

  byte flags;

  flags = mcp2515_read_register(CANINTF);
  if ((flags & 0x01) == 0x01)
  {
    renderMsg(0x61);
    // Clear the message flag..
    mcp2515_modifyRegister(CANINTF, 0x01, 0x00);
  }

  if ((flags & 0x02) == 0x02)
  {
    renderMsg(0x71);
    // Clear the message flag..
    mcp2515_modifyRegister(CANINTF, 0x02, 0x00);
  }
}


void processMessage() {


  Serial1.print("T");
  Serial1.print(heady, HEX);
  Serial1.print(datalength);
  for (int i = 0; i < datalength; i++)
  {
    char buffer[8];
    sprintf(buffer, "%02x", message[i]); Serial1.print(buffer);

  }
  Serial1.print("\r");



}

void do_connect() {

  if (Serial1.available() > 0) {
    ch = Serial1.read();
    delay(1000);
    if (ch =='V' || ch=='v') { //answer to version request

      Serial1.print("V1001\r");
      delay(1000);
    } 
    else if (ch =='O')  { //start main fuctions
      ConnStatus = 1;
      delay(1000);
    } 
  }
}
void loop()
{

  if (ConnStatus==1) {

    readCanAndSendToSerial();
    if (Serial1.available() > 0) {
      ReadSerial();
    }
  }
  else {
    do_connect();
  }



}

