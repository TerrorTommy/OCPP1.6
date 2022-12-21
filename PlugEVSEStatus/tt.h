#define PWM_WIRTE_PIN 26 // 25 DEFULT
#define PWM1_CH 0
#define PWM1_RES 8
#define PWM1_FREQ 1000
#define ANALOGREAD_PIN 25 // 26 DEFULT
#define MAX_AMP 18        // *edit later
#define FULL_DUTYCYCLE 255
#define NO_SAMPLES 400
unsigned long previousMillis = 0; // *delete later
const int period = 100;
int dutyCycle = FULL_DUTYCYCLE;
enum sysState
{
     NOT_CONNECTED,
     PLUG_CONNECTED,
     CONNECTED,
     DIODE_FAILED,
     CHARGING,
     IDLE,
     ALREADY_CHARGED,
     FINISH_CHARGING,
};
sysState STATE = NOT_CONNECTED;
sysState STATE_PLUG = NOT_CONNECTED;
sysState STATE_CHARGING = IDLE;

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

sysState checkState(int adcMax, int adcMin)
{
     if (adcMax >= 3779 and adcMax < 4096)
     {
          STATE = NOT_CONNECTED;
          STATE_PLUG = NOT_CONNECTED;
          dutyCycle = FULL_DUTYCYCLE;
     }
     else
     {
          STATE_PLUG = PLUG_CONNECTED;
          dutyCycle = dutyCycleCal(MAX_AMP);
     }
     if (STATE_PLUG == PLUG_CONNECTED)
     {
          if (adcMin >= 550)
               STATE = DIODE_FAILED;
          else if (adcMax >= 3150 and adcMax < 3779)
          {
               if (STATE_CHARGING = IDLE)
               {
                    STATE = CONNECTED;
               }
               else if (STATE_CHARGING = ALREADY_CHARGED)
               {
                    STATE = FINISH_CHARGING;
                    STATE_CHARGING = IDLE;
               }
          }
          else if (adcMax >= 2618 and adcMax < 3150)
          {

               STATE = CHARGING;
               STATE_CHARGING = ALREADY_CHARGED;
          }
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
     ledcWrite(PWM1_CH, dutyCycle);
     checkAnalog readCP;
     readCP = readVal(ANALOGREAD_PIN, NO_SAMPLES);
     if (currentMillis - previousMillis >= period)
     {
          checkState(readCP.valMax, readCP.valMin);
          previousMillis = currentMillis;
          // Serial.println((String) "max : " + readCP.valMax + " min : " + readCP.valMin + " state : " + plugParams.statePlug);
          // Serial.println((String) "PWM : " + plugParams.PWM1_DutyCycle);
     }
}