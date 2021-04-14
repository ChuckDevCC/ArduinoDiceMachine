# Arduino DnD Dice Roller
This is an academic project for the Winter 2021 Semester. If you wanted to delve further into Arduiuno design and try this build for yourself, follow along with this Readme!

# Resources

These are the things you'll need!

### Software:
- [Arduino IDE][AIDE]

### Hardware:
- Some sort of Arduino micro-board. I used the [Arduino Uno][uno] 

- [LCD Display][lcd]

- Two breadboards with one being at least 50 rows. This project can be a little large!

- Some sort of Arduino kit, such as [this one from SOLARBOTICS][kit] that includes at least 2 buttons, wires, a potentiometer and serial integrated circuit such as this 74HC595:

<img src="https://i.imgur.com/JsIZY13.jpg" width="200">


> Note that you do not need to follow my specific build. It is built like this due to the parts I had on hand. 
Feel free when you are confident to change the code and design of the circuitry.



# Creating the Project

## Uploading the Code

With all your parts shipped and in, plug your Arduino board into your computer, then boot up your Arduino IDE.

- Once you plug your Arduino into a USB port, it should show in a new COMX port.
- Navigate to Tools -> Port -> Change to the Arduino port.
- Click "Upload" (Right arrow beside the check mark) to send code to Arduino.

![ComSetup](https://i.imgur.com/K5Ee9tq.png)
>Picture Credits: Chuck S. (Myself) From Chaotic Electronics Research Project

And then your Arduino code is loaded on the board!

## Building the Circuitry

### LCD
> Full build Picture
The LCD is quite straight forward. The manual that comes with tells you where the 4 wires need to be charged. Once plugged in, flip the display over and turn the blue square dial to an appropriate brightness setting - It's set to max by default.
> Dial picture

### LED's and Shift Register


This is under the assumption you have bought the same integrated circuit as listed earlier. If you did, place it on the edge of one of your breadboards, with the notch facing outwards. I have created a diagram using [Fritzing][fritzing] and a different IC to show you the wire structure. Explaining why all this works is beyond the scope of this readme, just enjoy some of the magic!

<img src = "https://i.imgur.com/Jx6eMrS.png">

### Buttons

The buttons straddle between both sides of the breadboard. one side has the control pin and positive on one side, and a resistor on the otherside of the breadboard and button, on the same prong file as the control pin. It will look something like the diagram below.

<img src = "https://imgur.com/94HTJIj.png">

make sure you have two of these, and the submit button goes to pin 8, with the dice button going to pin 9.

### Potentiometer

The potentiometer will have 3 prongs. The middle one is the control pin, which will go to Analog pin A0. the other two will be both 5V and ground. Check your type of potentiometer and attach the wires as required!

### RGB LED

Like the potentiometer, this also depends on the type of LED you receive. This diagram is the common case, but double-check if your colors seem different!

- Red : Pin 11
 - Green : Pin 10
 - Blue : Pin 9
<img src = "https://imgur.com/PgLbUu6.png">

# How to Use

First and foremost, give your Arduino a reset; Either press the 'Reset' button on the board, or upload again through the IDE.
If everything is setup as documented here, you will get a little welcome message on the LCD board, then it will tell you to hit submit.
It will ask you to load the amount of dice. This is for one roll, such as 6 20-sided die. It has a maximum of six. Every time you press the button,
it should light up the LED's sequentially. If something seems off, re-check the connections on the IC and the LED's. 
Next we use the pontetiometer to adjust which dice we want to roll. It reads bit values between the ground and five volt connections, 
and we use conditionals such as this one:

###
<img src="https://imgur.com/L3Rh99O.jpg">

to put these values to use. Once that is selected, we can use dice modifiers - a staple to any DnD game.
The LED will glow red or green to signify minus or plus (extra flavour!).

**Note: You don't need to choose a number. 0 works perfectly fine!**

You will get a prompt to roll more dice. This is in instances such as *inspiration* dice, an extra 1d4 to rolls. The LED will light again for a YES/NO prompt.
**This will only work 7 times - there are only 7 dice in a DnD set!**

Once you are finished, the dice roll value will display one roll at a time.
You will get a fun light show if you hit a critical failure or critical success - a little added tension - and the light will turn red or green accordingly.

After all the rolls, you will get the total roll with the modifiers calculated afterwards. It will be stationary until you want to run again!


# Current Limitations and Future Prospects
- Some parts of the button presses are tricky time-wise. Either the button seems to not register, or if you press for too long it will skip over a function.
This is because of the delay function - ```delay(250); ``` - and the summation of all of them in each function. 
- As with the point above, the delay times could be uniform with a constant declaration which might help with the timing.
- The overall rolling process is quite slow. This was more a passion project to see if it could be done. There is room for optimization, like any project!
- Removing the potentiometer for a third button or another type of input would have a better quality of life. I used what I had with my kit, and enjoy it's aesthetics regardless.


[AIDE]: <https://www.arduino.cc/en/software>
 [uno]: <https://store.arduino.cc/usa/fundamentals-bundle>
 [lcd]: <https://www.amazon.ca/gp/product/B07T8S3P1M/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1>
 [kit]: <https://solarbotics.com/product/ardx/>
[fritzing]: <https://github.com/fritzing/fritzing-app/releases/tag/CD-548>
