 #include <SoftwareSerial.h>
#include <DS3231.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
// Init the DS3231 using the hardware interface
SoftwareSerial mySerial(8, 9); // to tx,rx of gsm modem
DS3231  rtc(SDA, SCL);
#define RLY 13
#define BTN 12
#define BUZ 11
#define SBUZ 10

double volt;
#define ADC_MULTISAMPLING 5
#define ADC_MULTISAMPLING_SAMPLES (1 << ADC_MULTISAMPLING)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
bool activate = 0;
bool sms_bit = 0;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0; 
uint8_t tmp,tmp1,ss,trials,aa; 
int temp;
int minutes,ee_delay,night_delay,day_delay;
uint16_t yr;
void printtime();
int8_t read_serial(unsigned int);
void SendMessage();
void Timeset();
// A array of the weekday
char* days[] = { "NA", "Mon", "Tue", "Wed", "Thu", "Fre", "Sat", "Sun" };
  String mobone,mobtwo,mobthree;
  Time  t;
void short_beep()
{
  digitalWrite(BUZ,HIGH);
  delay(200);
  digitalWrite(BUZ,LOW);
  
}
void long_beep()
{
  digitalWrite(BUZ,HIGH);
  delay(300);
  digitalWrite(BUZ,LOW);
  
}
void rtc_setter()
{
  Serial.print("The current date and time is: ");
  printtime();
  
  Serial.println("Please change to newline ending the settings on the lower right of the Serial Monitor");
  Serial.flush();
  Timeset();
}
int read_volt(byte chan)
{
   // Filter the ADC by multisampling with the values defined at the beginning
  int adc = 0;
  for (int i = 0; i < ADC_MULTISAMPLING_SAMPLES; ++i)
    adc += analogRead(chan);
    adc = adc >> ADC_MULTISAMPLING; 
  return(adc);

}
int writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
byte len = strToWrite.length();
EEPROM.write(addrOffset, len);
for (int i = 0; i < len; i++)
{
EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
}
return addrOffset + 1 + len;
}
int readStringFromEEPROM(int addrOffset, String *strToRead)
{
int newStrLen = EEPROM.read(addrOffset);
char data[newStrLen + 1];
for (int i = 0; i < newStrLen; i++)
{
data[i] = EEPROM.read(addrOffset + 1 + i);
}
data[newStrLen] = '\0';
*strToRead = String(data);
return addrOffset + 1 + newStrLen;
}
void showTime()
{   char *buffer;
    lcd.setCursor(0,0);
    lcd.print("  ");
    lcd.print(rtc.getTimeStr()); // hour
   
}   

void setup() {
  byte n;

  // put your setup code here, to run once:
 Serial.begin(115200);
 mySerial.begin(9600);
 pinMode(BTN,INPUT_PULLUP);
 pinMode(RLY,OUTPUT);
  pinMode(10,OUTPUT);
 pinMode(BUZ,OUTPUT);
 digitalWrite(RLY,LOW);
 digitalWrite(BUZ,LOW); 
  digitalWrite(10,LOW); 
 lcd.begin(16,2);
 // Initialize the rtc object
  rtc.begin();
 currentMillis=0;
 previousMillis=0;
 ss=0;
 minutes=0;
 trials=0;
 sms_bit=0;
 delay(1000);
 readStringFromEEPROM(0x00, &mobone);
 mobone.remove(0, 1);
 mobone.remove(10, 1);
 Serial.print(mobone);
 readStringFromEEPROM(0x10, &mobtwo);
 Serial.print("-");
 mobtwo.remove(10, 1);
 Serial.print(mobtwo);
 readStringFromEEPROM(0x20, &mobthree);
 Serial.print("-");
 mobthree.remove(10, 1);
 Serial.println(mobthree);
 day_delay= EEPROM.read(0x30);
 Serial.println(day_delay);
 night_delay= EEPROM.read(0x35);
 Serial.println(night_delay);
 /*t = rtc.getTime();
 Serial.println(t.hour,DEC);
 */
 n=read_serial(5000);
 //SendMessage();
  if(n==1)
  rtc_setter();
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis(); //First, we save the current elapsed time in milliseconds
 if (currentMillis - previousMillis >= 1000) 
 {
  previousMillis = currentMillis;
 
  showTime();
  temp=read_volt(A0);
  volt = temp * (15.0/1023);
  lcd.setCursor(11,0);
  lcd.print("B");
  lcd.print(volt,1);

 ss=ss+1;
// Serial.println(ee_delay);
 if(ss>59)
 {ss=0;
 minutes++;
 short_beep();
 if(activate==1)
 {//short_beep();
  //long_beep();
   if(sms_bit==1)
   {
    lcd.clear();
    lcd.print("Sending sms");
    SendMessage();
    lcd.clear();
    sms_bit=0;
   }
 }
 }
 lcd.setCursor(0,1);
 lcd.print("Rem-");
 aa=ee_delay-minutes-1;
 if(aa<0)
 aa=0;
 lcd.write(byte(0x30+(aa/10)));
 lcd.write(byte(0x30+(aa%10)));
 lcd.print(":");
 aa=59-ss;
 lcd.write(byte(0x30+(aa/10)));
 lcd.write(byte(0x30+(aa%10)));
/* lcd.print(" T-");
 lcd.print(3-trials);*/
 
 
 
 if(((ee_delay-minutes-1)<=0) && (ss==59))
 {minutes=0; sms_bit=1;
 // trials=trials+1;
  activate=1;
  short_beep();
  /*lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Trials left ");
  lcd.print(3-trials);
  lcd.setCursor(0,1);
  lcd.print("Press Button");
   SendMessage();
   Serial.println("SMS SENT");*/
 /* if(trials>=3)
  {
    long_beep();
    lcd.clear();
  lcd.print("Sending sms");
    SendMessage();
    lcd.clear();
    trials=0;
  }*/
 }
 }
 if((digitalRead(BTN)==LOW) )
 {
  delay(500);short_beep();
  lcd.clear();
  lcd.print("  **OK** ");
  delay(500);ss=0;minutes=0;
  activate=0;trials=0;
 }
 if(activate==1)
 digitalWrite(11,HIGH);
 else
 digitalWrite(11,LOW);
 
 t = rtc.getTime();
 if(t.hour>=23 && t.hour<=6)
 {
  ee_delay=night_delay;
 }
 else
 {
  ee_delay=day_delay;
 }
}

void readADC()
{
  int i,sum=0;
  for(i=0;i<8;i++)
  {
   sum=sum+analogRead(A0);
  }
 
}
// 1- y  entered 2- other key   0 timeout
int8_t read_serial(unsigned int timeout){
    char c;
    uint8_t  answer=0;
    unsigned long previous;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial.available() != 0){
            c = Serial.read();
           
            // check if the desired answer 1  is in the response of the module
            if ( c == 'Y' || c == 'y')
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            else 
            {
                answer = 2;
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));
    return answer;
}

void printtime()
{
  // Send Day-of-Week
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  
  // Send date
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");

  // Send time
  Serial.println(rtc.getTimeStr());
  
}
// This set of codes is allows input of data
void Timeset() {
 /* while(Serial.read() >= 0) ;
  Serial.print("Please enter the current year, 2000-2999. - ");
  while (Serial.available() == 0) {}
  yr=Serial.parseInt();
  Serial.println(yr);
  while(Serial.read() >= 0) ;
  Serial.print("Please enter the current month, 1-12. - ");
  while (Serial.available() == 0) {}
  tmp = Serial.parseInt();
  Serial.println(tmp);
  while(Serial.read() >= 0) ;
  Serial.print("Please enter the current day of the month, 1-31. - ");
  while(Serial.read() >= 0) ;
  while (Serial.available() == 0) {}
  tmp1 = Serial.parseInt();
  Serial.println(tmp1);
  rtc.setDate(tmp1, tmp, yr);
  while(Serial.read() >= 0) ;
  Serial.println("Please enter the current day of the week, 1-7.");
  Serial.print("1 Mon | 2 Tues | 3 Weds | 4 Thu | 5 Fri | 6 Sat | 7 Sun - ");
  while (Serial.available() == 0) {}
  tmp = Serial.parseInt();
  Serial.println(tmp);
  rtc.setDOW(tmp);
  while(Serial.read() >= 0) ;*/
  while(Serial.read() >= 0) ;
  Serial.print("Please enter the current hour in 24hr format, 0-23. - ");
  while (Serial.available() == 0) {}
  tmp = Serial.parseInt();
  Serial.println(tmp);
  while(Serial.read() >= 0) ;
  Serial.print("Please enter the current minute, 0-59. - ");
  while (Serial.available() == 0) {}
  tmp1 = Serial.parseInt();
  Serial.println(tmp1);
  rtc.setTime(tmp, tmp1, 0);
  Serial.println("The time and date is updated.");
  Serial.println("Enter First mobile no(10 digit)");
  while (Serial.available() == 0) {}
  while(Serial.available() < 9) ;
  String s1 = Serial.readString();
  writeStringToEEPROM(0x00, s1);
  Serial.print("You entered-");
  Serial.println(s1);
  s1="";
  Serial.println("Enter Second mobile no(10 digit)");
  while (Serial.available() == 0) {}
  while(Serial.available() < 9) ;
   s1 = Serial.readString();
  writeStringToEEPROM(0x10, s1); 
  Serial.print("You entered-");
  Serial.println(s1);
  s1="";
  Serial.println("Enter Third mobile no(10 digit)");
  while (Serial.available() == 0) {}
  while(Serial.available() < 9) ;
   s1 = Serial.readString();
  writeStringToEEPROM(0x20, s1);
  Serial.print("You entered-");
  Serial.println(s1);
   while(Serial.read() >= 0) ;
  Serial.println("DAY Time delay (0-59) Minutes - ");
  while (Serial.available() == 0) {}
  tmp1 = Serial.parseInt();
  EEPROM.write(0x30, tmp1);
  Serial.println(tmp1);
   while(Serial.read() >= 0) ;
  Serial.println  ("NIGHT Time delay (0-59) Minutes - ");
  while (Serial.available() == 0) {}
  tmp1 = Serial.parseInt();
  EEPROM.write(0x35, tmp1);
  Serial.println(tmp1);
  day_delay= EEPROM.read(0x30);
   night_delay= EEPROM.read(0x35);
  ss=0;
 minutes=0;
 trials=0;
 sms_bit=0;
}
void SendMessage()
{ int tr;
  tr=mobone.indexOf("x");
  if(tr<0)
  {
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.print("AT+CMGS=\"+91");
  mySerial.print(mobone);
  mySerial.println("\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.print("The gatemen is not attentive at LC gate 179T. Please contact him imediately");// The SMS text you want to send
  delay(100);
  mySerial.print((char)26);// ASCII code of CTRL+Z
  delay(4000);
  }
  tr=mobtwo.indexOf("x");
  if(tr<0)
  {
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.print("AT+CMGS=\"+91");
  mySerial.print(mobtwo);
  mySerial.println("\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.print("The gatemen is not attentive at LC gate 179T. Please contact him imediately");// The SMS text you want to send
  delay(100);
  mySerial.print((char)26);// ASCII code of CTRL+Z
  delay(3000);
  }
  tr=mobthree.indexOf("x");
  if(tr<0)
  {
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.print("AT+CMGS=\"+91");
  mySerial.print(mobthree);
  mySerial.println("\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.print("The gatemen is not attentive at LC gate 179T. Please contact him imediately");// The SMS text you want to send
  delay(100);
  mySerial.print((char)26);// ASCII code of CTRL+Z
  delay(3000);
  }
}
