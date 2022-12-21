#include "plugEvseStatus.h"
void setup(){
  Serial.begin(115200);
  Plug_EVSEStatus_Setup();
}
void loop(){
  Enable_Plug_EVSEStatus();
}
