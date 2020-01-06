#include <Wire.h>
#include <EEPROM.h>
#include <Keypad.h>                 // Khai báo thư viện Keypad
#include <LiquidCrystal_I2C.h>      // Khai báo thư viện LCD sử dụng I2C
LiquidCrystal_I2C lcd(0x3f, 16, 2); // 0x27 địa chỉ LCD, 16 cột và 2 hàng
              
#define RELAY     12
#define GREEN     11
#define RED       10
#define startBtn  9

const byte ROWS = 4; // Bốn hàng
const byte COLS = 3; // Ba cột

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {8, 7, 6};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

byte customChar[] = {
  B11000,
  B01100,
  B00110,
  B00011,
  B00011,
  B00110,
  B01100,
  B11000
};
byte customArrow1[] = {
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000
};
byte customArrow2[] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000
};
/**********************************************Variable*******************************************/
byte str3[4] = {0,0,0,0};
byte upStr4[4] = {0,0,0,0};
byte lowStr4[4] = {0,0,0,0};
unsigned long checksum = 0;
int count=-1, addr=0;
bool typing=0;
unsigned long dem;
unsigned int error;
/**********************************************Function*******************************************/
short Char2Int(char x){
  switch(x){
    case '0':
      return 0;
      break;
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
    case '*':
      return 10;
      break;
    case '#':
      return 11;
      break;
  }
}
void sendInterface(){
  //lcd.setCursor(0, 1);
  //lcd.print("Any keys to send ...");
  //keypad.waitForKey();
  //lcd.setCursor(0,1);
  lcd.clear();
  lcd.print("Sending");
  for (int i=0;i<=15;i++){
    lcd.setCursor(i,1);
    lcd.write(byte(0));
    delay(50); 
  }
  lcd.setCursor(0,0);
  lcd.print("Thresh: ");
  delay(100);
  lcd.setCursor(0,1);
  lcd.print("                ");
  
  count=-1;
}
void ReadPi(String buff){
  if(buff=="PASS"){
    lcd.clear();
    //digitalWrite(RELAY,1);
    lcd.setCursor(9,1);
    lcd.print("PASS");
    digitalWrite(RED,1);
    digitalWrite(GREEN,0);
  }
  else if (buff=="SPEED"){
    lcd.clear();
    //digitalWrite(RELAY,1);
    lcd.setCursor(0,1);
    lcd.print("SPEED OK");
    digitalWrite(RED,0);
    digitalWrite(GREEN,1); 
  }
  else if (buff=="RESEND"){
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("ERROR DETECTED!!");
    digitalWrite(RED,0);
    digitalWrite(GREEN,1);
    Serial.write(str3,3);
    Serial.write(0);
    Serial.write(lowStr4,4);
    Serial.write(upStr4,4);
    Serial.write(checksum);
  }
}
void delScreen(char key){
  if (key == '#'){
    count=-1;
    lcd.clear();
    lcd.print(" Enter Thresh");
  }
}
void displayVal(String num1, String num2, String num3){
  lcd.clear();
  lcd.print("Thresh: ");
  lcd.print(num1);
  lcd.setCursor(0, 1);
  lcd.print("Low:");
  lcd.print(num2);
  lcd.print(" Up:");
  lcd.print(num3);
}
/********************************************SETUP**************************************************/
void setup(){
  Serial.begin(9600);
  //Serial.setTimeout(100);
  while(!Serial); //true if serial port available
  Serial.setTimeout(1000); //default

  keypad.setHoldTime(2000);
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad

  pinMode(RELAY,OUTPUT);
  pinMode(GREEN,OUTPUT);
  digitalWrite(GREEN,1); // off
  pinMode(RED,OUTPUT);
  digitalWrite(RED,1); // on
  pinMode(startBtn,INPUT);
  
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  
  //delay(100);
  for(int i=0; i<4; i++)
    str3[i] = EEPROM.read(i);
  for(int i=4; i<8; i++)
    lowStr4[i%4] = EEPROM.read(i);
  for(int i=8; i<12; i++)
    upStr4[i%4] = EEPROM.read(i);
    
  displayVal(String(float(str3[0]*100 + str3[1]*10 + str3[2])/100), String(lowStr4[0]*1000 + lowStr4[1]*100 + lowStr4[2]*10 + lowStr4[3]), String(upStr4[0]*1000 + upStr4[1]*100 + upStr4[2]*10 + upStr4[3]));

  lcd.createChar(0,customChar);
  lcd.createChar(1,customArrow1);
  lcd.createChar(2,customArrow2);

  checksum = str3[0] + str3[1] + str3[2] + upStr4[0] + upStr4[1] + upStr4[2] + upStr4[3] + lowStr4[0]+ lowStr4[1] + lowStr4[2] + lowStr4[3];
  Serial.write(str3,3);
  Serial.write(110); //reset arduino
  Serial.write(lowStr4,4);
  Serial.write(upStr4,4);
  Serial.write(checksum);
  //digitalWrite(RELAY,0);
}
/********************************************LOOP***************************************************/
void loop(){
  if(Serial.available())
    ReadPi(Serial.readStringUntil('\r'));
  
  char key = keypad.getKey(); //Returns the key that is pressed. This function is non-blocking
  if (typing && key){
    ++count;
    if (count==0){      
      if (key=='*' || key=='#')
        --count;
      else {
        str3[0] = Char2Int(key);
        lcd.setCursor(4, 1);
        lcd.print(str3[0]);
      }
    }
    else if (count==1){
      if (key=='*'){
        --count;
        //str3[3] = 1; //dấu phẩy thứ nhất
        lcd.setCursor(5,1);
        lcd.print('.');
      }
      else{
        str3[1] = Char2Int(key);
        lcd.setCursor(6, 1);
        lcd.print(str3[1]);
      }
    }
    else if (count==2){
      if (key == '*')
        --count;
      else{
        str3[2] = Char2Int(key);
        lcd.setCursor(7, 1);
        lcd.print(str3[2]);

        /******************** get ready for 4 digit lower ********************/
        delay(500);
        lcd.clear();
        lcd.print("LOWER SPEED: ");
      }
    }
    else if (count == 3){
      if (key=='*' || key=='#')
        --count;
      else{
        lowStr4[0] = Char2Int(key);
        lcd.setCursor(3,1);
        lcd.print(lowStr4[0]);
      }
    }
    else if (count == 4){
      if (key=='*' || key=='#')
        --count;
      else{
        lowStr4[1] = Char2Int(key);
        lcd.setCursor(4,1);
        lcd.print(lowStr4[1]);
      }
    }
    else if (count == 5){
      if (key=='*' || key=='#')
        --count;
      else{
        lowStr4[2] = Char2Int(key);
        lcd.setCursor(5,1);
        lcd.print(lowStr4[2]);
      }
    }
    else if (count == 6){
      if (key == '*')
        --count;
      else{
        lowStr4[3] = Char2Int(key);
        lcd.setCursor(6,1);
        lcd.print(lowStr4[3]);

        /******************** get ready for 4 digit upper ********************/
        delay(500);
        lcd.clear();
        lcd.print("UPPER SPEED: ");
      }
    }
    else if (count == 7){
      if (key=='*' || key=='#')
        --count;
      else{
        upStr4[0] = Char2Int(key);
        lcd.setCursor(3,1);
        lcd.print(upStr4[0]);
      }
    }
    else if (count == 8){
      if (key=='*' || key=='#')
        --count;
      else{
        upStr4[1] = Char2Int(key);
        lcd.setCursor(4,1);
        lcd.print(upStr4[1]);
      }
    }
    else if (count == 9){
      if (key=='*' || key=='#')
        --count;
      else{
        upStr4[2] = Char2Int(key);
        lcd.setCursor(5,1);
        lcd.print(upStr4[2]);
      }
    }
    else if (count == 10){
      if (key=='*' || key=='#')
        --count;
      else{
        upStr4[3] = Char2Int(key);
        lcd.setCursor(6,1);
        lcd.print(upStr4[3]);

        /************************************* Process Data *************************************/
        delay(500);
        lcd.clear();
        unsigned long val3 = str3[0]*100 + str3[1]*10 + str3[2];
        unsigned long up_val4= upStr4[0]*1000 + upStr4[1]*100 + upStr4[2]*10 + upStr4[3];
        unsigned long low_val4= lowStr4[0]*1000 + lowStr4[1]*100 + lowStr4[2]*10 + lowStr4[3];
        displayVal(String((float)val3/100), String(low_val4), String(up_val4));
        
        /************************************* sending *************************************/
        checksum = str3[0] + str3[1] + str3[2] + upStr4[0] + upStr4[1] + upStr4[2] + upStr4[3] + lowStr4[0]+ lowStr4[1] + lowStr4[2] + lowStr4[3];
        sendInterface(); // waiting for pressed any keys to send ...
        for(int i=0; i<4; i++){
          EEPROM.update(i,str3[i]);
          delay(10);
        }
        Serial.write(str3,3);
        Serial.write(0); // 3 digit flag >> just fill the gap

        for(int i=4; i<8; i++){
          EEPROM.update(i,lowStr4[i%4]);
          delay(10);
        }
        Serial.write(lowStr4,4);

        for(int i=8; i<12; i++){
          EEPROM.update(i,upStr4[i%4]);
          delay(10);
        }
        Serial.write(upStr4,4);

        Serial.write(checksum);

        displayVal(String((float)val3/100), String(low_val4), String(up_val4));

        typing = 0;
      }
    }
  }
}

void keypadEvent(KeypadEvent key){
  switch (keypad.getState()){
  case HOLD:
    if (key == '#'){
      typing = 1;
      // get ready for thresh
      lcd.clear();
      lcd.print("ENTER THRESH: ");
    }
    if (key=='*'){
      lcd.clear();
      lcd.print("Detecting USB...");
      Serial.write(str3,3);
      Serial.write(111); // command export .xlsx file
      Serial.setTimeout(5000); // wait 5s
      // check valid usb id
      if(Serial.find("USB")){
        lcd.clear();
        lcd.print("Copying ...");
        Serial.setTimeout(500);
        bool stateGREEN = 0;
        // waiting done copying
        while(!Serial.find("usb")){
          digitalWrite(GREEN,stateGREEN);
          digitalWrite(RED,1);
          stateGREEN = !stateGREEN;
        }
        lcd.clear();
        lcd.print("DONE");
        lcd.setCursor(0,1);
        lcd.print("REJECT USB");
      }
      else{
        lcd.setCursor(1,1);
        lcd.print("ACCESS DENIED");
        digitalWrite(RED,0);digitalWrite(GREEN,1);
      }
    }
    break;
  case PRESSED:
    if (key=='#')
      if(typing){
        if (count<3){
          count=-1;
          lcd.clear();
          lcd.print("ENTER THRESH");
        }
        else if (count<7){
          count=2;
          lcd.clear();
          lcd.print("LOWER SPEED");
        }
        else{
          count=6;
          lcd.clear();
          lcd.print("UPPER SPEED");
        }
      }
    break;
  }
}
