//for rtc
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>//connect sda of rtc to A4 and scl of rtc to A5 of arduino
//

//for voltage sensor
#include "EmonLib.h"             // Include Emon Library

#define VOLT_CAL 148.7

EnergyMonitor emon1;
EnergyMonitor emon2;
unsigned long printPeriod = 1000; //Refresh rate
unsigned long previousMillis = 0;

//for voltage measurement
const int numReadings = 10;

float readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

//for current measurement
const int numReadings2 = 10;

float readings2[numReadings2];      // the readings from the analog input
int readIndex2 = 0;              // the index of the current reading
float total2 = 0;                  // the running total
float average2 = 0;                // the average



char inchar; // variable to store the incoming character gsm module
#include <SoftwareSerial.h>
SoftwareSerial SIM900(7, 8);//RX,TX
int user_input=0;
int motor_status=0;
const int motor_relay = 5;
const int capacitor_relay = 6;//digital output pin that the capacitor is attached to
int red_flag =0;
float line_voltage=0;
float estimate_volts=0;
int water_sensor=0;
//these are the default measurement or constants used below can be changed here
#define min_line_voltage 0.75  // voltage range in which  the motor should operate
#define max_line_voltage 1.50
float estimate_current=0;
float device_current=0;
#define min_current 20//current reange in which the motor should operate
#define max_current 25
int present_hour=0;
int last_ack_hour=0;
#define ack_after_hours 4
int present_minute=0;
int last_ack_minute=0;
#define ack_after_minutes 10
int hourly_notification=0;
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

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {//for voltage sensor
    readings[thisReading] = 0;
  }

emon1.voltage(A0, VOLT_CAL, 1.7);
///if voltage is not in range then jump to measure voltage after 10 secs
//delay(4000);


}

void loop() {
  volt_measure();
  //while((user_input==0)  (motor_status==0)){
  if(motor_status==1){
    if(red_flag==1 | user_input==0){
     motor_stop();
     Serial.println("motor has stopped");
    }
  }
    gsm_recieve_send();
 // }
  while((user_input==1) and (motor_status==0) and (red_flag==0)){
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
    water_running();
  }
  //present_time();
}










//functions are declared here
void present_time(){
  tmElements_t tm;

  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    present_hour=tm.Hour;
    Serial.write(':');
    print2digits(tm.Minute);
    present_minute=tm.Second;
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }
  Serial.println(present_minute);
  Serial.print(last_ack_minute);
  if((present_minute-last_ack_minute)>=ack_after_minutes){
    last_ack_hour=present_hour;
    last_ack_minute=present_minute;
    hourly_notification=1;
  }else{
    hourly_notification=0;
  }
 Serial.print("hourly_notification_status ");
 Serial.println(hourly_notification);
}
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
///{put the voltage measurement code here use function so that it will be easier}
float volt_measure(){
  emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
  
  estimate_volts   = emon1.Vrms;             //extract Vrms into Variable
  
  Serial.println(estimate_volts);

           // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = estimate_volts;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  line_voltage = total / numReadings;
  // send it to the computer as ASCII digits
  Serial.print("\taverage");
  Serial.println(line_voltage);
  delay(5);        // delay in between reads for stability
  if((estimate_volts<min_line_voltage) | (estimate_volts>max_line_voltage) | (line_voltage<min_line_voltage) | (line_voltage>max_line_voltage) ){
    red_flag=0;
    Serial.println("red_flag raised");
  }
  else{
    red_flag=0;
    Serial.println("red_flag down allok");
  }
  
  //red_flag=0;//for testing only
  //float analog_measure=analogRead(A0);
  ///write the regression equation here and estimate the line_voltage here return the line voltage
  Serial.println("voltage measure function executed");
}
///{put current measurement FUNCTION here}
float current_measure(){
  emon2.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
  
  estimate_current= emon2.Vrms;             //extract Vrms into Variable
  
  Serial.println(estimate_current);

           // subtract the last reading:
  total2 = total2 - readings2[readIndex2];
  // read from the sensor:
  readings2[readIndex2] =estimate_current;
  // add the reading to the total:
  total2 = total2 + readings2[readIndex2];
  // advance to the next position in the array:
  readIndex2 = readIndex2 + 1;

  // if we're at the end of the array...
  if (readIndex2 >= numReadings2) {
    // ...wrap around to the beginning:
    readIndex2 = 0;
  }

  // calculate the average:
  device_current= total2 / numReadings2;
  // send it to the computer as ASCII digits
  Serial.print("\taverage");
  Serial.println(device_current);
  delay(5);        // delay in between reads for stability
  if((estimate_current<min_current) | (estimate_current>max_current)){
    red_flag=0;
    Serial.println("red_flag raised");
  }
  else{
    red_flag=0;
    Serial.println("red_flag down allok");
  }
  
  //red_flag=0;//for testing only
  //float analog_measure=analogRead(A0);
  ///write the regression equation here and estimate the line_voltage here return the line voltage
  Serial.println("current measure function executed");
///{put current measurement FUNCTION here}
  //red_flag=0;//for testing only
  //float analog_measure=analogRead(A1);
  //write the regression equation here and estimmate the current here and return the current in amps
}
//define motor starting sequencehere function
void motor_start(){
  digitalWrite(motor_relay,LOW);
  digitalWrite(capacitor_relay,LOW);
  delay(2000);
  digitalWrite(capacitor_relay,HIGH);
  Serial.println("motor starting sequence executed");
  motor_status=1;
  SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
          delay(1000);
          String dataMessage = ("motor has started");//use this line if adding extra parameters to pass
          SIM900.println(dataMessage);
          delay(100);
          SIM900.println((char)26);// ASCII of ctrl+z
          delay(3000);
        SIM900.println("AT+CMGD=1,4"); // delete all SMS
        delay(1000);
  Serial.println("Motor start message notified to user");

}

///define motor stopping sequence here
void motor_stop(){
  digitalWrite(motor_relay,HIGH);
  motor_status=0;
  Serial.println("motor stopping sequence executed");
  SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
          delay(1000);
          String dataMessage = ("motor has stopped");//use this line if adding extra parameters to pass
          SIM900.println(dataMessage);
          delay(100);
          SIM900.println((char)26);// ASCII of ctrl+z
          delay(3000);
        SIM900.println("AT+CMGD=1,4"); // delete all SMS
        delay(1000);
  Serial.println("Motor stop message notified to user");

}
void water_running(){
  red_flag=0;
  //if(water_sensor==0){
    //red_flag==1;
  //}
  Serial.println("water running sequence executed");
}
void gsm_recieve_send(){
  //this function is to recieve or send sms
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
        delay(500);
        }
      }
      if (inchar=='#')
      {
        delay(10);

        inchar=SIM900.read();
        if (inchar=='S')
        {   
          Serial.println("mc has recieved the status request");
          SIM900.println("AT+CMGF=1");
          delay(1000);
          SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
          delay(1000);
          String dataMessage = ("estimate_volts: " + String(estimate_volts)+ " \naverage_voltage: " + String(line_voltage)+"\nred_flag_status: "+String(red_flag));
          SIM900.println(dataMessage);
          delay(100);
          SIM900.println((char)26);// ASCII of ctrl+z
          delay(3000);
        SIM900.println("AT+CMGD=1,4"); // delete all SMS
        delay(1000);
        }
      }
    }
}
