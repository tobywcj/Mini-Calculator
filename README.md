# ELEC3662-Mini-Calculator
This project aims to interface and programme Tiva C Series TM4C123GH6PM microcontroller, a 16-fonts x 2-line HD44780 LCD and a 4x4 keypad each other through hardware and Keil uVision software.

# Release/Demo Version
![1](https://user-images.githubusercontent.com/71925079/149642361-37fab73e-3462-448b-bcc9-a65f1b99e757.jpg)

# Schematic Circuit layout
![2](https://user-images.githubusercontent.com/71925079/149642376-ad086739-160f-4fb2-a020-5ed3df4656d0.png)

# Keypad Buttons Display
The below figure 1 shown all the keys available for the keypad, which has a shift mode to access more keys. By pressing ‘S’ one time, the key will convert to the right keypad in the figure 1. The shift mode will only be valid for ONE time after the shift key is pressed to imitate original calculator. LED on the MCU will turn BLUE when shift mode is on to remind user.

![3](https://user-images.githubusercontent.com/71925079/149642391-ba1d701f-9d5f-41ec-b93a-f95fa6aea357.png)
Figure 1. Keys allocation

# Calculator Features
The following table shows the essential and additional features of my calculator.
‘@’ means the function is additional.
|Functions|Explanation|Controlled keys (Figure 1)|
|---|---|---|
|@ Infinite continuing calculation|pressing ‘=’ based on three-number calculation per equation, converting the previous answer into the first number of next calculation|‘=’|
|Shift-mode |	To allow more key usage	|‘S’|
|Addition	| /|	‘+’|
|Subtraction|/ |	‘-‘|
|Multiplication	|	/|‘S’ -> ‘*’|
|Division|	/|‘S’ -> ‘/‘|
|Times ten to the power|	/|‘S’ -> ‘E‘|
|@ Trigometric math functions|	sin, cos, tan|	‘S’ -> ‘cos‘, ‘S’ -> ‘sin‘, ‘S’ -> ‘tan‘|
|@ log10	|logarithm with base 10 of the input	|‘S’ ->‘log‘|
|Decimal number |	unlimited decimal places (precision of answer up to 4 decimal places)	|‘.‘|
|Precedence operation|	Automatic operation without brackets needed	|‘+’ , ‘-‘ , ‘*’ , ‘/‘|
|Rubout last entry	Rubout entire function for trigometric functions	‘R’
|delete entire entry|	/	|‘S’ -> ‘D‘|
|@ Syntax Error reminder|	Instant reminder of incorrect math equation input which leads to math error on the display including operations of “+”, “-“, “*”, “/”, “.”, “E” and trigometric functions	|/|
|@ Maximum-number operation reminder|	Instant reminder on the display to remind user not to type more number|	/|
|@ Enter Operand reminder |	Instant reminder on the display to remind user to only enter operand after pressing ‘=’ to continue calculation|	/|
|@ Perfect key-pressing optimization|	Display of keypad without character collision, button debouncing and instant response	|Any keys|
|@ LED response|	Incorrect password = red Successful calculator entry = green Every key pressed = yellow Shift mode on = blue|	/|
|@ Exit |	Reboot the calculator	|‘S’ -> ‘D‘->‘S’ -> ‘D‘|
|@ Calculator graphic design on entry and exit|	/	|/|
|@ Display-shifting	|Shifting display while typing equation|	/|
|Password Encryption |	Used Flash Memory	|/|
|Change password	|User can change the password by choosing option on the display|	Follow display options|
|Contrast adjustment	|/|	Using the potentiometer|

# Video demonstration of different functions
|Functions|Example|Video|
|---|---|---|
|Password encryption and change + Exit + Graphics| ![4](https://user-images.githubusercontent.com/71925079/149642827-e88787f4-3cbf-42bc-ae86-6a3cc848b1d6.jpg)|https://user-images.githubusercontent.com/71925079/149642888-b593efab-108a-4dcc-b434-6039f10c471b.mp4
|Infinite continuing calculation + Automatic conversion of previous answer to next calculation|Shown in the video|https://user-images.githubusercontent.com/71925079/149642881-d8a55cf0-3bef-4d10-801d-84f4e74c7443.mp4
|Rubout last entry + Delete entire entry + Precedence operation + Decimal number + Display-shifting|![5](https://user-images.githubusercontent.com/71925079/149642861-7621103f-4fdc-42fb-8f2d-d5227b6b5d54.jpg)|https://user-images.githubusercontent.com/71925079/149642901-1abaf5af-f646-4255-9ace-1ebb02e1d4ff.mp4
|Shift-mode (blue LED) + Trigometric functions + Times ten to the power + log10|![6](https://user-images.githubusercontent.com/71925079/149642866-9625fc7a-2aea-4bc4-b824-4c4d58342f84.jpg)|https://user-images.githubusercontent.com/71925079/149642917-977a69a8-9a08-4773-bceb-80fe17895a44.mp4
|Syntax Error reminder + Maximum-number operation reminder + Enter Operand reminder|Shown in the video|https://user-images.githubusercontent.com/71925079/149642923-af4bf2e2-d164-461d-aac1-f626e8cb0227.mp4

# Project Highlights
## ***ERROR reminders***

This calculator is coded with syntax error of different math function and operands signs problems, like the modern calculator, in order to make a flawless math calculation for users to avoid math and sequence mistakes. This part took most of my time as I have to think of every possible case of syntax error and implement logically in the coding, which need to use flags and counters to pass through functions.

1.	Syntax error
Syntax error will be popped up instantly after user inputted the cases of math error that calculator cannot operate, the key pressed will not be printed and stored in the calculation.
To prevent the syntax-error cases below:
-	(1) two operands are printed in a row (2) "A + B +=" (3) "1.2.3"/"1..2"
-	(1) "1+1cos" (2) "coscos" (3) “1+.cos" (4) “ cos- ” / “ cos* ” / “ cos/ ” (all trigometric functions)
-	(1) "1+10E+", "1+10E-", "1+10Ex", "1+10E/", "1+10E1.", "1+10E=" (2) "1+E" (4) "1+10EE"
-	(1) "1+1log" (2) "loglog" (3) 1+.log" (4) “ log-” / “ log* ” / “ log/ ”

2.	Max operands error
Reminding message will be popped up if user inputted more than three number calculation on the equation.

3.	Enter operand reminder
Reminding message will be popped up if user inputted a number after pressed ‘=’ to continue a calculation based on the previous answer, requiring user to enter operand for second number calculation.

## ***Rubout last entry / Delete entire entry***

Honestly, this part is quite challenging. To start with, it involved a lot of deletion of various math function, instead of just number. For cancellation of number, I have created a RemoveLastDigit_AnyNumber () function to remove any decimal or integer number. For cancellation of math functions including its input, I have created a Count_AnyNumber_digit () function to count the total digits of any number decimal or integer number, then adding the number digits involved for the math function. Thus, for trigometric, logarithm and times ten to the power function the calculator need to count all related digits then delete it. This part need to use strcpy() to delete the last character in the equation buffer, clear display, then print the string in the buffer again.

## ***Precedence operation***

Precedence operation
For software implementation, I divided three-number calculation into three parts, which the calculator will recalculate the equation everytime user input a one-digit number to get the final answer anytime. Regarding the first and second parts, just simple math calculations. The FirstNumber(), SecondNumber() and ThirdNumber() will convert every key user pressed into a whole number. Everytime user input operands ‘+’, ‘-‘, ‘*’, ‘/‘, it will increment the calculator into next number concatenation using strcat() to concatenate the key pressed to the equation buffer on the display.
For the third part, it is an automatic precedance operation on equation. After user inputted the second operand, the calculator will count the number of prioritized operand ‘*’and ‘/‘, then carry out precedance operation format based on number of prioritized operands, which will calculate the math of second and third number if the prioritized operand is located at the second operand.

## ***LED response***

This LED could inform users the calculator is carrying on which operation and situation. 

|Calculator situation|LED colour|
|---|---|
|Incorrect password|	red|
|Successful calculator entry|	green|
|Every key pressed	|yellow|
|Shift mode on|	blue|

***Accurate key pressing***

There is a 0.125 sec delay between first and second reads of key detection to avoid collision of characters pressed popped up at the same time and continuously writing the same character to the LCD when holding the key, attaining perfect and accurate response of keys.

![image](https://user-images.githubusercontent.com/71925079/149768825-180395bb-503e-4dbe-8ec9-033f3abb5ca0.png)







