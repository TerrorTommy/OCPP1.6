#define PWM_WIRTE_PIN 5
#define PWM1_CH 0
#define PWM1_RES 8
#define PWM1_FREQ 1000            // PWM frequency
#define ANALOGREAD_PIN 15
#define MAX_AMP 30                // *edit later
#define MAX_SAMPLES 400

// ###################################### convert plug-state to enum for using of switch-case ######################################
enum plugState
{
     notConnected,
     diodeFailed,
     connected,
     charging,
     finishedCharging,
};
// Real time EVSE plug status
plugState EV_CONNECT_STATE = notConnected;
plugState _STATUS_PLUG = notConnected; // variable for all plugEVSE status
struct plugParams
{
     bool statusPlugConnect = false;     // for check of plugEVSE connects to EV
     bool statusFinishCharging = false;  // for check of having a charge and already finish
     unsigned int PWM1_DutyCycle = 255;  // max value, direct voltage;
     // String statePlug = "not connected"; // variable for all plugEVSE status
} plugParams;

unsigned int dutyCycleCal(int maxAmp)
{
     return round(((maxAmp / 0.6) / 100) * plugParams.PWM1_DutyCycle);
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
          plugParams.PWM1_DutyCycle = 255;
          _STATUS_PLUG = notConnected;
          plugParams.statusFinishCharging = false;
     }
     else
     {
          plugParams.statusPlugConnect = true;
          plugParams.PWM1_DutyCycle = dutyCycleCal(MAX_AMP);
     }
}
// ###################################### Analog-range adjustment ######################################
plugState checkState(int adcMax, int adcMin)
{
     if (adcMin >= 550)
     {
          plugParams.statusFinishCharging = false;
          return diodeFailed;
     }
     else if (adcMax >= 3150 and adcMax < 3779)
     {
          if (plugParams.statusFinishCharging == true)
               return finishedCharging;
          else
               return connected;
     }
     else if (adcMax >= 2618 and adcMax < 3150)
     {
          plugParams.statusFinishCharging = true;
          return charging;
     }
}
void IMP_PlugEVSEStatusSetup()
{
     ledcAttachPin(PWM_WIRTE_PIN, PWM1_CH);
     ledcSetup(PWM1_CH, PWM1_FREQ, PWM1_RES);
}
void IMP_EnablePlugEVSEStatus()
{
     ledcWrite(PWM1_CH, plugParams.PWM1_DutyCycle);
     checkAnalog readCP;
     readCP = readVal(ANALOGREAD_PIN, MAX_SAMPLES);
     checkEVSEconnect(readCP);
     if (plugParams.statusPlugConnect)
     {
          _STATUS_PLUG = checkState(readCP.valMax, readCP.valMin);
     }
}
