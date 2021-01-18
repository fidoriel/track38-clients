/*
Copyright (c) 2019 fidoriel

MIT License

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* 940nm LED
 * Protocol <12345>\0
 * 1: T = Train, S = Switch
 * Train:
 * 2 and 3 GPIO Channel
 * 4 Channel and Subchannel 1-8
 * 5 Speed
 * Switch:
 * 2 and 3 GPIO Channel
 * 4 r = right or l = left switch
 * 5 Position T = Turn S = Straight
*/

#include <Arduino.h>
#include <Servo.h>
#include "PowerFunctions.h"

char inputString[32] = "";
bool reciveInProgress = false;
int counter = 0;
String readString;
Servo myservo;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  delay(100);
  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(57600);
}

bool readNewData() {
	char start = '<';
	char stop = '>';
	char tmp;

	while (Serial.available())
	{
		tmp = Serial.read();

		if (tmp == '\0')
		{
		}

		else if (tmp == stop && reciveInProgress)
		{
			reciveInProgress = false;
			inputString[counter] = '\0';
			counter = 0;
			readString = inputString;
			inputString[0] = '\0';
			Serial.flush();
			return true;
		}

		else if (reciveInProgress)
		{
			inputString[counter] = tmp;
			counter++;
		}
		
		else if (tmp == start)
		{
		reciveInProgress = true;
		inputString[counter] = tmp;
		counter++;
		}
	}
	return false;
}

void Straight(int ServoPin, char type) {
  myservo.attach(ServoPin);
  if (type == 'R')
  {
		myservo.write(175);
		delay(400);
		myservo.write(172);
  } 

  else
  {
		myservo.write(95);
		delay(400);
		myservo.write(98);
  }
  delay(50);
  myservo.detach();
  digitalWrite(ServoPin, LOW);
}

void Turn(int ServoPin, char type) {
  myservo.attach(ServoPin);
	if (type == 'L') {
		myservo.write(175);
		delay(400);
		myservo.write(172);
	} else
	{
		myservo.write(95);
		delay(400);
		myservo.write(98);
	}
	delay(50);
	myservo.detach();
	digitalWrite(ServoPin, LOW);
}

void pfGO(int irPin, int combCh, char speedChar)
{
	int ch;
	char subch;
	int speed;
	switch (combCh)
	{
		case 1:
			ch = 0;
			subch = 'R';
			break;
		case 2:
			ch = 0;
			subch = 'B';
			break;
		case 3:
			ch = 1;
			subch = 'R';
			break;
		case 4:
			ch = 1;
			subch = 'B';
			break;
		case 5:
			ch = 2;
			subch = 'R';
			break;
		case 6:
			ch = 2;
			subch = 'B';
			break;
		case 7:
			ch = 2;
			subch = 'R';
			break;
		case 8:
			ch = 2;
			subch = 'B';
			break;
	}

	switch (speedChar)
	{
		case 'S':
			speed = PWM_BRK;
			break;

		case 'A':
			speed = PWM_FWD1;
			break;
		case 'B':
			speed = PWM_FWD2;
			break;
		case 'C':
			speed = PWM_FWD3;
			break;
		case 'D':
			speed = PWM_FWD4;
			break;
		case 'E':
			speed = PWM_FWD5;
			break;
		case 'F':
			speed = PWM_FWD6;
			break;
		case 'G':
			speed = PWM_FWD7;
			break;

		case 'Z':
			speed = PWM_REV1;
			break;
		case 'Y':
			speed = PWM_REV2;
			break;
		case 'X':
			speed = PWM_REV3;
			break;
		case 'W':
			speed = PWM_REV4;
			break;
		case 'V':
			speed = PWM_REV5;
			break;
		case 'U':
			speed = PWM_REV6;
			break;
		case 'T':
			speed = PWM_REV7;
			break;
	}
	PowerFunctions pf(irPin, ch);

	if (subch == 'R')
	{
		pf.red_pwm(speed);
	}
	if (subch == 'B')
	{
		pf.blue_pwm(speed);
	}
}

void loop() {
	if (readNewData())
	{
		if (readString[1] == 'S')
		{
			int servoGPIO = readString.substring(2, 4).toInt();
			char switchKind = readString[4];
			char newPosition = readString[5];
			//Serial.print(String(servoGPIO));
			//Serial.print(switchKind);
			//Serial.print(newPosition);
			if (newPosition == 'T')
			{
				Turn(servoGPIO, switchKind);
				Serial.print("OK");
			}
			else if (newPosition == 'S')
			{
				Straight(servoGPIO, switchKind);
				Serial.print("OK");
			}
			readString = "";
		}

		if (readString[1] == 'T')
		{
			int trainGPIO = readString.substring(2, 4).toInt();
			int channelAndSubchannel = int(readString[4])-48;
			char speedChar = readString[5];
			pfGO(trainGPIO, channelAndSubchannel, speedChar);
			Serial.print("OK");
			readString = "";
		}
	}
}