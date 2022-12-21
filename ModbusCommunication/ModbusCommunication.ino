#include "modbusCommunication.h"

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N2, RX1, TX1);

  rModbus = Write_Read_Modbus(energy, READ_LENGTH);
  Serial.println(rModbus.len);
  for (int i = 0; i < rModbus.len; i++) {
    Serial.print((String)rModbus.arr[i] + " ");
  }
  Serial.println();
  Serial.println(rModbus.floatValue);
  Serial.println(rModbus.errorMessage);
}
void loop()
{
}
