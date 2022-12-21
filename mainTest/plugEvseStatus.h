#define PWM_WIRTE_PIN 26
#define PWM1_CH 0
#define PWM1_RES 8
#define PWM1_FREQ 1000 // PWM frequency
#define ANALOGREAD_PIN 25
#define MAX_AMP 10 // *edit later
int vMax  = 0;
int vMin = 0;

// ###################################### convert plug-state to enum for using of switch-case ######################################
enum plugState
{
  notConnected,
  diodeFailed,
  connected,
  charging,
  finishedCharging,
};
unsigned int hashitPlugState(String inString)
{
  if (inString == "not connected")
    return notConnected;
  if (inString == "diode failed")
    return diodeFailed;
  if (inString == "connected")
    return connected;
  if (inString == "charging")
    return charging;
  if (inString == "finished charging")
    return finishedCharging;
}

struct plugParams
{
  bool statusPlugConnect = false;     // for check of plugEVSE connects to EV
  bool statusFinishCharging = false;  // for check of having charge and finish
  unsigned int PWM1_DutyCycle = 255;  // max value, direct voltage;
  String statePlug = "not connected"; // variable for all plugEVSE status
} plugParams;

unsigned int dutyCycleCal(int maxAmp)
{
  return round(((maxAmp / 0.6) / 100) * 255);
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
    plugParams.statePlug = "not connected";
    plugParams.statusFinishCharging = false;
  }
  else
  {
    plugParams.statusPlugConnect = true;
    plugParams.PWM1_DutyCycle = dutyCycleCal(MAX_AMP);
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
  ledcWrite(PWM1_CH, plugParams.PWM1_DutyCycle);
  checkAnalog readCP;
  readCP = readVal(ANALOGREAD_PIN, 400);
  checkEVSEconnect(readCP);
  vMax = readCP.valMax;
  vMin = readCP.valMin;
  if (plugParams.statusPlugConnect)
  {
    plugParams.statePlug = checkState(readCP.valMax, readCP.valMin);
  }
}
