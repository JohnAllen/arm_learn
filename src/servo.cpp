#include <array>
#include "servo.h"
#include <fstream>
#include <iostream>
#include <wiringPi.h>
#include <pca9685.h>
#include <string>
#include <boost/filesystem.hpp>
#include "../../raspicam/src/raspicam_still.h"
#include <unistd.h>

/**
 *  *  *  * Calculate the number of ticks the signal should be high for the required amount of time
 *   *   *   */

int PIN_NUMS[] = {SERVO_0, SERVO_1, SERVO_2, SERVO_3};

using namespace std;
using namespace boost::filesystem;
using namespace boost::system;


int calcTicks(float impulseMs, int hertz)
{
    // impulseMs example: 0.6 - 2.4: 90 degrees = 1.5
    float cycleMs = 1000.0f / hertz; // 
    return (int)(MAX_PWM * impulseMs / cycleMs + 0.5f);
}

float map(float input, float min, float max)
{
    return (input * max) + (1 - input) * min;
}

float range_increment(float min, float max, float millis)
{
    return (max - min) / millis;
}

int angleMicroSec(int tick)
{
    return tick;
}

int servoAngle(int servo_num, int angle)
{
    float angleRange = 180;
    float servoRange = MAX - MIN;
//    cout << "servo range: " << servoRange << endl; // should be 2.4 - 0.6 == 1.8
    float anglePercent = angle / angleRange; // example 90 / 180 == 0.5
    float servoAngle = MIN + (anglePercent * servoRange); // 0.6 + (0.5 * 1.8) 
//    cout << "servoAngle is " << servoAngle << endl;
    int tick = calcTicks(servoAngle, HERTZ);
//    cout << "tick is: " << tick << endl;
    pwmWrite(servo_num, tick);
}

int resetServos()
{

    sleep(1);    
    servoAngle(SERVO_0, MIN_0);
    sleep(.5);    
    servoAngle(SERVO_1, MIN_1);
    sleep(.5);
    servoAngle(SERVO_2, MIN_2);
    sleep(.5);
}

int posArm(int angle)
{
   sleep(1.5);

    //raise 2
    int tick = calcTicks(2.4, HERTZ);
    pwmWrite(SERVO_2, tick);
    sleep(1.5);
    servoAngle(SERVO_1, TGT_1);
    sleep(1.5);
    servoAngle(SERVO_0, angle);
    sleep(1.5);
    servoAngle(SERVO_2, TGT_2);
    return 0;
}
int setServoMax(int num)
{
    int tick = calcTicks(MAX, HERTZ);
    pwmWrite(num, tick);
    return 0;
}
int setServoMin(int num)
{
    int tick = calcTicks(MIN, HERTZ);
    pwmWrite(num, tick);
    return 0;
}

int incServo(int servo_num)
{
    
}

