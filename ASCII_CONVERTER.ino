byte WaitAndRead()
{
   while (Serial.available() == 0) {
       // do nothing
   }
   return (byte) Serial.read();
}

int SerialReadHexDigit()
{
   byte c = WaitAndRead();
   if (c >= '0' && c <= '9') {
       return c - '0';
   } else if (c >= 'a' && c <= 'f') {
       return c - 'a' + 10;
   } else if (c >= 'A' && c <= 'F') {
       return c - 'A' + 10;
   } else {
       return -1;   // getting here is bad: it means the character was invalid
   }
}

int SerialReadHexByte()
{
   byte a = SerialReadHexDigit();
   byte b = SerialReadHexDigit();
   if (a<0 || b<0) {
       return -1;  // an invalid hex character was encountered
   } else {
       return (a*16) + b;
   }
} 
