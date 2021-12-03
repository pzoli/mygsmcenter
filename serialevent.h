class EvtSerialListener : public EvtListener {
  protected:
  Stream * _serial;
  public:
	EvtSerialListener(Stream *serial, EvtAction t);
	void setupListener();
	bool isEventTriggered();
};

EvtSerialListener::EvtSerialListener(Stream *serial, EvtAction t) {
  this->_serial = serial;
  this->triggerAction = t;
}

void EvtSerialListener::setupListener() {
  
}

bool EvtSerialListener::isEventTriggered() {
  return _serial->available();
}
