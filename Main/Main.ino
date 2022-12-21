#include "plugEvseStatus.h"
#include "modbusCommunication.h"
#include "OCPP_constString.h"
#include "OCPP_events_Enum.h"

#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#define RANDOM_UNIQUEID_LEN 24 // unique ID of charge point events

WebSocketsClient webSocket;

#define SSID "INA 2.4G"
#define PASSWORD "0815158010"

// IMP_WriteReadModbus -> export function
// HEARTBEAT_REQUEST -> main function
// _METERVALUES_REQUEST -> sub main function
// _GetTimeStructure -> sub funciton

// ws://152.70.39.176:9000/OCPP/CP_1
#define SERVER_ADDRESS "192.168.1.50"
#define PORT 8088
#define PATH_URL "OCPP/CP-INA-00001"
#define PROTOCAL "ocpp1.6"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

StaticJsonDocument<512> messageRecieve;

// ########################## uniqueId reciever from central ##########################
struct UNIQUE_ID
{
  const char *UNIQUEID_RECIEVER = "";
  String SEND_BOOTNOTIFICATION = "";
  String SEND_START_TRANSACTION = "";
  String SEND_STOP_TRANSACTION = "";
} UNIQUE_ID;

struct CP_PARAMS
{
  // defult interval 60s
  unsigned long HEARTBEAT_INTERVAL = 60000;
  unsigned int RECIEVED_CONNECTOR_ID;
  unsigned int TRANSACTION_ID_START;
  unsigned int TRANSACTION_ID_STOP;
  String ID_TAG;

} CP_PARAMS;

struct STATE
{
  bool ALREADY_WS_CONNECT = false;
  bool ALREADY_START_TRANSACTION = false;
} STATE;

void _DeserializationJson(uint8_t *payload)
{
  DeserializationError error = deserializeJson(messageRecieve, payload);
  // Test if parsing succeeds.
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
}
String _RandomChargingPointID(int len)
{
  const String alphanum =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  String tmp_s = "";
  while (len--)
  {
    tmp_s += String(alphanum[random(alphanum.length())]);
  }
  return tmp_s;
}

void BOOT_NOTIFICATION_REQUEST()
{
  String bootNotiUniqueID = _RandomChargingPointID(RANDOM_UNIQUEID_LEN);
  StaticJsonDocument<512> BN;
  BN.add(2);
  BN.add(bootNotiUniqueID);
  BN.add("BootNotification");
  JsonObject BN_idx3 = BN.createNestedObject();
  BN_idx3["chargePointVendor"] = BootNotificationTXT.chargePointVendor;
  BN_idx3["chargePointModel"] = BootNotificationTXT.chargePointModel;
  BN_idx3["chargePointSerialNumber"] = BootNotificationTXT.chargePointSerialNumber;
  BN_idx3["chargeBoxSerialNumber"] = BootNotificationTXT.chargeBoxSerialNumber;
  BN_idx3["firmwareVersion"] = BootNotificationTXT.firmwareVersion;
  BN_idx3["iccid"] = BootNotificationTXT.iccid;
  BN_idx3["meterType"] = BootNotificationTXT.meterType;
  BN_idx3["meterSerialNumber"] = BootNotificationTXT.meterSerialNumber;
  String output;
  serializeJson(BN, output);
  webSocket.sendTXT(output);
}

void HEARTBEAT_REQUEST()
{
  String heartBeatUniqueID = _RandomChargingPointID(RANDOM_UNIQUEID_LEN);
  StaticJsonDocument<128> HB;
  HB.add(2);
  HB.add(heartBeatUniqueID);
  HB.add("Heartbeat");
  HB.createNestedObject();
  String output;
  serializeJson(HB, output);
  webSocket.sendTXT(output);
}
void REMOTE_START_TRANSACTION_RESPONSE()
{
  StaticJsonDocument<128> RST;
  RST.add(3);
  RST.add(UNIQUE_ID.UNIQUEID_RECIEVER);
  RST[2]["status"] = RemoteStartTransResponseTXT.status.Accepted; // as long as "status" is "Accepted"
  String output;
  serializeJson(RST, output);
  webSocket.sendTXT(output);
}

void START_TRANSACTION_REQUEST()
{
  // uniqueId.tokenStartTransaction = RandomChargePointUniqueId(randomUniqueIdLength);
  UNIQUE_ID.SEND_START_TRANSACTION = _RandomChargingPointID(RANDOM_UNIQUEID_LEN);
  StaticJsonDocument<256> strtT;
  strtT.add(2);
  strtT.add(UNIQUE_ID.SEND_START_TRANSACTION);
  strtT.add("StartTransaction");
  JsonObject strtT_idx3 = strtT.createNestedObject();
  strtT_idx3["connectorId"] = CP_PARAMS.RECIEVED_CONNECTOR_ID;
  strtT_idx3["idTag"] = CP_PARAMS.ID_TAG; //************************idTag
  strtT_idx3["timestamp"] = _GetTimeStructure();
  strtT_idx3["meterStart"] = 1150; // integer, Wh unit
  strtT_idx3["reservationId"] = 0;
  String output;
  serializeJson(strtT, output);
  webSocket.sendTXT(output);
  // params.statusStartTransactionRequest = true;
}

void REMOTE_STOP_TRANSACTION_RESPONSE()
{
  StaticJsonDocument<128> RSpT;
  RSpT.add(3);
  RSpT.add(UNIQUE_ID.UNIQUEID_RECIEVER);
  RSpT[2]["status"] = RemoteStopTransResponseTXT.status.Accepted;
  // params.statusRemoteStopTransactionResponse;
  String output;
  serializeJson(RSpT, output);
  webSocket.sendTXT(output);
}

void STOP_TRANSACTION_REQUEST()
{
  // uniqueId.tokenStopTransaction = RandomChargePointUniqueId(randomUniqueIdLength);
  UNIQUE_ID.SEND_STOP_TRANSACTION = _RandomChargingPointID(RANDOM_UNIQUEID_LEN);
  StaticJsonDocument<256> STp;
  STp.add(2);
  STp.add(UNIQUE_ID.SEND_STOP_TRANSACTION);
  STp.add("StopTransaction");
  JsonObject STp_idx3 = STp.createNestedObject();
  STp_idx3["transactionId"] = CP_PARAMS.TRANSACTION_ID_STOP;
  STp_idx3["idTag"] = CP_PARAMS.ID_TAG; //************************idTag
  STp_idx3["timestamp"] = _GetTimeStructure();
  STp_idx3["meterStop"] = 1150;
  String output;
  serializeJson(STp, output);
  webSocket.sendTXT(output);
}

void _METERVALUES_REQUEST()
{
  rModbus = IMP_WriteReadModbus(energy, READ_LENGTH);
  float meterValue;
  if (rModbus.errorMessage == "No error")
  {
    // statement
    meterValue = rModbus.floatValue;
  }
  StaticJsonDocument<512> MV;
  MV.add(2);
  MV.add(UNIQUE_ID.UNIQUEID_RECIEVER);
  MV.add("MeterValues");
  JsonObject MV_idx3 = MV.createNestedObject();
  MV_idx3["connectorId"] = CP_PARAMS.RECIEVED_CONNECTOR_ID;
  MV_idx3["transactionId"] = CP_PARAMS.TRANSACTION_ID_START; // transactionId ที่กำลังใช้อยู่ในตอนนี้
  JsonObject MV_idx3_SmeterValue = MV_idx3["meterValue"].createNestedObject();
  MV_idx3_SmeterValue["timestamp"] = _GetTimeStructure(); // update time immediately
  JsonArray MV_idx3_SmeterValue_idx0 = MV_idx3_SmeterValue.createNestedArray("sampledValue");
  JsonObject MV_idx3_SmeterValue_idx0_SsampleValue_idx0 = MV_idx3_SmeterValue_idx0.createNestedObject();
  MV_idx3_SmeterValue_idx0_SsampleValue_idx0["value"] = meterValue;
  MV_idx3_SmeterValue_idx0_SsampleValue_idx0["measurand"] = MeterValuesRequestTXT.measurand.EnerActExpReg; //"Energy.Active.Import.Register"

  String output;
  serializeJson(MV, output);
  webSocket.sendTXT(output);
}

enum ENM_STATE_NOTI
{
  Available,
  Preparing,
  Charging,
  SuspendedEV,
  SuspendedEVSE,
  Finishing,
  Reserved,
  Unavailable,
  Faulted,
};
enum ENM_ERROR_NOTI
{
  ConnectorLockFailure,
  EVCommunicationError,
  GroundFailure,
  HighTemperature,
  InternalError,
  LocalListConflict,
  NoError,
  OtherError,
  OverCurrentFailure,
  PowerMeterFailure,
  PowerSwitchFailure,
  ReaderFailure,
  ResetFailure,
  UnderVoltage,
  OverVoltage,
  WeakSignal,
};
struct STU_STATE_NOTI
{
  ENM_STATE_NOTI CP_defultStatus;
  ENM_STATE_NOTI CP_currentStatus;
  String CP_Status;
  String CP_Errocode;
} STU_STATE_NOTI;

void _HashitStatusNoti_(ENM_STATE_NOTI stateNoti)
{
  switch (stateNoti)
  {
  case Available:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Available;
    break;
  case Preparing:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Preparing;
    break;
  case Charging:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Charging;
    break;
  case SuspendedEV:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.SuspendedEV;
    break;
  case SuspendedEVSE:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.SuspendedEVSE;
    break;
  case Finishing:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Finishing;
    break;
  case Reserved:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Reserved;
    break;
  case Unavailable:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Unavailable;
    break;
  case Faulted:
    STU_STATE_NOTI.CP_Status = StatusNotiRequestTXT.status.Faulted;
    break;
  default:
    break;
  }
}

class CP_STATE_CHECKING
{
  // ########################## Check status part ##########################
  /*StartTransaction? : N, EV connected? : N
    StartTransaction? : Y, EV connected? : N
    StartTransaction? : Y, EV connected? : Y*/
public:
  void CHECK_AVAILABLE()
  {
    if (STATE.ALREADY_START_TRANSACTION == false and EV_CONNECT_STATE == notConnected)
    {
      STU_STATE_NOTI.CP_currentStatus = Available; // *edit later
    }
  }

  void CHECK_PREPARING()
  {
    if (STATE.ALREADY_START_TRANSACTION == true and EV_CONNECT_STATE == notConnected)
    {
      STU_STATE_NOTI.CP_currentStatus = Preparing;
    }
  }
  void CHECK_CHARGING()
  {
    if (STATE.ALREADY_START_TRANSACTION == true and EV_CONNECT_STATE == connected and _STATUS_PLUG == charging)
    {
      STU_STATE_NOTI.CP_currentStatus = Charging;
    }
  }
  void CHECK_SUSPENDED_EV() {}
  void CHECK_SUSPENDED_EVSE() {}
  void CHECK_FINISHING() {}
  void CHECK_RESERVED() {}
  void CHECK_UNAVAILABLE() {}
  void CHECK_FAULTED() {}
} CP_STATE;

class CP_ERROR_CHECKING
{
public:
  void CHECK_POWER_METER_FAILURE()
  {
    rModbus = IMP_WriteReadModbus(energy, READ_LENGTH);
    if (rModbus.errorMessage == "read fail")
    {
      STU_STATE_NOTI.CP_Errocode = StatusNotiRequestTXT.errorCode.PowerMeterFailure;
    }
  }
  void CHECK_EV_COMMUNICATION_ERROR()
  {
    if (_STATUS_PLUG == diodeFailed)
    {
      STU_STATE_NOTI.CP_Errocode = StatusNotiRequestTXT.errorCode.EVCommunicationError;
    }
  }
} CP_ERROR;
void _StatusNotification_check_()
{
  // ########################## Check error part ##########################
  CP_ERROR.CHECK_EV_COMMUNICATION_ERROR();
  CP_ERROR.CHECK_POWER_METER_FAILURE();
  // ########################## Check status part ##########################
  CP_STATE.CHECK_AVAILABLE();
  CP_STATE.CHECK_PREPARING();
  CP_STATE.CHECK_CHARGING();
  CP_STATE.CHECK_UNAVAILABLE();
  CP_STATE.CHECK_FAULTED();
}
// void _SendNotificationStatus_(ENM_STATE_NOTI *defaultState, ENM_STATE_NOTI *currentState)
// {
//   if (*defaultState != *currentState)
//   {
//     *defaultState = *currentState;
//     switch (*defaultState)
//     {
//     case Available:
//     {
//       switch (*currentState)
//       {
//       case Preparing:
//       case Charging:
//       case SuspendedEV:
//       case SuspendedEVSE:
//       case Reserved:
//       case Unavailable:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//     }
//     break;
//     case Preparing:
//     {
//       switch (*currentState)
//       {
//       case Available:
//       case Charging:
//       case SuspendedEV:
//       case SuspendedEVSE:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//     }
//     break;
//     case Charging:
//     {
//       switch (*currentState)
//       {
//       case Available:
//       case SuspendedEV:
//       case SuspendedEVSE:
//       case Finishing:
//       case Unavailable:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//     }
//     break;
//     case SuspendedEV:
//     {
//       switch (*currentState)
//       {
//       case Available:
//       case Charging:
//       case SuspendedEVSE:
//       case Finishing:
//       case Unavailable:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//     }
//     break;
//     case SuspendedEVSE:
//       switch (*currentState)
//       {
//       case Available:
//       case Charging:
//       case SuspendedEV:
//       case Finishing:
//       case Unavailable:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//       break;
//     case Finishing:
//       switch (*currentState)
//       {
//       case Available:
//       case Preparing:
//       case Unavailable:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//       break;
//     case Reserved:
//       switch (*currentState)
//       {
//       case Available:
//       case Preparing:
//       case Unavailable:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//       break;
//     case Unavailable:
//       switch (*currentState)
//       {
//       case Available:
//       case Preparing:
//       case Charging:
//       case SuspendedEV:
//       case SuspendedEVSE:
//       case Faulted:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//       break;
//     case Faulted:
//       switch (*currentState)
//       {
//       case Available:
//       case Preparing:
//       case Charging:
//       case SuspendedEV:
//       case SuspendedEVSE:
//       case Finishing:
//       case Reserved:
//       case Unavailable:
//         _STATUSNOTIFICATION_REQUEST();
//         break;
//       default:
//         break;
//       }
//       break;
//     default:
//       break;
//     }
//   }
// }

void _SendNotificationStatus_(ENM_STATE_NOTI *defaultState, ENM_STATE_NOTI *currentState){
  if (*defaultState != *currentState){
    *defaultState == *currentState;
    _STATUSNOTIFICATION_REQUEST();
  }
}
void _AutoSendNotificationStatus_()
{
  _StatusNotification_check_();
  _HashitStatusNoti_(STU_STATE_NOTI.CP_currentStatus);
  _SendNotificationStatus_(&STU_STATE_NOTI.CP_defultStatus, &STU_STATE_NOTI.CP_currentStatus);
}
void _STATUSNOTIFICATION_REQUEST()
{
  StaticJsonDocument<512> stNft;
  stNft.add(2);
  stNft.add(UNIQUE_ID.UNIQUEID_RECIEVER);
  stNft.add("StatusNotification");
  JsonObject stNft_idx3 = stNft.createNestedObject();
  stNft_idx3["connectorId"] = CP_PARAMS.RECIEVED_CONNECTOR_ID;
  stNft_idx3["status"] = STU_STATE_NOTI.CP_Status;
  stNft_idx3["Errocode"] = STU_STATE_NOTI.CP_Errocode;
  stNft_idx3["info"] = "";
  stNft_idx3["timestamp"] = _GetTimeStructure();
  stNft_idx3["vendorId"] = "";
  stNft_idx3["vendorErrocode"] = "";
  String output;
  serializeJson(stNft, output);
  webSocket.sendTXT(output);
  delay(50);
}

void TRIGGERMESSAGE_RESPONSE()
{
  StaticJsonDocument<128> TM;
  TM.add(3);
  TM.add(UNIQUE_ID.UNIQUEID_RECIEVER);
  TM[2]["status"] = TriggerMessageResponseTXT.status.Accepted;
  String output;
  serializeJson(TM, output);
  Serial.println();
  webSocket.sendTXT(output);
}

String _GetTimeStructure()
{
  while (!timeClient.update())
    timeClient.forceUpdate();
  return timeClient.getFormattedDate();
}

void _SetUpTimeClient_()
{
  timeClient.begin();
  timeClient.setTimeOffset(25200); // GTM can be changed
}
void _Hexdump(const void *mem, uint32_t len, uint8_t cols = 16)

{
  const uint8_t *src = (const uint8_t *)mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++)
  {
    if (i % cols == 0)
    {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

// ########################## convert recievedUniqueId to enum for using of switch-case ##########################
enum DuringOperationState
{
  SEND_BOOTNOTIFICATION,
  SEND_START_TRANSACTION,
  SEND_STOP_TRANSACTION,
};
unsigned int _HashitDuringOperation(String recievedUniqueId)
{
  if (recievedUniqueId == UNIQUE_ID.SEND_BOOTNOTIFICATION)
    return SEND_BOOTNOTIFICATION;
  if (recievedUniqueId == UNIQUE_ID.SEND_START_TRANSACTION)
    return SEND_START_TRANSACTION;
  if (recievedUniqueId == UNIQUE_ID.SEND_STOP_TRANSACTION)
    return SEND_STOP_TRANSACTION;
}

void _AwatingMatchingEvent_()
{
  String recievedUniqueId = messageRecieve[1];
  // - Boot notification recieve event
  switch (_HashitDuringOperation(recievedUniqueId))
  {
  case SEND_BOOTNOTIFICATION:
  {
    CP_PARAMS.HEARTBEAT_INTERVAL = messageRecieve[2]["interval"];
    CP_PARAMS.HEARTBEAT_INTERVAL = CP_PARAMS.HEARTBEAT_INTERVAL * 1000; // convert to ms
  }
  break;
  case SEND_START_TRANSACTION:
  {
    String checkStatus = messageRecieve[2]["idTagInfo"]["status"];
    if (checkStatus == "Accepted") // *** edit later ***
    {
      CP_PARAMS.TRANSACTION_ID_START = messageRecieve[2]["transactionId"];
      // *enable EVSE to indentify status
      STATE.ALREADY_START_TRANSACTION = true;
    }
  }
  break;
  case SEND_STOP_TRANSACTION:
  {
    String checkStatus = messageRecieve[2]["idTagInfo"]["status"];
    // *disible EVSE to indentify status
    STATE.ALREADY_START_TRANSACTION = false;
  }
  break;
  }
}

void _AwaingEvent_()
{
  int messageTypeId = messageRecieve[0];
  if (messageTypeId == 2) // checck if central calls events
  {
    const char *events = messageRecieve[2];
    UNIQUE_ID.UNIQUEID_RECIEVER = messageRecieve[1];
    switch (IMP_HashitEventOp(events))
    {
    case RemoteStartTransaction:
    {
      CP_PARAMS.RECIEVED_CONNECTOR_ID = messageRecieve[3]["connectorId"]; // get connectorID
      REMOTE_START_TRANSACTION_RESPONSE();
      START_TRANSACTION_REQUEST();
    }
    break;
    case RemoteStopTransaction:
    {
      CP_PARAMS.TRANSACTION_ID_STOP = messageRecieve[3]["transactionId"]; // get transactionID
      REMOTE_STOP_TRANSACTION_RESPONSE();
    }
    case TriggerMessage:
    {
      const char *requestedMessage = messageRecieve[3]["requestedMessage"];
      CP_PARAMS.RECIEVED_CONNECTOR_ID = messageRecieve[3]["connectorId"]; // may edit later
      TRIGGERMESSAGE_RESPONSE();
      switch (IMP_HashitTrigMesEvt(requestedMessage)) // case inside TriggerMessage
      {
      case MeterValues:
        _METERVALUES_REQUEST();
        break;
      case StatusNotification:
        _STATUSNOTIFICATION_REQUEST();
        break;
      }
    }
    }
  }
}

void _WebSocketEvent_(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    STATE.ALREADY_WS_CONNECT = false;
    break;
  case WStype_CONNECTED:
    Serial.printf("[WSc] Connected to url: %s\n", payload);
    BOOT_NOTIFICATION_REQUEST();
    STATE.ALREADY_WS_CONNECT = true;
    // send message to server when Connected
    break;
  case WStype_TEXT:
    Serial.printf("[WSc] get text: %s\n", payload);
    _DeserializationJson(payload);
    _AwatingMatchingEvent_();
    _AwaingEvent_();
    // send message to server
    // webSocket.sendTXT("message here");
    break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    _Hexdump(payload, length);
    // send data to server
    // webSocket.sendBIN(payload, length);
    break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}

void _SetupWifi_()
{
  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void _SetupWebSocket_()
{
  // webSocket.begin("192.168.0.123", 81, "/");
  //  webSocket.begin(SERVER_ADDRESS, PORT, PATH_URL, PROTOCAL);
  webSocket.begin(SERVER_ADDRESS, PORT, PATH_URL, PROTOCAL);
  // event handler
  webSocket.onEvent(_WebSocketEvent_);
}
plugState previousPlugState = notConnected;
void _PlugStateRelayControl_(plugState status)
{
  if (status != previousPlugState)
  {
    previousPlugState = status;
    switch (status)
    {
    case notConnected:
      IMP_WriteModbus(relayOn, WRITE_LENGTH);
      EV_CONNECT_STATE = notConnected;
      break;
    case diodeFailed:
      IMP_WriteModbus(relayOn, WRITE_LENGTH);
      EV_CONNECT_STATE = notConnected;
      break;
    case connected:
      EV_CONNECT_STATE = connected;
      break;
    case charging:
      IMP_WriteModbus(relayOff, WRITE_LENGTH);
      EV_CONNECT_STATE = connected;
      break;
    case finishedCharging:
      IMP_WriteModbus(relayOn, WRITE_LENGTH);
      EV_CONNECT_STATE = notConnected;
      break;
    }
  }
}
void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N2, RX1, TX1);
  IMP_EnablePlugEVSEStatus();
  _SetupWifi_();
  _SetUpTimeClient_();
  _SetupWebSocket_();
}

unsigned long lastUpdate = millis();
void loop()
{
  webSocket.loop();
  // send Heartbeat
  _AutoSendNotificationStatus_();
  if (STATE.ALREADY_WS_CONNECT and lastUpdate + CP_PARAMS.HEARTBEAT_INTERVAL < millis())
  {
    HEARTBEAT_REQUEST();
    lastUpdate = millis();
  }
  // *enable EVSE to indentify status
  if (STATE.ALREADY_START_TRANSACTION)
  {
    IMP_PlugEVSEStatusSetup();
    _PlugStateRelayControl_(_STATUS_PLUG);
  }
}
