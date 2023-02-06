#include <IRremote.h>
int RECV_PIN = 11;
int led=2;
IRrecv irrecv(RECV_PIN);
decode_results results;
void setup(){
  Serial.begin(9600);
  pinMode(lde,OUTPUT);
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
}
void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
     // Receive the next value
    if(results.value==0xFF30CF){
     digitalWrite(led,HIGH);
    }else if(results.value==0xFF18E7){
      digitalWrite(led,LOW);
    }
    irrecv.resume();
  }
  delay(100);
}
