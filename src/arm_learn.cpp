#include <stdio.h>
#include "../../raspicam/src/raspicam_still.h"
#include <iostream>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <wiringPi.h>
#include <pca9685.h>
#include <fstream>
#include <ctime>
#include <string>
#include <chrono>

#define PIN_BASE 300
#define MAX_PWM 4096
#define HERTZ 50

#define SERVO_0 300
#define SERVO_1 301
#define SERVO_2 302
#define SERVO_3 303 

#define MIN_0 0.9
#define MAX_0 2.1
#define MIN_1 0.9
#define MAX_1 2.1

/**
 *  *  * Calculate the number of ticks the signal should be high for the required amount of time
 *   *   */
int calcTicks(float impulseMs, int hertz)
{
        float cycleMs = 1000.0f / hertz;
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

/*
 *  Teach an arm to learn to move to a certain location given an image containing that location
 *
 *  There is the question of specifying the location of the object in pixels or just giving it an image and hoping it do 
 *  its thing with a given image.
 *  I want to use an electronic success signal: when the end-effector reaches the desired location, a circuit is complete
 *  I like this approach because it is very binary and obvious whether or not the task has been successfully accomplished.
 *  The computer will know on its own whether or not it has been successful.  
 *  Idea is to manually move the end-effector with buttons a few times for a given image - perhaps only one time for each
 *  position, and see how well it does.
 */

using namespace std;
const int TASK_TIME_LIMIT_SECS = 20;

int main ()
{
    // required by ncurses.h; inits key read
    int TASK_ATTEMPT_DURATION_SECS = 0;
    
    cout << "Beginning" << endl;
    if (wiringPiSetup () == -1) //using wPi pin numbering
    {
        cout << "wiring pi not setup " << endl;
        exit(0);
    }
    
    int fd = pca9685Setup(PIN_BASE, 0x40, HERTZ);
    
    if (fd < 0) {
        printf("Error in setup\n");
        return fd;
    }
    
    pca9685PWMReset(fd);
    
    initscr();
    
    int tick, i, j = 0;

    float increment = range_increment(MIN_0, MAX_0, SCAN_TIME);
    tick = calcTicks(MIN_0, HERTZ);
    pwmWrite(SERVO_0, tick);  // set servo to minimum

    
    while (1==1) 
    {
        int key = getch();  // read keyboard input
        cout << "key" << key << "pressed" << endl;
        
        if (key == ' ' || key == 32 || TASK_ATTEMPT_DURATION_SECS >= TASK_TIME_LIMIT_SECS) // TODO which one? 
        {
            // Capture an image
            // Save it to a file to be read into TensorFlow later 
            // initiate/instantiate Raspicam
           raspicam::RaspiCam_Still Camera;
            cout << "Opening Camera..." << endl;
            sleep(2); 
            pwmWrite(1, 95); 
            pwmWrite(2, 95);
            
/*           if (!Camera.open())
            {
                cerr<<"Error opening camera"<< endl;
                return -1;
            }*/
            cout << "opening camera capturing an image" << endl; 
//          Camera.grab() // not still 
            Camera.setWidth(1280);
            Camera.setHeight(960);
            Camera.setISO(600);
        //    Camera.setShutterSpeed(100);
            Camera.setBrightness(65);
            Camera.setEncoding(raspicam::RASPICAM_ENCODING_JPEG); 
            Camera.open();

            sleep(2);
            unsigned int length = Camera.getImageBufferSize(); // Header + Image Data + Padding
            unsigned char * data = new unsigned char[length];
            if ( !Camera.grab_retrieve(data, length) ) {
                cerr<<"Error in grab"<<endl;
                return -1;
            }

            cout<<"saving picture.jpg"<<endl;
            ofstream file ( "picture.jpg",ios::binary );
            file.write(( char*)data, length);

/*
            unsigned char *data = new unsigned char[Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB )];
            //sleep(3); 
            Camera.retrieve(data);  //get the image
    //save
  */       
            /*std::ofstream outFile(file_name,std::ios::binary);
            outFile<<"P6\n"<<Camera.getWidth() <<" "<<Camera.getHeight() << endl;
            outFile.write((char*) data, Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB));
            cout<< "Image saved at " << file_name << endl;
           */ // delete the image from memory
            delete data; 

            // wait one second to allow image to be written to file before recording servo inputs
            sleep(1);
    
            bool SUCCESSFUL = false; // has arm successfully completed its task?
            
            // Begin recording button movements to complete move task
            // time to complete task counter; 
            //  if time greater than some max amount, don't even include it in our dataset - start over.  
            //  This could go on for ours if exploring to much or something else goes wrong.
            time_t begin = time(NULL); 
            cout << "Beginning attempt of robotic task.  I have " << TASK_TIME_LIMIT_SECS << " seconds to complete it" << endl;
            
            while (!SUCCESSFUL && TASK_ATTEMPT_DURATION_SECS < TASK_TIME_LIMIT_SECS)
            {
                // init ints that will hold keypress time for each servo button.  These numbers will ultimately fed to TensorFlow
                int h,i,j,k = 0;
                // if key pressed is one of our desired servo keys... http://www.asciitable.com/
                nodelay(stdscr, TRUE);  // make the screen input non-blocking
                while (key && key <= 107 && key >= 104) 
                {
                    cout << key << " pressed" << endl;
                    // Record buttons for time depressed
                    // Each loop increments key counter one second - see sleep(1) below 
                    switch (key)
                    {
                        case 104:
                        {
                            h++;
                            cout << " h pressed" << endl;
                        }
                        case 105:
                        {
                            i++;
                            cout << " i pressed" << endl;
                        }
                        case 106:
                        {
                            j++;
                            cout << " j pressed" << endl;
                        }
                        case 107:
                        {
                            k++;
                            cout << " k pressed" << endl;
                        }
                    }                    
                }

                // If electronic circuit has been completed
              int CIRCUIT_CONNECTED = digitalRead(4);  
              if (1==2/*pin is high*/)
                {
                    SUCCESSFUL = true; // exit loop that will store servo times 
                }
                
                time_t now = time(NULL);
                TASK_ATTEMPT_DURATION_SECS = difftime(now, begin); 
                cout << "SECONDS PASSED: " << TASK_ATTEMPT_DURATION_SECS << endl;
                cout << "I have " << TASK_TIME_LIMIT_SECS - TASK_ATTEMPT_DURATION_SECS << " seconds left to complete the task"<< endl;
                sleep(1);
            }
            // write individual servo times somewhere once task was successful
            // reset servo to initial state and wait for another 32/space bar
        }
    sleep(.5);
   } 
                return 0;   
}
