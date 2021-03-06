

//#define AllowAcessPoint         // opens a acessPoint if saved Wlan is not reachable
//#define WiFiNotRequired         // if there is no connection -> return from setup_server() (and run main loop) or restart esp until connection
//#define showKeysInWeb true      // shows keys in WEB page (for debug! not recommendet!!)

//#define usersubscribe_existing  //if a UserSubscribe() exists. Else the Server will connect automatically to set Topic
//#define MQTTBrokerChanged_existing  // if a MQTTBrokerChanged() exists you can do sth if Broker is changed via web

#define DeviceName "RolloRelais"
#define userpage_existing

         
#include "C:\Users\sebas\Documents\ESP8266-Server\ESPServer.h"






#define lockpairedOutputs


//paired outputs
// out1, out2
// out3, out5
// out4, out6
//see https://github.com/boaschti/MQTT_Mini_WLan_Relais/blob/master/Board/RolloRelais/pinning%20SSR.JPG
// set Jumper1 and R28

#define out1 12
#define out2 14
#define out3 16
#define outPower 15


#define in1  13
#define in2  2
#define shortpress        30UL
#define middlelongpress   250UL
#define longpress         2000UL
//timeout to reset ports
#define timeout           100000UL //1min40s
  



    static const char PROGMEM USERPAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name = 'viewport' content = 'width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0'>
  <title>RFM69 Node Configuration</title>
  <style>
    'body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }'
  </style>
</head>
<body>

    <p><a href="/"><button type="button">Home</button></a>
</body>
</html>
)rawliteral";


void handleconfigureUserWrite(){
    handleRoot();
}
void handleconfigureUser(){
    webServersend_P(USERPAGE_HTML);
}


bool runTimer = false;
unsigned long timstampTimeout;
const char* open_cmd = "open";
const char* close_cmd = "close";
const char* stop_cmd = "stop";

void setOutputs(uint8_t switchFunktion, uint8_t switchNr, bool resetOutputs);

void callback(char* topic, byte* payload, unsigned int length) {
    char receivedpayload[50];

    for (int i=0;i<length;i++) {
        receivedpayload[i] = (char)payload[i];
    }

    mqttpublish("Info", "got Message");
    
    if (strncmp(receivedpayload, open_cmd, 4) == 0){
    //if (receivedpayload ==  "open"){
        mqttpublish("Info", "got Message open");
        runTimer = true;
        timstampTimeout = millis();
        setOutputs(2, 1, false);            
    }
    if (strncmp(receivedpayload, close_cmd, 4) == 0){
        mqttpublish("Info", "got Message close");
        runTimer = true;
        timstampTimeout = millis();
        setOutputs(2, 2, false);            
    }
    if (strncmp(receivedpayload, stop_cmd, 4) == 0){
        mqttpublish("Info", "got Message stop");
        runTimer = true;
        timstampTimeout = millis();
        setOutputs(0,0,true);          
    }
  

}


  
void setup() {

    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(outPower, LOW);    
    
    pinMode(out1, OUTPUT);
    pinMode(out2, OUTPUT);
    pinMode(out3, OUTPUT);
    pinMode(outPower, OUTPUT);


    setup_server();
    
    pinMode(in1, INPUT);
    pinMode(in2, INPUT);
    
    Serial.println("Setup fertig run loop()");
}


uint8_t getPressFunktion(unsigned long presstime){
    
    if (presstime < shortpress){
        return 0;
    }      
    if ((presstime >= shortpress) && (presstime <= middlelongpress)){
        return 1;
    }
    if ((presstime > middlelongpress) && (presstime <= longpress)){
        return 2;
    }
    if (presstime > longpress){
        return 3;
    }
    
}

uint8_t getPairedOutput(uint8_t output){
    
    #ifdef lockpairedOutputs
    if(output == out1){
        return outPower;
    }
    if(output == out2){
        return outPower;
    }
    if(output == out3){
        return outPower;
    }
    #else
    return output;
    #endif
}

bool setAndCheckOutput(uint8_t output){
    
    Serial.print("check output: ");
    Serial.println(getPairedOutput(output));
    
    if(digitalRead(getPairedOutput(output)) == LOW){
        digitalWrite(output, HIGH);
        Serial.println("output switched");
        return true;
    }else{
        Serial.println("output not switched");
        return false;
    } 
}

void resetAllOutputs(){
    Serial.println("alles aus");
    digitalWrite(outPower, LOW);
    delay(200);
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    mqttpublishJSON("out1","OFF");
    mqttpublishJSON("out2","OFF");
    mqttpublishJSON("out3","OFF");
    mqttpublishJSON("outPower","OFF");
  
}


void setOutputs(uint8_t switchFunktion, uint8_t switchNr, bool resetOutputs = false){
    
    // switchFunktion == 0 not pressed
    // allOff == true
    // switchFunktion == 2 && switchNr == 1 && shortpressCounter == 0 out1, out3, out5 on
    // switchFunktion == 2 && switchNr == 2 && shortpressCounter == 0 out2, out4, out6 on
    // switchFunktion == 2 && switchNr == 1 && shortpressCounter == 0 out2 or out4 or out6 on
    // switchFunktion == 2 && switchNr == 2 && shortpressCounter == 0 out2 or out4 or out6 on
    

    // allOff == false
    // switchFunktion >= 1 out1, out2, out3, out4, out5, out6 off
    // switchFunktion == 1 incr shortpressCounter
    
    
    
    // switchFunktion == 3 reset shortpressCounter

    Serial.print("switch funktion: ");
    Serial.println(switchFunktion);    
    Serial.print("switch number: ");
    Serial.println(switchNr);    

    static uint8_t shortpressCounter = 0;
    static bool allOff = true;

    if(resetOutputs){
        shortpressCounter = 0;
        resetAllOutputs();
        allOff = true;
        return;
    }
    
    
    if(switchFunktion == 3){
        shortpressCounter = 0;
        Serial.println("reset counter");
    }
    
    if(allOff){
        if(switchFunktion == 1){
            shortpressCounter++;
            Serial.print("incr counter: ");
            Serial.println(shortpressCounter);
        }    
        if(switchFunktion == 2){
            if(switchNr == 1){
                switch(shortpressCounter){
                  case 0:
                      Serial.println("4 ein");
                      setAndCheckOutput(outPower);
                      mqttpublishJSON("outPower","ON");
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 1:
                      Serial.println("2 3 4 ein");
                      mqttpublishJSON("out2","ON");
                      mqttpublishJSON("out3","ON");
                      mqttpublishJSON("outPower","ON");                      
                      setAndCheckOutput(out2);
                      setAndCheckOutput(out3);
                      delay(100);
                      setAndCheckOutput(outPower);
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 2:
                      Serial.println("1 3 4 ein");
                      mqttpublishJSON("out1","ON");
                      mqttpublishJSON("out3","ON");
                      mqttpublishJSON("outPower","ON");
                      setAndCheckOutput(out1);
                      setAndCheckOutput(out3);
                      delay(100);
                      setAndCheckOutput(outPower);
                      shortpressCounter = 0;
                      allOff = false; 
                      break;
                  case 3:
                      Serial.println("1 2 4 ein");
                      mqttpublishJSON("out1","ON");
                      mqttpublishJSON("out2","ON");
                      mqttpublishJSON("outPower","ON");                      
                      setAndCheckOutput(out1);
                      setAndCheckOutput(out2);
                      delay(100);
                      setAndCheckOutput(outPower);
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  default:
                      resetAllOutputs();
                      shortpressCounter = 0;
                      Serial.println("reset counter");
                      allOff = true;
                      break;
                }
            }
            if(switchNr == 2){
                switch(shortpressCounter){
                  case 0:
                      Serial.println("1 2 3 4 ein");
                      setAndCheckOutput(out1);
                      setAndCheckOutput(out2);
                      setAndCheckOutput(out3);
                      delay(100);
                      setAndCheckOutput(outPower);                      
                      mqttpublishJSON("outPower","ON");                      
                      mqttpublishJSON("out1","ON");
                      mqttpublishJSON("out2","ON");
                      mqttpublishJSON("out3","ON");
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 1:
                      Serial.println("1 4 ein");
                      mqttpublishJSON("out1","ON");
                      mqttpublishJSON("outPower","ON");                      
                      setAndCheckOutput(out1);
                      delay(100);
                      setAndCheckOutput(outPower);                      
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 2:
                      Serial.println("2 4 ein");
                      mqttpublishJSON("out1","ON");
                      mqttpublishJSON("outPower","ON");                      
                      setAndCheckOutput(out2);
                      delay(100);
                      setAndCheckOutput(outPower);                      
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 3:
                      Serial.println("3 4 ein");
                      mqttpublishJSON("out3","ON");
                      mqttpublishJSON("outPower","ON");                      
                      setAndCheckOutput(out3);
                      delay(100);
                      setAndCheckOutput(outPower);                      
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  default:
                      resetAllOutputs();
                      shortpressCounter = 0;
                      Serial.println("reset counter");
                      allOff = true;
                      break;
                }
            }
        }
    }else{
        if(switchFunktion >= 1){
            Serial.println("Alles aus. Es war min. ein Kanal ein.");
            resetAllOutputs();
            shortpressCounter = 0;
            Serial.println("reset counter");
            allOff = true;
        }
    }
}


void loop(){
  
    while(1){
  
        loop_server();

        static bool in1Detected = false;
        static bool in2Detected = false;
        unsigned long in1pressTimestamp;
        unsigned long in2pressTimestamp;
        

        if((digitalRead(in1) == LOW) && (!in1Detected)){
            delay(60);
            if (digitalRead(in1) == LOW){
                Serial.println("Taste 1 gedrueckt");
                mqttpublishJSON("key1","ON");
                in1Detected = true;
                in1pressTimestamp = millis();
            }
        }
        
        if((digitalRead(in2) == LOW) && (!in2Detected)){
            delay(60);
            if (digitalRead(in2) == LOW){
                Serial.println("Taste 2 gerdrueckt");
                mqttpublishJSON("key2","ON");
                in2Detected = true;
                in2pressTimestamp = millis();
            }
        }
        
        if((digitalRead(in1) == HIGH) && (in1Detected)){
            delay(60);
            if (digitalRead(in1) == HIGH){
                Serial.println("Taste 1 losgelassen");
                mqttpublishJSON("key1","OFF");
                in1Detected = false;
                runTimer = true;
                timstampTimeout = millis();
                setOutputs(getPressFunktion(millis() - in1pressTimestamp), 1);
            }
        }
        
        if((digitalRead(in2) == HIGH) && (in2Detected)){
            delay(60);
            if (digitalRead(in2) == HIGH){
                Serial.println("Taste 2 losgelassen");
                mqttpublishJSON("key1","OFF");
                in2Detected = false;
                runTimer = true;
                timstampTimeout = millis();
                setOutputs(getPressFunktion(millis() - in2pressTimestamp), 2);
            }
        }   
        
        if ((runTimer) && ((timstampTimeout + timeout) <= millis())){
            //reset Outputs
            mqttpublish("Info", "timer reseet");
            Serial.println("Timer abgelaufen");
            setOutputs(0,0,true);
            runTimer = false;
        }
  
    }
}








































