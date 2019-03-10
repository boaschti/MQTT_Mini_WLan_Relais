


#define userpage_existing
#define AllowAcessPoint

#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESPServer.h"
#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESPServer.cpp"
#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESP8266HTTPUpdateServer.h"
#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESP8266HTTPUpdateServer.cpp"



/*
void UserSubscribe(){
    mqttpublish("Info", "usersubscribe");
}

void MQTTBrokerChanged(){
    return;  
}
*/

//paired outputs
// out1, out2
// out3, out5
// out4, out6


#define lockpairedOutputs
#define outpin1 5
#define outpin2 15
#define outpin3 4
#define outpin4 16
#define outpin5 12
#define outpin6 14

#define out1 outpin1
#define out2 outpin2
#define out3 outpin3
#define out4 outpin5
#define out5 outpin4
#define out6 outpin6

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


void callback(char* topic, byte* payload, unsigned int length) {
    mqttpublish("Info", "got Message");
}


  
void setup() {

    pinMode(out1, OUTPUT);
    pinMode(out2, OUTPUT);
    pinMode(out3, OUTPUT);
    pinMode(out4, OUTPUT);
    pinMode(out5, OUTPUT);
    pinMode(out6, OUTPUT);

    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);
    digitalWrite(out5, LOW);
    digitalWrite(out6, LOW);
    
    
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
    if ((presstime >= middlelongpress) && (presstime <= longpress)){
        return 2;
    }
    if (presstime > longpress){
        return 3;
    }
    
}

uint8_t getPairedOutput(uint8_t output){
    
    #ifdef lockpairedOutputs
    if(output == out1){
        return out2;
    }
    if(output == out2){
        return out1;
    }
    if(output == out3){
        return out4;
    }
    if(output == out4){
        return out3;
    }
    if(output == out5){
        return out6;
    }
    if(output == out6){
        return out5;
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
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);
    digitalWrite(out5, LOW);
    digitalWrite(out6, LOW);
    mqttpublishJSON("out1","OFF");
    mqttpublishJSON("out2","OFF");
    mqttpublishJSON("out3","OFF");
    mqttpublishJSON("out4","OFF");
    mqttpublishJSON("out5","OFF");
    mqttpublishJSON("out6","OFF");
  
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
                      Serial.println("1 3 5 ein");
                      setAndCheckOutput(out1);
                      setAndCheckOutput(out3);
                      setAndCheckOutput(out5);
                      mqttpublishJSON("out1","ON");
                      mqttpublishJSON("out3","ON");
                      mqttpublishJSON("out5","ON");
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 1:
                      Serial.println("1 ein");
                      mqttpublishJSON("out1","ON");
                      setAndCheckOutput(out1);
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 2:
                      Serial.println("3 ein");
                      mqttpublishJSON("out3","ON");
                      setAndCheckOutput(out3);
                      shortpressCounter = 0;
                      allOff = false; 
                      break;
                  case 3:
                      Serial.println("5 ein");
                      mqttpublishJSON("out5","ON");
                      setAndCheckOutput(out5);
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
                      Serial.println("2 4 6 ein");
                      setAndCheckOutput(out2);
                      setAndCheckOutput(out4);
                      setAndCheckOutput(out6);
                      mqttpublishJSON("out2","ON");
                      mqttpublishJSON("out4","ON");
                      mqttpublishJSON("out6","ON");
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 1:
                      Serial.println("2 ein");
                      mqttpublishJSON("out2","ON");
                      setAndCheckOutput(out2);
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 2:
                      Serial.println("4 ein");
                      mqttpublishJSON("out4","ON");
                      setAndCheckOutput(out4);
                      shortpressCounter = 0;
                      allOff = false;
                      break;
                  case 3:
                      Serial.println("6 ein");
                      mqttpublishJSON("out6","ON");
                      setAndCheckOutput(out6);
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
        static bool runTimer = false;
        static unsigned long timstampTimeout;
        unsigned long in1pressTimestamp;
        unsigned long in2pressTimestamp;
        

        if((digitalRead(in1) == LOW) && (!in1Detected)){
          Serial.println("Taste 1 gedrueckt");
          mqttpublishJSON("key1","ON");
          in1Detected = true;
          in1pressTimestamp = millis();
        }
        
        if((digitalRead(in2) == LOW) && (!in2Detected)){
          Serial.println("Taste 2 gerdrueckt");
          mqttpublishJSON("key2","ON");
          in2Detected = true;
          in2pressTimestamp = millis();
        }
        
        if((digitalRead(in1) == HIGH) && (in1Detected)){
            Serial.println("Taste 1 losgelassen");
            mqttpublishJSON("key1","OFF");
            in1Detected = false;
            runTimer = true;
            timstampTimeout = millis();
            setOutputs(getPressFunktion(millis() - in1pressTimestamp), 1);
        }
        
        if((digitalRead(in2) == HIGH) && (in2Detected)){
          Serial.println("Taste 2 losgelassen");
          mqttpublishJSON("key1","OFF");
          in2Detected = false;
          runTimer = true;
          timstampTimeout = millis();
          setOutputs(getPressFunktion(millis() - in2pressTimestamp), 2);
        }   
        
        if ((runTimer) && ((timstampTimeout + timeout) <= millis())){
            //reset Outputs
            Serial.println("Timer abgelaufen");
            setOutputs(0,0,true);
            runTimer = false;
        }
  
    }
}








































