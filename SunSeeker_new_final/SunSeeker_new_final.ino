#include <Servo.h>

int photoPinLeft = A0; //green
int photoPinRight = A1; //blue
int photoPinTop = A2;  //orange
int photoPinBottom = A3; //brown
int servoRelayOff = 5;
int servoRelayOn = 6;
int servoRelayState = 11;
int piLed = 13;    //Pi light level led

int getSunLevel() {
    int lowLightCutoff = 600;
    int highLightCutoff = 700;
    //average all four photo sensor values over time
    int sensorAvg = (digitalRead(photoPinLeft) + digitalRead(photoPinRight) + digitalRead(photoPinTop) + digitalRead(photoPinBottom)) / 4;
    int total = 0;
    for (int i = 0; i < 10; i++) {
      total += sensorAvg;
      }
    int avgPhotoLevel = total / 10;
    //Serial.println(avgPhotoLevel);
    if (avgPhotoLevel <= lowLightCutoff ) {
      return 0;
    } else if (avgPhotoLevel > lowLightCutoff && avgPhotoLevel < highLightCutoff) {
      return 1;
    } else {
      return 2;  
    } 
    
}

class Seeker {
  public: 
  Servo servo;
  int val1;
  int val2;
  int tol;    //Adjustment so it doesn't constantly move
  int pos;
  int startPos;
  int endPos;
  int parkPos;
  int updateInterval;
  unsigned long lastUpdate;

  public: 
  Seeker(int startP, int endP, int parkP) {
    startPos = startP;
    endPos = endP;
    parkPos = parkP;
    lastUpdate = 0;
    updateInterval = 100; //How often the position is updated in ms
    tol = 2;
  }
  
  void Attach(int pin) {
    servo.attach(pin);
  }

  void park() {
    servo.write(parkPos);
  }
  
  void Detach() {
    servo.detach();
  }
  
  void Update(int pin1, int pin2) {
    
    int sunLevel = getSunLevel();
    val1 = analogRead(pin1);  
    val2 = analogRead(pin2);
    
    if((millis() - lastUpdate) > updateInterval) {
      lastUpdate = millis();

      switch (sunLevel) {
        case 0 : 
          if (digitalRead(servoRelayState) == HIGH) {
            pos = parkPos;                      //park solar panel     
            servo.write(pos);
            delay(2000);      
            digitalWrite(servoRelayOff, HIGH); 
            delay(10);
            digitalWrite(servoRelayOff, LOW);   //turn off relay
          }
          digitalWrite(piLed, LOW);        //set pi led to off
          Serial.println("Case 0");
          break;
        case 1 : 
          //hold position, if relay is on leave it on, if relay is off leave it off
          digitalWrite(piLed, LOW);        //set pi led to off
          Serial.println("Case 1");
          break;
        case 2 :
          digitalWrite(piLed, HIGH);        //set pi led to on
          if (digitalRead(servoRelayState) == LOW) {
             digitalWrite(servoRelayOn, HIGH); 
             delay(10);
             digitalWrite(servoRelayOn, LOW);   //turn relay on
          }
                                              //track sun
          if ((abs(val1 - val2) <= tol) && (abs(val2 - val1) <= tol)) {
            pos = pos;   //Hold position when photoresistors are within the tolerence
          } else if (pos > endPos) {
            pos = endPos;
          } else if (pos < startPos) {
            pos = startPos;  
          } else if (val1 > val2) {
            pos = --pos;
          } else if (val2 > val1) {
            pos = ++pos;
          }
          servo.write(pos);
          delay(50);
          Serial.println("Case 2");
          break;
        default:
          pos = parkPos;                      //park solar panel     
          servo.write(pos);
          delay(2000);        
          digitalWrite(servoRelayOff, HIGH); 
          delay(10);
          digitalWrite(servoRelayOff, LOW);   //turn off relay
          digitalWrite(piLed, LOW);        //set pi led to off
          Serial.println("default");
        }
    }          
  }
};

Seeker hSeeker(0, 180, 90);
Seeker vSeeker(50, 130, 70);


void setup() {
  Serial.begin(9600);
  pinMode(servoRelayOff, OUTPUT);
  pinMode(servoRelayOn, OUTPUT);
  pinMode(servoRelayState, INPUT);
  pinMode(piLed, OUTPUT);
  digitalWrite(servoRelayOn, HIGH); 
  delay(10);
  digitalWrite(servoRelayOn, LOW);
  digitalWrite(piLed, HIGH);
  hSeeker.Attach(9);
  vSeeker.Attach(10);
  hSeeker.park();
  vSeeker.park();
  delay(2000);
}

void loop() {
  vSeeker.Update(photoPinTop, photoPinBottom);
  hSeeker.Update(photoPinLeft, photoPinRight);
}

