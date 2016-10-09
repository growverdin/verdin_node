//Libraries
#include <DHT.h>

void setup() {
  Serial.begin(9600);
}

void loop() {  
  bool endOfMessage = false;
  String message[20];
  int commandCount = 0;
  String serialRead;

  //Receiving messages
  while(!endOfMessage) {
    if ( Serial.available() > 0 ) {
      serialRead = Serial.readString();

      if (serialRead.equalsIgnoreCase("startingMessage")) {
        commandCount = 0;
        Serial.write("sendNext");
      } else {
        while (serialRead.indexOf("/") > -1) {
          message[commandCount] += serialRead.substring(0, serialRead.indexOf("/"));
          serialRead = serialRead.substring(serialRead.indexOf("/")+1, serialRead.length());
          commandCount++;
        }

        if (serialRead.equalsIgnoreCase("*")) {
          endOfMessage = true;
        } else {
          message[commandCount] += serialRead.substring(0, serialRead.length());
          Serial.write("sendNext");
        }
      }
    }
  }

  //Signal that received message
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);

  //if it has commands to execute
  if (commandCount > 0) {
    String response[20];
    int responseCount = 0;

    //executes action of each command
    for (int i=0 ; i<commandCount ; i++) {
      String command = message[i];
      int action = command.substring(0, command.indexOf("#")).toInt();
      command = command.substring(command.indexOf("#"), command.length());

      //turn on power port
      pinMode(12, OUTPUT);
      digitalWrite(12, HIGH);
      delay(500);
      
      int pin;
      int value;

      //executes the action for each of the ports
      while (command.indexOf("#") > -1) {        
        //get next pin
        if (String(command.charAt(1)).equalsIgnoreCase("0")) {
          pin = String(command.charAt(command.indexOf("#")+2)).toInt();
        } else {
          pin = command.substring(command.indexOf("#")+1, command.indexOf("#")+3).toInt();
        }

        //check if it has value attached for actuation
        if (command.indexOf('?') > -1) {
          if (command.indexOf('#', command.indexOf('?')) > -1) {
            value = command.substring(command.indexOf('?')+1, command.indexOf('#', command.indexOf('?'))).toInt();
          } else {
            value = command.substring(command.indexOf('?')+1, command.indexOf("/")).toInt();
          }
        }

        //performs the action
        //MEASUREMENTS
        if (action == 1) {
          pinMode(pin, INPUT);
          response[responseCount] = String((int)(((analogRead(pin)-213)*-0.123456789)+98));
          responseCount++;
        } else if (action == 3) {
          DHT dht(pin, DHT22);
          dht.begin();
          float temp = dht.readTemperature();
          float hum = dht.readHumidity();
          response[responseCount] = String(temp) + "/" + String(hum);
          responseCount++;
        } else if (action == 4) {
          pinMode(pin, INPUT);
          response[responseCount] = String(((float)analogRead(pin)*5.0/1024/6)*3.5);
          responseCount++;
        }
        //ACTUATIONS
        else if (action == 2) {
          if (value == -1) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
          } else {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            delay(value);
            digitalWrite(pin, HIGH);
          }
          response[responseCount] = String(value);
          responseCount++;
        }

        //check if has more pins and erase the last one
        if (command.indexOf("#", 1) > -1) {
          command = command.substring(command.indexOf("#", 1), command.length());
        } else {
          command = "";
        }
      }
      
      //turn off power port
      delay(500);
      digitalWrite(12, LOW);
    }

    //sends each result value
    for (int i=0 ; i<responseCount ; i++) {
      Serial.print(response[i]);
      delay(100);
    }

    //sends signal to end the communication
    Serial.print("endOfMessage");
  }
}
