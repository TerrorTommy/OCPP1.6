enum EventOperation
{
  RemoteStopTransaction,
  RemoteStartTransaction,
  TriggerMessage,
};
enum TriggerMessageEvent
{
  MeterValues,
  StatusNotification
};

int IMP_HashitEventOp(String inString)
{
  if (inString == "RemoteStopTransaction")
    return RemoteStopTransaction;
  if (inString == "RemoteStartTransaction")
    return RemoteStartTransaction;
  if (inString == "TriggerMessage")
    return TriggerMessage;
}
int IMP_HashitTrigMesEvt(String inString)
{
  if (inString == "MeterValues")
    return MeterValues;
  if (inString == "StatusNotification")
    return StatusNotification;
}
