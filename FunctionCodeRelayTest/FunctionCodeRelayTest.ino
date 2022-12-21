#include "crc.h"
#define RX1 18
#define TX1 19
#define WRITE_LENGTH 11
#define READ_LENGTH 8

byte relayOn[WRITE_LENGTH] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00}; // 8A
byte relayOn1[WRITE_LENGTH] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x8A, 0x9F};
byte relayOff[WRITE_LENGTH] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0xFF, 0x00, 0x00 ,0x00};
byte readRelay[READ_LENGTH] = {0x01, 0x03, 0xFD, 0x00, 0x00, 0x01};
byte energy[READ_LENGTH] = {0x01, 0x04, 0x01, 0x56, 0x00, 0x02};

struct readModbus
{
  double floatValue;
  String errorMessage;
  int *arr;
  int len;
} rModbus;

readModbus Write_Read_Modbus(byte *funcCode, int len)
{
  static int arr[50]; // create a buffer for recieving data from meter
  int lenCount = 0;
  Serial1.write(funcCode, len);
  delay(50);

  while (Serial1.available())
  {
    arr[lenCount] = Serial1.read();
    lenCount++;
  }
  if (lenCount == 0)
  {
    return {0x00, "read fail", arr, lenCount};
  }
  else
  {
    double float4Byte = hex2float(arr, lenCount);
    return {float4Byte, "No error", arr, lenCount};
  }
}
double hex2float(int hexArr[], int len)
{
  String strBIN = "";
  for (size_t i = len - 6; i <= len - 3; i++)
  {
    String arr = String(hexArr[i], BIN);
    int lenArr = arr.length();
    if (lenArr < 8)
    {
      for (size_t i = 0; i < 8 - lenArr; i++)
      {
        arr = "0" + arr;
      }
    }
    strBIN += arr;
  }
  if (strBIN.length() <= 32)
  {
    int signBit;
    if (strBIN[0] == '0')
      signBit = 1;
    else if (strBIN[0] == '1')
      signBit = -1;
    String compBIN = "";
    for (size_t i = 1; i < 9; i++)
    {
      compBIN += strBIN[i];
    }
    int compINT = strtol(compBIN.c_str(), NULL, 2);
    double actualComp = pow(2, compINT - 127);
    String mantissa = "";
    double actualMantis = 0;
    for (size_t i = 9; i < strBIN.length(); i++)
    {
      mantissa += strBIN[i];
    }
    for (int i = 0; i < mantissa.length(); i++)
    {
      actualMantis += String(mantissa[i]).toDouble() * pow(2, -(i + 1));
    }
    actualMantis = actualMantis + 1;
    return signBit * actualComp * actualMantis;
  }
}



void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N2, RX1, TX1);
  
  rModbus = Write_Read_Modbus(Function_Code(relayOn, WRITE_LENGTH, 1), WRITE_LENGTH);
  delay(100);
  rModbus = Write_Read_Modbus(Function_Code(relayOff, WRITE_LENGTH, 1), WRITE_LENGTH);
}
void loop()
{

}
