#define WRITE_LENGTH 11
#define READ_LENGTH 8

byte getRelayState[] = {0x01, 0x03, 0xFD, 0x00, 0x00, 0x01};
byte relayOn[] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00};
byte relayOff[] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0xFF, 0x00};

int connectorID = 1;

struct CRC
{
     int highByte;
     int lowByte;
} CRC1;

CRC CALCULATE_CRC(unsigned int pure_crc)
{
     String str = String(pure_crc, HEX);
     String hightStr, lowStr;
     if (str.length() == 4)
     {
          hightStr = str.substring(0, 2);
          lowStr = str.substring(2, 4);
     }
     else if (str.length() == 3)
     {
          hightStr = str.substring(0, 1);
          lowStr = str.substring(1);
     }
     int lowByte = strtol(lowStr.c_str(), NULL, HEX);
     int highByte = strtol(hightStr.c_str(), NULL, HEX);
     return {highByte, lowByte};
}

unsigned char *MESSAGE_TO_CHAR(byte *buf, unsigned int length)
{
     unsigned int len = length - 2;
     static unsigned char arr[50];
     for (int i = 0; i < len; i++)
          arr[i] = buf[i];
     return arr;
}

static uint16_t MODBUS_CRC16_v2(unsigned char *buf, unsigned int length)
{
     unsigned int len = length - 2;
     static const uint16_t table[2] = {0x0000, 0xA001};
     uint16_t crc = 0xFFFF;
     unsigned int i = 0;
     char bit = 0;
     unsigned int Xor = 0;

     for (i = 0; i < len; i++)
     {
          crc ^= buf[i];

          for (bit = 0; bit < 8; bit++)
          {
               Xor = crc & 0x01;
               crc >>= 1;
               crc ^= table[Xor];
          }
     }
     return crc;
}

void APPEND_CRC2MESSAGE(byte *message, unsigned int length, CRC crc)
{
     message[length - 1] = crc.highByte;
     message[length - 2] = crc.lowByte;
}

byte *Function_Code(byte *func_code, unsigned int length, unsigned int connectorID)
{
     static byte arr[50];
     func_code[0] = connectorID;
     unsigned int crc = MODBUS_CRC16_v2(MESSAGE_TO_CHAR(func_code, length), length);
     CRC1 = CALCULATE_CRC(crc);
     APPEND_CRC2MESSAGE(func_code, length, CRC1);
     for (int i = 0; i < length; i++)
          arr[i] = func_code[i];
     return arr;
}
void setup()
{
     Serial.begin(115200);
     byte *mess = Function_Code(relayOff, WRITE_LENGTH, connectorID);
     for (int i = 0; i < WRITE_LENGTH; i++)
          Serial.print((String)mess[i] + " ");
}
void loop()
{
     
}
