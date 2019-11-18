#include <avr/wdt.h>
//to store the current time and date
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
int alarm_day=0;
int alarm_month=0;
int alarm_time_hour=0;
int alarm_time_min=0;
int no_of_hrs=0;
int no_of_mins=0;
int off_timer=0;
tmElements_t tm;
//....gsm variables here 
char inchar; // variable to store the incoming character
// 
//..motor and capacitor names declared here
int motorRelay = 8;
int capacitorRelay=7;
int motorOffRelay=9;
//
//for voltage sensor
//#include "EmonLib.h"             // Include Emon Library
//
//#define VOLT_CAL 148.7
//#define CURR_CAL 1
//
//EnergyMonitor emon1;
//EnergyMonitor emon2;
//unsigned long printPeriod = 1000; //Refresh rate
//unsigned long previousMillis = 0;
//
//          //for voltage measurement
//const int numReadings = 10;
//
//float readings[numReadings];      // the readings from the analog input
//int readIndex = 0;              // the index of the current reading
//float total = 0;                  // the running total
//float average = 0;                // the average
//float average_line_voltage=0;//average of the line_voltage is stored in this variable    n/10
//float line_voltage=0;//instantaneous voltage is stored here
//#define min_line_voltage 0.75  // min voltage in which  the motor should operate
//#define max_line_voltage 1.50  // max voltage in which  the motor should operate
/////
////for current measurement
//const int numReadings2 = 10;
//
//float readings2[numReadings2];      // the readings from the analog input
//int readIndex2 = 0;              // the index of the current reading
//float total2 = 0;                  // the running total
//float average2 = 0;                // the average
//float estimate_current=0;//current at that instant
//float average_estimate_current=0;//average of the current of ten readings n/10
//#define min_current 20// min current in which the motor should operate
//#define max_current 25// max current in which the motor should operate
///
//for gsm module
#include <SoftwareSerial.h>
SoftwareSerial SIM900(5, 6);//RX,TX
//
//user defined variables
int user_input=0;//if user wants to switch on the motor or not
int motor_status=0;//the motor status at any given time.this is a digital variable meaning the physical running of the motor is not specified.
int red_flag =0;//any parameters out of range then this red_flag is raised and anywhere in the program this occurs the motor is turned down.
//int water_sensor=0;//not needed in this iteration
int initialize=1;

void setup()
{
  wdt_disable();//watch_dog timer is disabled here so that if enabled elsewhere it gets nullified
  //some of the setup is done here
  Serial.begin(9600);//for screen output
  SIM900.begin(9600);//for gsm output
  delay(2000); 
  SIM900.println("AT+CMGF=1");// set SMS mode to text
  delay(100);
  SIM900.println("AT+CNMI=2,2,0,0,0");// just to get a notification when SMS arrives &direct out SMS upon receipt to the GSM serial out
  delay(100);
  // digital output pin that the motor is attached to
  pinMode(motorRelay,OUTPUT);
  digitalWrite(motorRelay,HIGH);
  pinMode(capacitorRelay,OUTPUT);
  digitalWrite(capacitorRelay,HIGH);
  pinMode(motorOffRelay,OUTPUT);
  digitalWrite(motorOffRelay,HIGH);
  delay(5000);
  delay(5000);//...........??????provide a certain delay before the motor starts so that the voltages settle
  wdt_enable(WDTO_4S);
//  emon1.voltage(A0, VOLT_CAL, 1.7);
//  emon2.voltage(A1, VOLT_CAL, 1.7);
//for time and date only
  read_time();
  display_time();
  Serial.println("program started and successfully connected to SERIAL monitor at ");
}

void loop()
{
  read_time();
  display_time();//remove this after code is finalised
  if (initialize==1){
    SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
  delay(1000);
  Serial.println("notified_user");
  wdt_reset();
  String dataMessage = ("mc has started and entered loop");//use this line if adding extra parameters to pass
  SIM900.println(dataMessage);
  delay(100);
  SIM900.println((char)26);// ASCII of ctrl+z
  wdt_reset();
  delay(3000);
  wdt_reset();
  SIM900.println("AT+CMGD=1,4"); // delete all SMS
  wdt_reset();
  delay(1000);
  wdt_reset();
  initialize=0;
  }
  //program has reached gsm recieve stage
  gsm_function();//scan for any new inputs from user
//  volt_measure();
//  //while((user_input==0)  (motor_status==0)){
  if(motor_status==1){
    if(red_flag==1 | user_input==0){
     motor_stop();
     Serial.println("motor has stopped");
    }
  }
  while((user_input==1) and (motor_status==0) and (red_flag==0)){
    Serial.println("program has measured voltage");
    ///start the motor if everything is ok 
    if((red_flag==0) and (user_input==1)){
      motor_start();
      wdt_reset();
      delay(2000);
      wdt_reset();
      Serial.println("motor has started ");
    }
  }
  if(motor_status==1){
//      current_measure();
      water_running();
  } 
  if(off_timer==1){
    timer_switch_off();
  }
}
/////
void motor_start(){
  digitalWrite(motorRelay,LOW);
  digitalWrite(capacitorRelay,LOW);
  delay(1500);
  wdt_reset();
  digitalWrite(capacitorRelay,HIGH);
  delay(1000);
  digitalWrite(motorRelay,HIGH);
  Serial.println("motor starting sequence executed");
  motor_status=1;
  SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
  delay(1000);
  wdt_reset();
  String dataMessage = ("motor has started");//use this line if adding extra parameters to pass
  SIM900.println(dataMessage);
  delay(100);
  SIM900.println((char)26);// ASCII of ctrl+z
  wdt_reset();
  delay(3000);
  wdt_reset();
  SIM900.println("AT+CMGD=1,4"); // delete all SMS
  wdt_reset();
  delay(1000);
  wdt_reset();
  Serial.println("Motor start message notified to user");
}
////define motor stopping sequence here
void motor_stop(){
  digitalWrite(motorOffRelay,LOW);
  delay(2000);
  digitalWrite(motorOffRelay,HIGH);
  motor_status=0;
  Serial.println("motor stopping sequence executed");
  SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
  delay(1000);
  wdt_reset();
  String dataMessage = ("motor has stopped");//use this line if adding extra parameters to pass
  SIM900.println(dataMessage);
  delay(100);
  SIM900.println((char)26);// ASCII of ctrl+z
  wdt_reset();
  delay(3000);
  wdt_reset();
  SIM900.println("AT+CMGD=1,4"); // delete all SMS
  delay(1000);
  wdt_reset();
  Serial.println("Motor stop message notified to user");
  wdt_reset();
}





///gsm function
void gsm_function(){
  Serial.println("gsm has entered recieve loop");
//If a character comes in from the GSM...
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
          SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
          delay(1000);
          wdt_reset();
          String dataMessage = ("switching off motor in few secs");//use this line if adding extra parameters to pass
          SIM900.println(dataMessage);
          delay(100);
          SIM900.println((char)26);// ASCII of ctrl+z
          wdt_reset();
          delay(3000);
        }
        else if (inchar=='1')
        {
          user_input=1;
          SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
          delay(1000);
          String dataMessage = ("switching on motor in few secs");//use this line if adding extra parameters to pass
          SIM900.println(dataMessage);
          delay(100);
          SIM900.println((char)26);// ASCII of ctrl+z
          wdt_reset();
          delay(3000);
        }
        delay(100);
       SIM900.println("AT+CMGD=1,4"); // delete all SMS
       wdt_reset();

       delay(2000);
      }
      
    }
    else if (inchar=='#')
    {
      delay(10);

      inchar=SIM900.read();
      if (inchar=='s')
      {
        delay(10);
        SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
        delay(1000);
        String dataMessage = ("motor_status: " + String(motor_status)+"\nred_flag_status: "+String(red_flag));
        SIM900.println(dataMessage);
        delay(100);
        SIM900.println((char)26);// ASCII of ctrl+z
        wdt_reset();
        Serial.println("watch_dog_reset_function_executed");
        delay(3000);
      }
    }
    else if (inchar=='s')
    {
      delay(10);

      inchar=SIM900.read();
      if (inchar=='h')
      {
        delay(10);
        inchar=SIM900.read();
        delay(10);
        no_of_hrs=inchar;
        read_time();
        if(tm.Hour+no_of_hrs>23){
          alarm_time_hour=tm.Hour+no_of_hrs-24;
          if((tm.Month==1 || tm.Month==3 || tm.Month==5 || tm.Month==7 || tm.Month==8 || tm.Month==10 || tm.Month==12) && ((tm.Day+1)>31)){
            alarm_month=tm.Month+1;
            alarm_day=1;
          }
          else if ((tm.Month==4 || tm.Month==6 || tm.Month==9 || tm.Month==10 || tm.Month==11) && ((tm.Day+1)>30)){
            alarm_month=tm.Month+1;
            alarm_day=1;
          }
          else if ((tm.Month==2) && ((tm.Day+1)>28)){
            alarm_month=tm.Month+1;
            alarm_day=1;
          }
          else{
            alarm_day=tm.Day+1;
            alarm_month=tm.Month;
          }
        }
        else{
          alarm_time_hour=tm.Hour+no_of_hrs;
        }
        SIM900.println("AT+CMGS=\"+918861812823\"\r"); // Input the mobile number| YY is country code
        delay(1000);
        String dataMessage = ("alarm set for motor to switch off at " + String(alarm_time_hour)+":00 "+" on "+String(alarm_day)+"/"+String(alarm_month));
        SIM900.println(dataMessage);
        delay(100);
        SIM900.println((char)26);// ASCII of ctrl+z
        wdt_reset();
        Serial.println("watch_dog_reset_function_executed");
        delay(3000);
      }
    }
  }
  wdt_reset();
  Serial.println("watch_dog_reset_function_executed");
}

///
//void volt_measure(){
//  emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
//  
//  line_voltage   = emon1.Vrms;             //extract Vrms into Variable
//  
////  Serial.println(line_voltage);
//
//           // subtract the last reading:
//  total = total - readings[readIndex];
//  // read from the sensor:
//  readings[readIndex] = line_voltage;
//  // add the reading to the total:
//  total = total + readings[readIndex];
//  // advance to the next position in the array:
//  readIndex = readIndex + 1;
//
//  // if we're at the end of the array...
//  if (readIndex >= numReadings) {
//    // ...wrap around to the beginning:
//    readIndex = 0;
//  }
//
//  // calculate the average:
//  average_line_voltage = total / numReadings;
//  // send it to the computer as ASCII digits
////  Serial.print("\taverage");
////  Serial.println(line_voltage);
//  if((line_voltage<min_line_voltage) | (line_voltage>max_line_voltage) | (average_line_voltage<min_line_voltage) | (average_line_voltage>max_line_voltage) ){
//    red_flag=0;
//    Serial.println("red_flag raised");
//  }
//  else{
//    red_flag=0;
//    Serial.println("red_flag down allok");
//  }
//  ///write the regression equation here and estimate the line_voltage here return the line voltage
//  Serial.println("voltage measure function executed");
//  wdt_reset();
//}
//
//
//
//////////////
//void current_measure(){
//  emon2.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
//  
//  estimate_current= emon2.Vrms;             //extract Vrms into Variable
//  
//  Serial.println(estimate_current);
//
//           // subtract the last reading:
//  total2 = total2 - readings2[readIndex2];
//  // read from the sensor:
//  readings2[readIndex2] =estimate_current;
//  // add the reading to the total:
//  total2 = total2 + readings2[readIndex2];
//  // advance to the next position in the array:
//  readIndex2 = readIndex2 + 1;
//
//  // if we're at the end of the array...
//  if (readIndex2 >= numReadings2) {
//    // ...wrap around to the beginning:
//    readIndex2 = 0;
//  }
//
//  // calculate the average:
//  average_estimate_current= total2 / numReadings2;
//  // send it to the computer as ASCII digits
////  Serial.print("\taverage");
////  Serial.println(average_estimate_current);
//  delay(5);        // delay in between reads for stability
//  if((estimate_current<min_current) | (estimate_current>max_current)){
//    red_flag=0;
//    Serial.println("red_flag raised");
//  }
//  else{
//    red_flag=0;
//    Serial.println("red_flag down allok");
//  }
//  
//  //red_flag=0;//for testing only
//  //float analog_measure=analogRead(A0);
//  ///write the regression equation here and estimate the line_voltage here return the line voltage
//  Serial.println("current measure function executed");
/////{put current measurement FUNCTION here}
//  //red_flag=0;//for testing only
//  //float analog_measure=analogRead(A1);
//  //write the regression equation here and estimmate the current here and return the current in amps
//  wdt_reset();
//}
//////

void water_running(){
  red_flag=0;
  //if(water_sensor==0){
    //red_flag==1;
  //}
  Serial.println("water running sequence executed");
  wdt_reset();
}
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
void read_time(){
    if (RTC.read(tm)) {
    Serial.println("Ok");
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  delay(1000);
//  if(tm.Hour==present_time_hour+no_of_hrs && tm.Minute==present_time_min+no_of_mins && tm.Second >= 0 && tm.Second <= 3){
//    Serial.println("it_works");
//  }
}
void display_time(){
    Serial.println("Time = ");    
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
}
void timer_switch_off(){
  if(tm.Day==alarm_day && tm.Month==alarm_month && tm.Hour==alarm_time_hour && tm.Minute==alarm_time_min && tm.Second >= 0 && tm.Second <= 3){
    Serial.println("it_works");//replace this with user input =0 so that the motor can switch off
    off_timer=0;
  }
}
