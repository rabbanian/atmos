/*
    Copyright (c) 2017 Hamidreza Rabbanian

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

//using built-in librarys for working with LCD and KEYPAD
#include <LiquidCrystal.h>
#include <Keypad.h>
//declaring LCD pin-map to the Arduino
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
//defining required arrays for Keypad lib
char hexaKeys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[4] = {7, 6, 5, 4};
byte colPins[4] = {3, 2, 1, 0};
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, 4, 4);

//temporarily holding key that has been pressed.
char key;
//sensor vaules
float sensors[3] = {0, 0, 0};
//for calculating Most-Far value of sensors from Average
float distence[3] = {0, 0, 0};
//high and low threshold value
int highTemp = 30;
int lowTemp = 5;
//password that user have to insert before going into configuration mode
char password[16] = "123467";
//used for menu control
boolean iteration = false;
boolean action = true;

void setup() {
  lcd.begin(16, 2);
}

void loop() {
   //reading sensors information and converting them into temperature based on a simple formula
   sensors[0] = (analogRead(0) / 16) - 15;
   sensors[1] = (analogRead(1) / 16) - 15;
   sensors[2] = (analogRead(2) / 16) - 15;
   //calculating average temperature and sensors variation from it
   float average = (sensors[0] + sensors[1] + sensors[2]) / 3;
   distence[0] = abs(average - sensors[0]);
   distence[1] = abs(average - sensors[1]);
   distence[2] = abs(average - sensors[2]);
   //detecting maximum variation
   float falseSensor = max(max(distence[0],distence[1]), distence[2]);
   //if this maximum variation is too much then detect which sensors is for, then exclude it's value from average temperature
   if(falseSensor > 10)
   {
      if(falseSensor == distence[0])
	 average -= sensors[0]/3;
      else if(falseSensor == distence[1])
	 average -= sensors[1]/3;
      else if(falseSensor == distence[2])
	 average -= sensors[2]/3;

      average = (average * 3) / 2;
   }
   //ptint appropriate description
   lcd.setCursor(0, 0);
   if(average > highTemp)
      lcd.print("Temp High       ");
   else if(average < lowTemp)
      lcd.print("Temp Low        ");
   else
      lcd.print("Temp OK         ");
   //print actual temperature value (which is average of two or maybe three sensors value)
   lcd.setCursor(0, 1);
   lcd.print(average);
   //check if it's needed to Configuring settings (with the press of the 'A' key by the user) if yes then go and set it
   Configuration();
}

void Configuration()
{
   //Configuration mode activated by A key-press so it's check this event every loop cycle
   key = keypad.getKey();
   if (key == 'A')
   {
     //so we are in Configuration mode first we need to check password before going to setting mode, AccessControl takes care of it
      if(AccessControl())
        do {
          iteration = false;
          //it's within a loop (terminated by iteration variable) beacause maybe user wants to set more than one option at a time
          Settings();
        } while(iteration);
   }
}

boolean AccessControl()
{
   //user input characters saved in this variable
   char input[16] = {0};
   lcd.clear();
   lcd.print("Enter Password:");
   //waitForKey is a blocking method, so program execution stops here until user press some key
   key = keypad.waitForKey();
   lcd.setCursor(0, 1);
   //getting 16 character from user as password (the last character is not '\0' so we don't need a extra cell)
   for(int i = 0; i < 16; i++)
   {
      // 'D' means Enter, we shall not include it into input characters
      if(key == 'D')
        break;
      //as the prompt says if 'A' or 'C' been pressed it means we must cancel the process of getting input and back to the temperature monitoring mode
      if(key == 'A' || key == 'C')
      {
        lcd.clear();
        lcd.print("Canceled!");
        delay(1500);
        return false;
      }
      lcd.print("*");
      input[i] = key;
      key = keypad.waitForKey();
   }
   //we have to check password character by character in order (if it was right set isEqual true)
   boolean isEqual = true;
   for(int i = 0; i < 16; i++)
      if(input[i] != password[i])
	    {
  	    isEqual = false;
  	    break;
	    }

   //prints appropriate prompt
   lcd.clear();
   if(isEqual)
   {
      lcd.print("Access Granted!");
      delay(1000);
      return true;
   } else
   {
      lcd.print("Access Denied!");
      delay(1500);
      return false;
   }
}
//this is where user can change value of high and low thresholds
void Settings()
{
   //action determines whether the value passed by getInput function must be used or not
   action = true;
   lcd.clear();
   lcd.print("Options:");
   lcd.setCursor(0, 1);
   lcd.print("B, C");
   key = keypad.waitForKey();
   lcd.clear();
   if(key == 'B')
   //it's within a loop beacause user maybe inserts Invalid inputs, in this case we shall continue the loop until we get valid input
    while(setB(getInput("Upper Range:")));
   else if(key == 'C')
    while(setC(getInput("Lower Range:")));
   else if(key == 'A')
    {
      lcd.print("Canceled!");
      delay(1500);
      return;
    }
   else
   {
     lcd.print("Invalid Option!");
     lcd.setCursor(0, 1);
     lcd.print("A for Cancel.");
     delay(1500);
     iteration = true;
   }

}
//this function gets a input from user (up to 5 character including dot) and converts it into float data-type
//it also manages some of operations need to take in order to cancel or clear the input
float getInput(const char * prompt)
{
  //we need to remember that this input is Negative or Positive, right?
  boolean isNegative = false;
  char input[6] = "0";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);
  lcd.setCursor(0, 1);
  int i = 0;
  for(; i < 6; i++)
  {
    key = keypad.waitForKey();
    switch(key)
    {
      case 'A':
        lcd.clear();
        lcd.print("Canceled!");
        delay(1500);
        //since we are in a for loop and switch statement we can't just break beacause we're only breaking from switch case not from for loop
        //assigning i to a value greater than its limit causes loop to end, and it's also a code name to remember from witch state we're ending loop
        //i.e. is it Canceled? or, Conformed? or just user cleared operation (Not Saved)
        i = 9;
        continue;
      case 'B':
        lcd.clear();
        lcd.print("Not Saved!");
        delay(1500);
        i = 10;
        continue;
      case 'C':
        lcd.clear();
        lcd.print("Invalid Key!");
        lcd.setCursor(0, 1);
        lcd.print("Try Again");
        delay(1500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(prompt);
        lcd.setCursor(0, 1);
        for (int j = 0; j < i; j++) {
          lcd.print(input[j]);
        }
        i--;
        continue;
      case 'D':
        i = 11;
        continue;
      case '#':
        lcd.setCursor(0, 1);
        lcd.print("       ");
        lcd.setCursor(0, 1);
        //toggle negativeness
        isNegative = !isNegative;
        if(isNegative)
          lcd.print("-");
        for (int j = 0; j < i; j++) {
          lcd.print(input[j]);
        }
        i--;
        continue;
      case '*':
        //replacing '*' char with '.' char means * key in the keypad inserts a '.' character
        key = '.';
      break;
    }
    //this is where our loop has ended but we want to continue it beacause only A key (Enter button) stops it
    if(i == 5){
      i--;
      continue;
    }
    input[i] = key;
    lcd.print(key);
  }
  //based on which value i takes we can decide what to do with user's input
  switch (i) {
    case 10:
      //user Canceled, so we need to inform next method to not use this float value returned by this method
      action = false;
      break;
    case 11:
      //user wants to clear the operation but also wnats to back in settings mode, iteration is set to achieve that
      action = false;
      iteration = true;
      break;
    case 12:
      //in this case everything is OK, we can use the input, user Conformed it
      action = true;
      //the actual convertion takes place in this method
      float temp = convertToFloat(input);
      return isNegative ? temp * -1 : temp;
  }
  return 0;
}
//setB and setC are method that checking requirements for each option
boolean setB(const float input)
{
  lcd.clear();
  if(action == false)
  return false;
  if(input > 48.00 || input < 20.00)
  {
    lcd.print("Out Of Range!");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(1500);
    return true;
  } else
  {
    lcd.print("All Saved!");
    delay(1000);
    highTemp = input;
    return false;
  }
}

boolean setC(const float input)
{
  lcd.clear();
  if(action == false)
  return false;
  if(input > 10.00 || input < -14.00)
  {
    lcd.print("Out Of Range!");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(1500);
    return true;
  }else
  {
    lcd.print("All Saved!");
    delay(1000);
    lowTemp = input;
    return false;
  }
}

float convertToFloat(const char input[])
{
  int precision = 4;
  int value = 0;
  int i;
  for (i = 0; i < 6; i++) {
    if(input[i] == '.')
    {
      precision = i;
      continue;
    } else if(input[i] == '\0')
      break;
    value *= 10;
    value += int(input[i]) - int('0');
  }
  if(i != 5 && precision == 4)  precision = i - 1;
  precision = i - precision - 1;
  return (float) value / pow(10, precision);
}
