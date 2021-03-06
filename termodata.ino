//library for LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//library for Card
#include <SPI.h>
#include <SD.h>
File myFile;
//char* inStringus = "";
int inStringus = 0;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);



// set sensor
const int  buttonPin3 = 5;
// set relay


int WorkMode = 1;

// set buttons
const int buttonPin2 = 7;
const int buttonPin1 = 6;
const int buttonPin0 = 5;

// set variables for
int buttonSensor = 0;


int variableTemperaturePlus = 30;

// set variables for buttons
int buttonSelect = 0;
int buttonPlus = 0;
int buttonMinus = 0;

int modecount = 0;
int minutecount = 0;

unsigned long lastDebounceTime = 0;
unsigned long currentDebounceTime = 0;
unsigned long debounceDelay = 50; 

///
String variableContainer = "";
int SDVariableContainer = 0;



char* modename[] = {"work: ", "AMode.txt", "ATemp.txt", "BMode.txt", "BTemp.txt", "AFTemp.txt", "BFTemp.txt"};
char* humanModename[] = {"work: ", "time", "ts*", "time2", "ts2*", "tf*", "tf2*"};
int modevariable[] = {0, 0, 0, 0, 0, 0, 0};

int saverCount = 1;

// Steinhart–Hart_equation variables
#define THERMISTORPIN A0
// R for 25 degrees
#define THERMISTORNOMINAL 10000

// temp. nominal
#define TEMPERATURENOMINAL 25

// numbers of tops for middle Temperature
#define NUMSAMPLES 5

// beta coefficient (usually 3000-4000)
#define BCOEFFICIENT 3950

// R2
#define SERIESRESISTOR 220

int samples[NUMSAMPLES];
// END of Steinhart–Hart_equation variables

// period betwen ts and t
int goPeriod = 0;


//////
/// void real temperature

void realTemperature(int fromSensorValue){
  /*
  float voltage = fromSensorValue * (5.0 / 1023.0);
  //lcd.print(voltage);
*/

uint8_t i;
float average;
// сводим показания в вектор с небольшой задержкой между снятием показаний
for (i=0; i< NUMSAMPLES; i++) {
samples[i] = fromSensorValue;
delay(10);  
}

// рассчитываем среднее значение
average = 0;
for (i=0; i< NUMSAMPLES; i++) {
average += samples[i];
}
average /= NUMSAMPLES;
/*
Serial.print("Average analog reading ");
Serial.println(average);
*/

// конвертируем значение в сопротивление
average = 1023 / average - 1;
average = SERIESRESISTOR / average;
/*
Serial.print("Thermistor resistance ");
Serial.println(average);
*/
float steinhart;
steinhart = average / THERMISTORNOMINAL; // (R/Ro)
steinhart = log(steinhart); // ln(R/Ro)
steinhart /= BCOEFFICIENT; // 1/B * ln(R/Ro)
steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
steinhart = 1.0 / steinhart; // инвертируем

steinhart -= 273.15; // конвертируем в градусы по Цельсию
//Serial.print("Temperature ");
lcd.print(steinhart);
lcd.print("*C, ");

}
// END of Steinhart–Hart_equation void

// plus/minus


void changeNumber(int action, int modecount) {
// for temperature mode
  if(modecount == 2 
  or modecount == 4
  or modecount == 5
  or modecount == 6
){
  if(action == 1 ){
    modevariable[modecount] = modevariable[modecount] - 1;
    } else if(action == 0){
    modevariable[modecount] = modevariable[modecount] + 1;
      }
    
    }
 else{
  if(action == 1 ){
    modevariable[modecount] = modevariable[modecount] + 5;
    } else if(action == 0){
    modevariable[modecount] = modevariable[modecount] - 5;
      }
    }
    /////////////////////////
     
}
// write to SD cart
void dataWriter(){
// debug monitoring
Serial.print("lastDebounceTime: ");
Serial.println(lastDebounceTime);
Serial.print("millis: ");
Serial.println(millis());
Serial.print("debounceDelay: ");
Serial.println(debounceDelay);

  if ((millis() - lastDebounceTime) > debounceDelay) {
      
            SD.remove(modename[modecount]);
            myFile = SD.open(modename[modecount] , FILE_WRITE);
        
            // if the file opened okay, write to it:
        
            if (myFile) {
              Serial.print(modevariable[modecount]);
              Serial.print("Writing to test.txt...");
        
              myFile.print(modevariable[modecount]);
              // close the file:
              myFile.close();
              Serial.println("done.");
              Serial.println(millis());
            } else {
              // if the file didn't open, print an error:
              Serial.println("error opening test.txt");
            }

            // set last Debounce
          lastDebounceTime = millis();  
      }
}


    /////////////////////
// show to LCD
void LCDShow(){
    lcd.clear();
    lcd.backlight();
    lcd.print(humanModename[modecount]);
    lcd.print(": ");
    lcd.backlight();
    if(modecount == 2 
    or modecount == 4
    or modecount == 5 
    or modecount == 6){
     realTemperature(modevariable[modecount]);
      }else{
    lcd.print(modevariable[modecount]);
      }

    //////////////////////////////////

    delay(500);
  }
/// end of void
///////////

//////
// counter void
void counterus(int currentTimeVariable){
minutecount++;
        if (minutecount == 60) {
          minutecount = 0;
          /////////////////////
          // remove  function
          modevariable[currentTimeVariable] = modevariable[currentTimeVariable] - 1;
          SD.remove(modename[currentTimeVariable]);
          myFile = SD.open(modename[currentTimeVariable] , FILE_WRITE);
          if (myFile) {
            myFile.print(modevariable[currentTimeVariable]);
            // close the file:
            myFile.close();
          }
          ///////////////////
        }
}
// end of counter void

void setup()
{
  // initialize the LCD
  lcd.begin();
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);

  if (!SD.begin(10)) {
    Serial.println("initialization failed -- not so good!");
    return;
  }
  Serial.println("initialization done.");

  /////////////////////////////////
  /// Saver function
  /////////////////////////////////



  // re-open the file for reading:
  for (saverCount = 1; saverCount < 7; saverCount++) {
    variableContainer = "";
    myFile = SD.open(modename[saverCount]);
    if (myFile) {
      // read from the file until there's nothing else in it:
      while (myFile.available()) {

        variableContainer = myFile.readStringUntil('\n');
      }
      SDVariableContainer = variableContainer.toInt();
      modevariable[saverCount] = SDVariableContainer;
      Serial.print(modevariable[saverCount]);
      Serial.println('-');
      

      // set saved data to loop

    }

  }

  Serial.println(analogRead(A0));
  //////////////////////////////

  pinMode(9, OUTPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin0, INPUT);

  // Turn on the blacklight and print a message.
  lcd.clear();
  lcd.backlight();
  lcd.print(humanModename[modecount]);

}



void loop()
{

  buttonSelect = digitalRead(buttonPin2);
  buttonPlus = digitalRead(buttonPin1);
  buttonMinus = digitalRead(buttonPin0);

  int sensorValue = analogRead(A0);
  int termocounter = 0;

  /**
    main loop
  */



  if ((WorkMode == 0) ) {
    //Serial.println(WorkMode);
    // disable relay
    digitalWrite(9, HIGH);
  }  else {
    

        
    // detect Time
    //////////
    // first mode
    //////////
    if (modevariable[1] >= 1) {
      ///////////////////////
      /// visual part
      ///////////////////////
      lcd.clear();
      lcd.backlight();
      lcd.print("A: ");
      realTemperature(sensorValue);

      lcd.backlight();
      lcd.print(modevariable[1]);


      ///////////////////////
      //sensor part

      if ((modevariable[5] < sensorValue) && goPeriod == 0 ) {
       
        //enable relay
        digitalWrite(9, LOW);
        /*
        Serial.println("go heart");
        Serial.print(modevariable[2]);
        Serial.print("/");
        Serial.println(modevariable[5]);
        Serial.println(sensorValue);
        */
        if(modevariable[2] > sensorValue){
        // counter void
        counterus(1);
        // end counter void
        }
      Serial.println("go heart");
      }
      ////////////
      else  {
      Serial.println("WorkMode");
        
        goPeriod = 1;
        if (modevariable[2] < sensorValue){
        goPeriod = 0;
        Serial.println("!goPeriod = 0");
        }
        // disable relay
        digitalWrite(9, HIGH);
        // start counter
        // counter void
        counterus(1);
        // end counter void
      }

    }
  // end of first mode

    //////////
    // second mode
    //////////
else if (modevariable[3] >= 1) {
      ///////////////////////
      /// visual part
      ///////////////////////
      lcd.clear();
      lcd.backlight();
      lcd.print("B: ");
      realTemperature(sensorValue);

      lcd.backlight();
      lcd.print(modevariable[3]);


      ///////////////////////
      //sensor part

      if ((modevariable[6 ] < sensorValue) && goPeriod == 0 ) {
       
        //enable relay
        digitalWrite(9, LOW);
        /*
        Serial.println("go heart");
        Serial.print(modevariable[2]);
        Serial.print("/");
        Serial.println(modevariable[5]);
        Serial.println(sensorValue);
        */
        // start counter during relay on
        if(modevariable[4] > sensorValue){
        // counter void
        counterus(3);
        // end counter void
        }
        
      Serial.println("go heart");
      }
      else  {
      Serial.println("WorkMode");
        goPeriod = 1;
        if (modevariable[4] < sensorValue){
        goPeriod = 0;
        Serial.println("!goPeriod = 0");
        }
        // disable relay
        digitalWrite(9, HIGH);
        // start counter
        // counter void
        counterus(3);
        // end counter void

      }

    }
  // end of second mode
// if time gone
else {
      lcd.clear();
      //lcd.backlight();
      lcd.print("Done! t: ");
      realTemperature(sensorValue);

      // disable relay
        digitalWrite(9, HIGH);
  }
    // detect Time
    // Second mode

    //////////////////////
    delay(1000);
  }

  // end of main loop
  if (buttonSelect == HIGH) {
    modecount++;
    //reset mode name
    if (modecount == 7) {
      modecount = 0;
      WorkMode = 1;
    } else {
      WorkMode = 0;
    }

    lcd.clear();
    lcd.backlight();
    
    
    if(modecount == 2 
    or modecount == 4
    or modecount == 5
    or modecount == 6){
      lcd.print(humanModename[modecount]);
      lcd.print(": ");
      lcd.backlight();
     realTemperature(modevariable[modecount]);
      }else{
    lcd.print(humanModename[modecount]);
    lcd.print(": ");
    lcd.backlight();
    lcd.print(modevariable[modecount]);
      }
    delay(500);
  }
// end of mode select

  // plus
  if (buttonPlus == HIGH) {
    changeNumber(1, modecount);
    dataWriter();
    LCDShow();
  }
  // minus
  if (buttonMinus == HIGH) {
    changeNumber(0, modecount);
    dataWriter();
    LCDShow();
  }
  else {
    lcd.backlight();
  }
  // Do nothing here...
}
