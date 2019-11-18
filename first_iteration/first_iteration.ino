char inchar; // variable to store the incoming character 
#include <SoftwareSerial.h>
SoftwareSerial SIM900(7, 8);//RX,TX
int user_input=0;
int motor_status=0;
const int motor_relay = 4;
const int capacitor_relay = 3;//digital output pin that the capacitor is attached to
int red_flag =0;
int line_voltage=0;
int line_voltage_status=0;
int water_sensor=0;
float preset_duration=10000;//if needed can run the motor for an amount of time and auto shut down
//these are the default measurement or constants used below can be changed here
const int min_line_voltage=200;// voltage range in which  the motor should operate
const int max_line_voltage=250;
int device_current_status=0;
int device_current=0;
//const int min_current=.......;//current reange in which the motor should operate
//const int max_current=.......;

int icount=0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SIM900.begin(9600);
  delay(2000); 
  SIM900.println("AT+CMGF=1");// set SMS mode to text
  delay(100);
  SIM900.println("AT+CNMI=2,2,0,0,0");
  // just to get a notification when SMS arrives &direct out SMS upon receipt to the GSM serial out
  delay(100);

  // digital output pin that the motor is attached to
pinMode(motor_relay,OUTPUT);
digitalWrite(motor_relay,HIGH);
pinMode(capacitor_relay,OUTPUT);
digitalWrite(capacitor_relay,HIGH);

delay(5000);
Serial.println("program started and successfully connected to SERIAL monitor");
delay(5000);//...........??????provide a certain delay before the motor starts so that the voltages settle
///create a label voltage measurement here so that if the voltage measured is not in range then program can jump here




///if voltage is not in range then jump to measure voltage after 10 secs
//delay(4000);
}

void loop() {
  Serial.println("program has reached loop");
  //while((user_input==0)  (motor_status==0)){
    if(red_flag==1 | user_input==0){
     motor_stop();
     Serial.println("motor has stopped");
    }
    Serial.println("program has entered the gsm recieve stage");
    while(SIM900.available() >0)
    {
      inchar=SIM900.read();
      if (inchar=='$')
      {
        delay(10);

        inchar=SIM900.read();
        if (inchar=='a')
        {
          delay(10);
          inchar=SIM900.read();
          if (inchar=='0')
          {     
      user_input=0;
      Serial.println("user input is 0");
          }
          else if (inchar=='1')
          {
           Serial.println("user input is 1");
           user_input=1;
          }
          delay(100);
        SIM900.println("AT+CMGD=1,4"); // delete all SMS
        delay(2000);
        }
      }
    }
 // }
  while((user_input==1) and (motor_status==0) and (red_flag==0)){
    volt_measure();
    Serial.println("program has measured voltage");
    ///start the motor if everything is ok 
    if((red_flag==0) and (user_input==1)){
      motor_start();
      delay(5000);
      Serial.println("motor has started ");
    }
  }
  if(motor_status==1){
    current_measure();
    volt_measure();
    water_running();
  }
  delay(10000);
}











//functions are declared here
///{put the voltage measurement code here use function so that it will be easier}
float volt_measure(){
  red_flag=0;//for testing only
  //float analog_measure=analogRead(A0);
  ///write the regression equation here and estimate the line_voltage here return the line voltage
  Serial.println("voltage measure function executed");

}

///{put current measurement FUNCTION here}
float current_measure(){
  red_flag=0;//for testing only
  //float analog_measure=analogRead(A1);
  //write the regression equation here and estimmate the current here and return the current in amps
  Serial.println("current measurement function executed");
}
//define motor starting sequencehere function
void motor_start(){
  digitalWrite(motor_relay,LOW);
  digitalWrite(capacitor_relay,LOW);
  delay(2000);
  digitalWrite(capacitor_relay,HIGH);
  Serial.println("motor starting sequence executed");
  motor_status=1;
}

///define motor stopping sequence here
void motor_stop(){
  digitalWrite(motor_relay,HIGH);
  Serial.println("motor stopping sequence executed");
  motor_status=0;
}
void water_running(){
  red_flag=0;
  //if(water_sensor==0){
    //red_flag==1;
  //}
  Serial.println("water running sequence executed");
}
