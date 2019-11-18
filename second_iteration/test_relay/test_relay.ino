const int motor_relay = 5;
const int capacitor_relay = 6;
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
pinMode(motor_relay,OUTPUT);
digitalWrite(motor_relay,HIGH);
pinMode(capacitor_relay,OUTPUT);
digitalWrite(capacitor_relay,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

digitalWrite(motor_relay,LOW);
digitalWrite(capacitor_relay,LOW);
Serial.println("relay on");
delay(1000);
digitalWrite(motor_relay,HIGH);
digitalWrite(capacitor_relay,HIGH);
Serial.println("relay off");
delay(1000);


}
