#define RX1 18
#define TX1 19
#define WRITE_LENGTH 11
#define READ_LENGTH 8
byte relayOn[] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x8A, 0x9F}; 
byte relayOff[] = {0x01, 0x10, 0xFD, 0x00, 0x00, 0x01, 0x02, 0xFF, 0x00, 0xCB, 0x6F};
byte energy[] = {0x01, 0x04, 0x01, 0x56, 0x00, 0x02, 0x90, 0x27};
// Voltage = 01 04 00 00 00 02 71 CB -> V
// Current = 01 04 00 06 00 02 91 CA -> A
// Active power = 01 04 00 0C 00 02 B1 C8 -> W
// Apparent power = 01 04 00 12 00 02 D1 CE -> VA
// Reactive power = 01 04 00 18 00 02 F1 CC -> VAr
// Power factor = 01 04 00 1E 00 02 11 CD
// Phase 1 phase angle = 01 04 00 24 00 02 31 C0 -> degrees
// Frequency of supply voltages = 01 04 00 46 00 02 90 E1 -> Hz
// Import active energy = 01 04 00 48 00 02 F1 DD -> kWh
// Export active energy = 01 04 00 4A 00 02 50 1D -> kWH
// Import reactive energy = 01 04 00 4C 00 02 B0 1C -> kVArh
// Export reactive energy = 01 04 00 4E 00 02 11 DC -> kVArh
// Total apparent energy = 01 04 00 50 00 02 71 DA -> kVAh
// Active power demand = 01 04 00 54 00 02 30 1B -> W
// Maximum active power demand = 01 04 00 56 00 02 91 DB -> W
// Apparent power demand = 01 04 00 64 00 02 30 14 -> VA
// Maximum apparent power demand = 01 04 00 66 00 02 91 D4 -> VA
// reactive power demand = 01 04 00 6C 00 02 B1 D6 -> VAr
// Maximum reactive power demand = 01 04 00 6E 00 02 10 16 -> VAr
// Current demand = 01 04 01 02 00 02 D1 F7 -> A
// Maximum current demand = 01 04 01 08 00 02 F1 F5 -> A
// total active energy = 01 04 01 56 00 02 90 27 -> kWh
// total reactive energy = 01 04 01 58 00 02 F1 E4 -> kVArh









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

void IMP_WriteModbus(byte funcCode[], int len)
{
     Serial1.write(funcCode, len);
}

struct readModbus
{
     double floatValue;
     String errorMessage;
     int *arr;
     int len;
} rModbus;

readModbus IMP_WriteReadModbus(byte funcCode[], int len)
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
          return {0x00, "read fail", arr, lenCount};
     else
     {
          double float4Byte = hex2float(arr, lenCount);
          return {float4Byte, "No error", arr, lenCount};
     }
}
