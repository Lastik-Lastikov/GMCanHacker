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

long id;
byte byData[8]; // Allocate some space for the Bytes
int index = 0;   // Index into array; where to store the Bytes
char ch;
String str;
int counter;
int type;
int length;
//int msgCount = 0;
//int i2c_counter;
//byte command[8]; //
byte answer[32]; // 

boolean acc = false;

void setup()
{

  Serial.begin(115200);
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

  //  Serial.println("Init MCP2515");
  mcp2515_init(33, 0x00);  //Iinitialize MCP2515: 1Mbit/s, 500, 125 Kbits/s, 33kps with filters enabled.
  mcp2515_write_register(CANCTRL, 0x00);
  delay(250);
}


void ReadSerial() {
  
 String canID;
 char inchar;
 int  canLen;
 byte data[8];
 
 while (Serial.available() > 0)
  {inchar = Serial.read();
    delay(10);
    if (inchar == 'T') {
        for (int i = 0; i<8; i++)
        {
          canID += Serial.read();
        }
        canLen = Serial.read();
        Serial.readBytes(data, canLen);
    if (can_send_29bit_message(canID.toInt(), sizeof(data), data))
      {
        Serial.println("suceeded");
      }
      else
      {
        Serial.println("failed");
      }
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

 
    Serial.print("T");
    Serial.print(heady, HEX);
    Serial.print("8");
    for (int i = 0; i < 8; i++) 
    {
      Serial.print(message[i], HEX);
    }
    Serial.print("\r");



}
void loop()
{
//Если нужно сгенерировать сообшение
// msgGen(12345678);
  readCanAndSendToSerial();
  if (Serial.available() > 0) {
    ReadSerial();
  }


}

