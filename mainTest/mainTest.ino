#include "modbusCommunication.h"
#include "plugEvseStatus.h"

unsigned long previousMillis = 0; // *delete later
unsigned int period = 500;
String currentState;
bool stateRelay = 0;


void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N2, RX1, TX1);
  Plug_EVSEStatus_Setup();
  Write_Modbus(relayOn, WRITE_LENGTH);
  delay(50);
}
void loop()
{
  Enable_Plug_EVSEStatus();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= period)
  {
    previousMillis = currentMillis;
    Serial.println(plugParams.statePlug);
    Serial.println((String)"PWM : " + plugParams.PWM1_DutyCycle);
    Serial.println((String)"Max : " + vMax + " Min : " + vMin);
    if (currentState != plugParams.statePlug) {
      currentState = plugParams.statePlug;
      switch (hashitPlugState(plugParams.statePlug))
      {
        case notConnected:
          Write_Modbus(relayOn, WRITE_LENGTH);
          stateRelay = 0;
          delay(50);
          break;
        case diodeFailed:
          Write_Modbus(relayOn, WRITE_LENGTH);
          stateRelay = 0;
          delay(50);
          break;
        case connected:
          if (stateRelay == 1) {
            Write_Modbus(relayOn, WRITE_LENGTH);
            stateRelay = 0;
            delay(50);
          }
          break;
        case charging:
          Write_Modbus(relayOff, WRITE_LENGTH);
          stateRelay = 1;
          delay(50);
          break;
        case finishedCharging:
          Write_Modbus(relayOn, WRITE_LENGTH);
          stateRelay = 0;
          delay(50);
          break;
      }
    }
  }
}
