#include <Eventually.h>
#include "serialevent.h"

char incoming_char=0;
char request_char=0;
String arrived_line = "";
String request_line = "";
bool isIncomingSMS = false;
String phoneNumber = "";
EvtManager mgr;

bool serialAction() {
  incoming_char=Serial1.read();
  arrived_line.concat(incoming_char);
  //Serial.print(incoming_char);
  if (incoming_char == '\n') {
    if (isIncomingSMS) {
      Serial.print("{\"action\":\"sms\",\"from\":\"");
      Serial.print(phoneNumber);
      Serial.print("\",\"message\":\"");
      Serial.print(arrived_line.substring(0,arrived_line.length()-2));
      Serial.println("\"}");
      isIncomingSMS = false;  
    } else if (arrived_line.indexOf("RING") > -1) {
        // If the message received from the shield is RING
        // Send ATA commands to answer the phone
        // Serial1.print("ATA\r");
    } else if (arrived_line.indexOf("+CLIP:") > -1) {
      //+CLIP: "+36301234567",145,"",,"",0
      int start = arrived_line.indexOf("\"");
      int end = arrived_line.indexOf("\"",start+1);
      phoneNumber = arrived_line.substring(start+1,end);
      if (phoneNumber.length() > 0) {
        Serial.print(F("{\"action\":\"ring\",\"from\":\""));
        Serial.print(phoneNumber);
        Serial.println("\"}");
      }
    } else if (arrived_line.indexOf("+CMT:") > -1) {
      //+CMT: "+36301234567","","21/11/23,17:07:41+04"
      int start = arrived_line.indexOf("\"");
      int end = arrived_line.indexOf("\"",start+1);
      phoneNumber = arrived_line.substring(start+1,end);
      if (phoneNumber.length() > 0) {
        isIncomingSMS = true;
      }
    } else if (arrived_line.indexOf("OK") > -1) {
      Serial.println(F("{\"action\":\"result\",\"status\":\"OK\"}"));
    } else if (arrived_line.indexOf("ERROR") > -1) {
      Serial.println(F("{\"action\":\"result\",\"status\":\"ERROR\"}"));
    } else if (arrived_line.indexOf("NO CARRIER") > -1) {
      Serial.println(F("{\"action\":\"call-status\",\"status\":\"NO CARRIER\"}"));
    } else if (arrived_line.indexOf("BUSY") > -1) {
      Serial.println(F("{\"action\":\"call-status\",\"status\":\"BUSY\"}"));
    } else if (arrived_line.indexOf("+CUSD: 0,") > -1) {
      int start = arrived_line.indexOf("\"");
      int end = arrived_line.indexOf("\"", start + 1);
      Serial.print(F("{\"action\":\"balance\",\"message\":\""));
      Serial.print(arrived_line.substring(start + 1, end));
      Serial.println("\"}");
    }
    arrived_line = "";
  }
  return true;
}

bool requestAction() {
  request_char = Serial.read();
  if (request_char == '\n') {
    //Serial.print(request_line);
    if (request_line.indexOf(F("SMS:")) > -1) {
      String targetPhone = getValue(request_line,':',1);
      String message = getValue(request_line,':',2);
      sendSMS(targetPhone,message);
    } else if (request_line.indexOf(F("CALL:")) > -1) {
      call(request_line.substring(5));
    } else if (request_line.indexOf(F("HANGUP")) > -1) {
      hangUp();
    } else if (request_line.indexOf(F("ANSWER")) > -1) {
      answer();
    } else if (request_line.indexOf(F("BALANCE")) > -1) {
      balance();
    } else if (request_line.indexOf(F("STATUS")) > -1) {
      getStatus();
    } else if (request_line.indexOf(F("POWER")) > -1) {
      powerSwithc();
    } else if (request_line.indexOf(F("INITMODEM")) > -1) {
      initModem();
    }
    request_line = "";
  } else {
    request_line.concat(request_char);
  }
  return true;
}

void balance() {
  Serial1.println(F("AT+CUSD=1,\"*102#\"\r"));
}

void sendSMS(String targetPhoneNumber, String msg) {
  //Serial.println();
  //Serial.println("*** target:[" + targetPhoneNumber + "]");
  //Serial.println("*** message:[" + msg +"]");

  Serial1.print(F("AT + CMGS = \""));
  Serial1.print(targetPhoneNumber);
  Serial1.println("\""); 
  delay(100);
  Serial1.println(msg);
  delay(100);
  Serial1.println((char)26);
  delay(100);
  Serial1.println();
}
void call(String targetPhoneNumber) {
  //Serial.println();
  //Serial.println("*** phone:[" + targetPhoneNumber + "]");
  Serial1.println("ATD + " + targetPhoneNumber + ";");
  delay(100);
  Serial1.println();
}
void hangUp() {
  Serial1.println("ATH");
}
void answer() {
  Serial1.print("ATA\r");
}

String getStatus() {
  // https://m2msupport.net/m2msupport/atcpas-phone-activity-status/
  Serial1.println("AT+CPAS");
  delay(1000);
  String result = "";
  while(Serial1.available()) {
    char c = Serial1.read();
    result.concat(c);
  }
  //Serial.print("status.length=");
  //Serial.println(result.length());
  if (result.length()>0) {    
    int startIdx = result.indexOf("+CPAS:");
    //Serial.print("gsm[");  
    //Serial.print(startIdx);
    //Serial.print(","); 
    if (startIdx > -1) {
      int endIdx = result.indexOf("\r\n",startIdx);
      //Serial.print(endIdx);
      //Serial.println("]");
      if (endIdx > -1) {
        Serial.print(F("{\"action\":\"status\",\""));
        Serial.print(result.substring(startIdx,endIdx));    
        Serial.println("\"}");
      }
    }
  } else {
        Serial.println(F("{\"action\":\"status\",\"power off\"}"));
  }
  return result;
}

void powerSwithc() {
    // Power On GSM shield
    // https://arduino.stackexchange.com/questions/28331/solved-soft-power-on-sim900-module-jp-pads-missing
    pinMode(9, OUTPUT); 
    digitalWrite(9,LOW);
    delay(1000);
    digitalWrite(9,HIGH);
    delay(2000);
    pinMode(9, INPUT);  
}

void initModem() {
    Serial1.print(F("AT+CLIP=1\r")); // turn on caller ID notification
    delay(100);
    // AT command to set SIM900 to SMS mode
    Serial1.print(F("AT+CMGF=1\r")); 
    delay(100);
    // Set module to send SMS data to serial out upon receipt 
    Serial1.print(F("AT+CNMI=2,2,0,0,0\r"));
    delay(100);
}

String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup() {
  Serial.begin(19200); 
  while (!Serial);
  Serial.println(F("{\"action\":\"setup\",\"message\":\"Start setup...\"}"));
  // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  Serial1.begin(19200); // for GSM shield

  String gsmStatus = getStatus();
  if (gsmStatus.length() == 0) {  
    powerSwithc();
    // For serial monitor
    // Give time to log on to network.
    delay(10000); 
    initModem();
  }
    
  Serial.println(F("{\"action\":\"setup\",\"message\":\"Ready to receive calls...\"}"));
  
  mgr.addListener(new EvtSerialListener(&Serial1, (EvtAction) serialAction));
  mgr.addListener(new EvtSerialListener(&Serial, (EvtAction) requestAction));

  Serial.println(F("{\"action\":\"setup\",\"message\":\"Listeners started.\"}"));
}

USE_EVENTUALLY_LOOP(mgr)
