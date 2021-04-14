/*
/ Carl Chuck Sandin
/ April 2021
*/

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
// these two libraries are installed for the
// LCD and gives us functions to manipulate it.
LiquidCrystal_I2C lcd(0x27, 16, 2);

/*//////////////////////////////////////////////////////////////////////*/
// The 74HC595 micro-controller will be used as a shift register
// to 'load dice' into our LED's, as the UNO has a small amount of
// available data pins. Data is our write, clock and latch is to
// cycle bits in.
const int data = 2;
const int clock = 3;
const int latch = 4;
/*//////////////////////////////////////////////////////////////////////*/
//Pin declarations for input.
const int shiftButton = 5;
const int submitButton = 8;
const int potPin = 0;

//Pins for the RGB LED light.
const int redPin = 11;
const int greenPin = 10;
const int bluePin = 9;
/*//////////////////////////////////////////////////////////////////////*/
// This series of variables is to manipulate the shift register.
// We have 6 LEDs so we use 2 bit arrays and 2 mask arrays to select
// which LED is being used.
int ledState = 0;
int bits[] = {B000001, B000010, B000100, B001000, B010000, B100000};
int masks[] = {B111110, B111101, B111011, B110111, B101111, B011111};
/*//////////////////////////////////////////////////////////////////////*/
// Arrays for all dice rolls. Since there are only 7 dice in a DnD set,
// we only need an array of size 7. roll Values has one extra for the total.

int numDice[7];
int diceType[7];
char modType[7];
int modNum[7];
int rollValues[8];

// global index that the functions can manipulate and store
// the appropriate dice selections and rolls.
int index = 0;

// lets the program know it's time to calculate rolls.
bool finish = false;
//tells us if the user has run the program once already.
bool initFlag = false;

/*//////////////////////////////////////////////////////////////////////*/
void setup()
{
  //boot up the LCD display.
  lcd.init();
  lcd.backlight();

  // this block of declarations is essential
  // for the pins to work. we map them 
  // based on their input/output function.
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(shiftButton, INPUT);
  pinMode(submitButton, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  //turn off the RGB LED on start up.
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  // This is for the dice roll to be as close
  // to random as possible by pulling in a signal
  // from an empty pin.
  randomSeed(analogRead(A1));
  //turn LED's off on startup
  updateLEDs(0);

  //Not essential, but good for debugging.
  Serial.begin(9600);



}
/*//////////////////////////////////////////////////////////////////////*/
void loop()
{
  // Run until the user says they are done
  // or we hit 7 dice loaded.
  while (finish == false &&  index < 7)
  {
    // first time run, display a message.
    if (initFlag == false) {
      lcdInitMessage();
    }
    
    diceAmount();
    diceSelect();
    modifiers();
    rollAgain();
  }
  rollAll();
  lcdFinalMessage();
}

/*//////////////////////////////////////////////////////////////////////*/
// This function is just to have a little lightshow if
// the user rolls critical high or low. 
// it uses the updateLED function and sends a binary string
// to change the LED's. the 1 is the position in the LED row.

void lightShow() {

  updateLEDs(B000001);
  delay(100);
  updateLEDs(B000010);
  delay(100);
  updateLEDs(B000100);
  delay(100);
  updateLEDs(B001000);
  delay(100);
  updateLEDs(B010000);
  delay(100);
  updateLEDs(B100000);
  delay(100);
  
  updateLEDs(B100000);
  delay(100);
  updateLEDs(B010000);
  delay(100);
  updateLEDs(B001000);
  delay(100);
  updateLEDs(B000100);
  delay(100);
  updateLEDs(B000010);
  delay(100);
  updateLEDs(B000001);
  delay(100);

  //after the light show, turn all of them off.
  for (int i = 0; i < 6; i++)
  {
    changeLED(i, LOW);
  }

}
/*//////////////////////////////////////////////////////////////////////*/
// this checks if the user is ready to roll again. If not, it will
// simply display the total value until they are ready to continue.
void lcdFinalMessage() {
  
  int submitState = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ROLL COMPLETE!");
  lcd.setCursor(0, 1);
  lcd.print("ROLL AGAIN?");
  delay(3000);
  
  do {
    int potRead = analogRead(potPin);
    if (potRead > 512)
    {
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("NO");
      delay(100);
      digitalWrite(bluePin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
    }
    else
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("YES");
      delay(100);
      digitalWrite(redPin, LOW);
      digitalWrite(bluePin, LOW);
      digitalWrite(greenPin, HIGH);
    }

    if (digitalRead(submitButton) == HIGH)
    {
      submitState++;
      if (potRead > 512)
      {
        do {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("TOTAL: ");
          lcd.setCursor(7, 0);
          lcd.print(rollValues[7]);
          lcd.setCursor(0, 1);
          lcd.print("SUBMIT IF READY");
          delay(200);
          digitalWrite(redPin, LOW);
          digitalWrite(bluePin, HIGH);
          digitalWrite(greenPin, LOW);
        } while (digitalRead(submitButton) == LOW);
        finish = false;
        index = 0;
      }
      else
      {
        finish = false;
        index = 0;
      }

      delay(250);
    }
  } while (submitState == 0);

  digitalWrite(redPin, LOW);
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
}
/*//////////////////////////////////////////////////////////////////////*/
void rollAll() {

  int eachRoll = 0;
  int toStore = 0;
  int dieType = 0;
  int grandTotal = 0;
  int submitState = 0;
  int shiftState = 0;

  // initial condition, or if it is only one die being rolled.
  if (index == 0)
  {
    // pull the dice type.
    dieType = diceType[0];
    // roll the defined amount by the user (i.e 2d6)
    for (int j = 0; j < numDice[0]; j++)
    {
      // roll randomly between 1-x. plus 1 is needed for this function
      // as it rolls from (x,y-1).
      eachRoll = random(1, (dieType + 1));
      
      // this conditional statement is for the critical failures and successes.
      // makes a lightshow if it does.
      
      if (eachRoll == 1)
      {
        lightShow();
        digitalWrite(bluePin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(redPin, HIGH);

      }
      else if (eachRoll == dieType)
      {
        lightShow();
        digitalWrite(bluePin, LOW);
        digitalWrite(greenPin, HIGH);
        digitalWrite(redPin, LOW);
      }
      else
      {
        digitalWrite(bluePin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(redPin, LOW);
      }

      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print(eachRoll);
      delay(2000);


      // add the modifiers to each roll based on (p)lus or minus.
      if (modType[0] == 'p')
      {
        eachRoll += modNum[0];
      }
      else
      {
        eachRoll -= modNum[0];
      }


      toStore += eachRoll;
    }
    //store the total.
    rollValues[7] = toStore;
  }
  else
  {//the structure will be a bit different
    // with multiple dice rolls, hence the else.
    for (int i = 0; i <= index; i++)
    {
      Serial.println(index);
      Serial.println(i);
      dieType = diceType[i];
      toStore = 0;

      for (int j = 0; j < numDice[i]; j++)
      {
        eachRoll = random(1, (dieType + 1));

        if (modType[i] == 'p')
        {
          eachRoll += modNum[i];
        }
        else
        {
          eachRoll -= modNum[i];
        }

        if (eachRoll == 1)
        {
          lightShow();
          digitalWrite(bluePin, LOW);
          digitalWrite(greenPin, LOW);
          digitalWrite(redPin, HIGH);
        }
        else if (eachRoll == dieType)
        {
          lightShow();
          digitalWrite(bluePin, LOW);
          digitalWrite(greenPin, HIGH);
          digitalWrite(redPin, LOW);
        }
        else
        {
          digitalWrite(bluePin, LOW);
          digitalWrite(greenPin, LOW);
          digitalWrite(redPin, LOW);
        }

        lcd.clear();
        lcd.setCursor(7, 0);
        lcd.print(eachRoll);
        delay(2000);

        toStore += eachRoll;

      }
      rollValues[i] = toStore;
      grandTotal += toStore;
    }
    rollValues[7] = grandTotal;
    // with all the values, we want the user to be able to see all values
    // before continuing.
    for (int i = 0; i <= index; i++)
    {
      do {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ROLL ");
        lcd.setCursor(5, 0);
        lcd.print(i + 1);
        lcd.setCursor(6, 0);
        lcd.print(":");
        lcd.setCursor(8, 0);
        lcd.print(rollValues[i]);
        lcd.setCursor(0, 1);
        lcd.print("R FOR NEXT");
        delay(300);
        if ( digitalRead(shiftButton) == HIGH)
        {
          shiftState++;
        }
      } while (shiftState == 0);
    }
  }
  // wait for the user to say they want to continue.
  do {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TOTAL: ");
    lcd.setCursor(7, 0);
    lcd.print(rollValues[7]);
    lcd.setCursor(0, 1);
    lcd.print("SUBMIT IF READY");
    delay(200);
    digitalWrite(bluePin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);

    if (digitalRead(submitButton) == HIGH)
    {
      submitState++;
    }
    
  } while (submitState == 0);
  
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
}
/*//////////////////////////////////////////////////////////////////////*/
// This function just simply asks if the user wants to roll more dice
// before they  do the final roll.
void rollAgain() {

  int submitState = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DO YOU WANT TO");
  lcd.setCursor(0, 1);
  lcd.print("ROLL MORE DICE?");
  delay(3000);

  do {
    int potRead = analogRead(potPin);
    if (potRead > 512)
    {
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("NO");
      delay(100);
      digitalWrite(bluePin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
    }
    else
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("YES");
      delay(100);
      digitalWrite(redPin, LOW);
      digitalWrite(bluePin, LOW);
      digitalWrite(greenPin, HIGH);
    }

    if (digitalRead(submitButton) == HIGH)
    {
      submitState++;
      if (potRead > 512)
      {
        finish = true;
      }
      else
      {
        index++;
      }

      delay(250);
    }
  } while (submitState == 0);

  digitalWrite(redPin, LOW);
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
}

/*//////////////////////////////////////////////////////////////////////*/
// the initial display message for the user to tell them the button positions.
void lcdInitMessage() {
  int submitState = 0;
  lcd.setCursor(0, 0);
  lcd.print("SUBMIT BUTTON");
  lcd.setCursor(0, 1);
  lcd.print("IN MIDDLE");
  delay(3000);
  lcd.clear();

  
  do {
    lcd.setCursor(0, 0);
    lcd.print("PRESS SUBMIT");
    lcd.setCursor(0, 1);
    lcd.print("TO BEGIN");
    // if button is pressed and reads 5V.
    if (digitalRead(submitButton) == HIGH)
    {
      // init flag is now true to say the user has started.
      submitState++;
      initFlag = true;
      delay(250);
    }
  } while (submitState == 0);

  lcd.clear();
}
/*//////////////////////////////////////////////////////////////////////*/
/* this combines all the other functions - diceAmount(), diceSelect() -
    and uses them to select a modifier tree for the dice rolls.
    This means +X, -X where x is an integer, and applies to all dice rolls.
*/
void modifiers() {

  int submitState = 0;
  int load = 0;

  char typeOfMod;

  lcd.clear();
  // only need to explain the first time
  if ( index == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("DICE MODS");
    lcd.setCursor(0, 1);
    lcd.print("USE BOTH INPUTS");
    delay(3000);
    lcd.clear();
  }

  do {
    int potRead = analogRead(potPin);
    if (potRead > 512)
    {
      lcd.setCursor(6, 0);
      lcd.print("MINUS");
      typeOfMod = 'm';
      delay(100);
      digitalWrite(bluePin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
    }
    else
    {
      lcd.setCursor(5, 0);
      // we print a blank to overwrite the old MINUS
      // as it will remain on the screen.
      lcd.print("      ");
      lcd.setCursor(7, 0);
      lcd.print("PLUS");
      typeOfMod = 'p';
      delay(100);
      digitalWrite(redPin, LOW);
      digitalWrite(bluePin, LOW);
      digitalWrite(greenPin, HIGH);
    }


    lcd.setCursor(7, 1);
    lcd.print(load);

    if (digitalRead(shiftButton) == HIGH)
    {
      if (load < 6)
      {
        changeLED(load, HIGH);
        delay(200);
        load += 1;
        lcd.setCursor(7, 1);
        lcd.print(load);
      }
      else
      {
        //Wraps our LED strip around if it goes past six.
        for (int i = 0; i < 6; i++)
        {
          changeLED(i, LOW);
        }
        load = 0;
        delay(200);
      }
    }
    if (digitalRead(submitButton) == HIGH)
    {
      submitState++;
      modType[index] = typeOfMod;
      modNum[index] = load;
      delay(250);
    }
  } while (submitState == 0);

  for (int i = 0; i < 6; i++)
  {
    changeLED(i, LOW);
  }
  digitalWrite(redPin, LOW);
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
}

/*//////////////////////////////////////////////////////////////////////*/
// Using a potentiometer reading in bits - 0 to 1023,
// to select which dice is going to be used for the roll.
void diceSelect() {

  int submitState = 0;
  int diceNum = 0;

  lcd.clear();
  // only need to explain the first time
  if (index == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("USE POT DIAL");
    lcd.setCursor(0, 1);
    lcd.print("TO CHOOSE DIE");
    delay(3000);
  }
  do {
    int potRead = analogRead(potPin);
    if (potRead < 146)
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D4");
      diceNum = 4;
      delay(100);
    }
    else if (potRead >= 146 && potRead < 292)
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D6");
      diceNum = 6;
      delay(100);
    }
    else if (potRead >= 292 && potRead < 438)
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D8");
      diceNum = 8;
      delay(100);
    }
    else if (potRead >= 438 && potRead < 584 )
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D10");
      diceNum = 10;
      delay(100);
    }
    else if (potRead >= 584 && potRead < 730)
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D12");
      diceNum = 12;
      delay(100);
    }
    else if (potRead >= 730 && potRead < 876)
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D20");
      diceNum = 20;
      delay(100);
    }
    else if (potRead >= 876)
    {
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("D100");
      diceNum = 100;
      delay(100);
    }
    if (digitalRead(submitButton) == HIGH)
    {
      submitState++;
      diceType[index] = diceNum;
      delay(250);
    }
  } while (submitState == 0);
}
/*//////////////////////////////////////////////////////////////////////*/
void diceAmount() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HOW MANY DICE?");
  lcd.setCursor(0, 1);
  lcd.print("USE RIGHT BUTTON");

  int submitState = 0;
  int load = 0;

  do {
    if (digitalRead(shiftButton) == HIGH)
    {
      if (load < 6)
      {

        changeLED(load, HIGH);
        delay(250);
        load += 1;
      }
      else
      {
        //Resets our Shift Register LED row.
        for (int i = 0; i < 6; i++)
        {
          changeLED(i, LOW);
        }
        load = 0;
        delay(250);
      }
    }
    // can't roll 0 dice! this is a simple defense mechanism.
    if (digitalRead(submitButton) == HIGH)
    {
      if (load != 0)
      {
        submitState++;
        numDice[index] = load;
        delay(250);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("NEED ATLEAST");
        lcd.setCursor(0, 1);
        lcd.print("1 DIE!");
        delay(3000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HOW MANY DICE?");
        lcd.setCursor(0, 1);
        lcd.print("USE RIGHT BUTTON");
      }
    }

  } while (submitState == 0);
  // Clear Row again.
  for (int i = 0; i < 6; i++)
  {
    changeLED(i, LOW);
  }
  lcd.clear();
}
/*//////////////////////////////////////////////////////////////////////*/
/*
   updateLEDs() - sends the LED states set in ledStates to the 74HC595
   sequence
*/
void updateLEDs(int value) {
  digitalWrite(latch, LOW);     //Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH);   //Pulls the latch high displaying the data
}
/*//////////////////////////////////////////////////////////////////////*/
/* changeLED(int led, int state) - changes an individual LED
   LEDs are 0 to 7 and state is either 0 - OFF or 1 - ON
*/
void changeLED(int led, int state) {
  ledState = ledState & masks[led];  //clears ledState of the bit we are addressing
  if (state == HIGH) {
    ledState = ledState | bits[led]; //if the bit is on we will add it to ledState
  }
  updateLEDs(ledState);              //send the new LED state to the shift register
}
/*//////////////////////////////////////////////////////////////////////*/
