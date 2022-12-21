#define PWM_WIRTE_PIN 26 //25 DEFULT
#define PWM1_CH 0
#define PWM1_RES 8
#define PWM1_FREQ 1000
#define ANALOGREAD_PIN 25  //26 DEFULT
#define MAX_AMP 18                // *edit later
#define FULL_DUTYCYCLE 255
#define NO_SAMPLES 400
unsigned long previousMillis = 0; // *delete later  
const int period = 500;

struct plugParams
{
     bool statusPlugConnect = false;     // for check of plugEVSE connects to EV
     bool statusFinishCharging = false;  // for check of having charge and finish
     unsigned int PWM1_DutyCycle = FULL_DUTYCYCLE;  // max value, direct voltage;
     String statePlug = "not connected"; // variable for all plugEVSE status
} plugParams;
 
unsigned int dutyCycleCal(int maxAmp)
{
     return round(maxAmp / 0.6);
}
struct checkAnalog
{
     unsigned int valMax;
     unsigned int valMin;
};
checkAnalog readVal(int analogPinToTest, int noSamples)
{
     int maximum = 0;
     int minimum = 5000;
     int value;

     for (int i = 0; i <= noSamples; i++)
     {
          value = analogRead(analogPinToTest);
          if (value <= minimum)
               minimum = value;
          if (value >= maximum)
               maximum = value;
     }
     return {maximum, minimum};
}
void checkEVSEconnect(checkAnalog readCP)
{
     if (readCP.valMax >= 3779 and readCP.valMax < 4096)
     {
          plugParams.statusPlugConnect = false;
          plugParams.PWM1_DutyCycle = FULL_DUTYCYCLE;
          plugParams.statePlug = "not connected";
          plugParams.statusFinishCharging = false;
     }
     else
     {
          plugParams.statusPlugConnect = true;
          plugParams.PWM1_DutyCycle = dutyCycleCal(MAX_AMP);
//          plugParams.PWM1_DutyCycle = FULL_DUTYCYCLE;
     }
}
// ###################################### Analog-range adjustment ######################################
String checkState(int adcMax, int adcMin)
{
     if (adcMin >= 550)
     {
          plugParams.statusFinishCharging = false;
          return "diode failed";
     }
     else if (adcMax >= 3150 and adcMax < 3779)
     {
          if (plugParams.statusFinishCharging == true)
               return "finished charging";
          else
               return "connected";
     }
     else if (adcMax >= 2618 and adcMax < 3150)
     {
          plugParams.statusFinishCharging = true;                       
          return "charging";
     }
}
void Plug_EVSEStatus_Setup()
{
     ledcAttachPin(PWM_WIRTE_PIN, PWM1_CH);
     ledcSetup(PWM1_CH, PWM1_FREQ, PWM1_RES);
}
void Enable_Plug_EVSEStatus()
{
     unsigned long currentMillis = millis();
     ledcWrite(PWM1_CH, plugParams.PWM1_DutyCycle);
     checkAnalog readCP;
     readCP = readVal(ANALOGREAD_PIN, NO_SAMPLES);
     checkEVSEconnect(readCP);
     if (currentMillis - previousMillis >= period)
     {
          if (plugParams.statusPlugConnect)
          {
               plugParams.statePlug = checkState(readCP.valMax, readCP.valMin);
          }
          previousMillis = currentMillis;
          Serial.println((String) "max : " + readCP.valMax + " min : " + readCP.valMin + " state : " + plugParams.statePlug);
          Serial.println((String) "PWM : " + plugParams.PWM1_DutyCycle);
     }
}
