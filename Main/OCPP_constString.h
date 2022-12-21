struct BootNotificationTXT
{
  String chargePointVendor = "AVT-Company";
  String chargePointModel = "AVT-Express";
  String chargePointSerialNumber = "avt.001.13.1";
  String chargeBoxSerialNumber = "avt.001.13.1.01";
  String firmwareVersion = "0.9.87";
  String iccid = "imsi";
  String imsi = "";
  String meterType = "AVT NQC-ACD";
  String meterSerialNumber = "avt.001.13.1.01";
} BootNotificationTXT;

struct RemoteStartTransResponseTXT
{
  struct status
  {
    String Accepted = "Accepted";
    String Rejected = "Rejected";
  } status;
} RemoteStartTransResponseTXT;

struct RemoteStopTransResponseTXT
{
  struct status
  {
    String Accepted = "Accepted";
    String Rejected = "Rejected";
  } status;
} RemoteStopTransResponseTXT;

struct TriggerMessageResponseTXT
{
  struct status
  {
    String Accepted = "Accepted";
    String Rejected = "Rejected";
    String NotImplemented = "NotImplemented";
  } status;
} TriggerMessageResponseTXT;

struct MeterValuesRequestTXT
{
    struct measurand
  {
    String EnerActExpReg = "Energy.Active.Export.Register";
    String EnerActImpReg = "Energy.Active.Import.Register";
    String EnerReacExpReg = "Energy.Reactive.Export.Register";
    String EnerReacImpReg = "Energy.Reactive.Import.Register";
    String EnerActExpInt = "Energy.Active.Export.Interval";
    String EnerActImpInt = "Energy.Active.Import.Interval";
    String EnerReacExpInt = "Energy.Reactive.Export.Interval";
    String EnerReacImpInt = "Energy.Reactive.Import.Interval";
    }measurand;
}MeterValuesRequestTXT;

struct StatusNotiRequestTXT
{
  struct errorCode
  {
    String ConnectorLockFailure = "ConnectorLockFailure";
    String EVCommunicationError = "EVCommunicationError";
    String GroundFailure = "GroundFailure";
    String HighTemperature = "HighTemperature";
    String InternalError = "InternalError";
    String LocalListConflict = "LocalListConflict";
    String NoError = "NoError";
    String OtherError = "OtherError";
    String OverCurrentFailure = "OverCurrentFailure";
    String PowerMeterFailure = "PowerMeterFailure";
    String PowerSwitchFailure = "PowerSwitchFailure";
    String ReaderFailure = "ReaderFailure";
    String ResetFailure = "ResetFailure";
    String UnderVoltage = "UnderVoltage";
    String OverVoltage = "OverVoltage";
    String WeakSignal = "WeakSignal";
  } errorCode;
  struct status
  {
    String Available = "Available";
    String Preparing = "Preparing";
    String Charging = "Charging";
    String SuspendedEVSE = "SuspendedEVSE";
    String SuspendedEV = "SuspendedEV";
    String Finishing = "Finishing";
    String Reserved = "Reserved";
    String Unavailable = "Unavailable";
    String Faulted = "Faulted";
  } status;
} StatusNotiRequestTXT;