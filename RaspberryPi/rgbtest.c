#include <signal.h>     // For signal handling
#include <unistd.h>     // For sleep and delay functions
#include <stdio.h>      // For printf and fprintf functions
#include <string.h>     // For strerror function
#include <errno.h>      // For error number and strerror function
#include <stdlib.h>     // For exit function
#include <wiringPi.h>   // For GPIO control

#define RGBLEDPOWER 24   // GPIO pin for powering the RGB LED
#define RED 27           // GPIO pin for the red color of the LED
#define GREEN 28         // GPIO pin for the green color of the LED
#define BLUE 29          // GPIO pin for the blue color of the LED

void sig_handler(int signo);  // Declaration of the signal handler function

int main (void)
{
    signal(SIGINT, (void *)sig_handler);  // Registering the signal handler for SIGINT

    if (wiringPiSetup () == -1)  // Initialize wiringPi and check for errors
    {
        fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
        return 1;
    }

    pinMode(RGBLEDPOWER, OUTPUT);  // Set the RGBLEDPOWER pin as output
    pinMode(RED, OUTPUT);          // Set the RED pin as output
    pinMode(GREEN, OUTPUT);        // Set the GREEN pin as output
    pinMode(BLUE, OUTPUT);         // Set the BLUE pin as output

    digitalWrite(RGBLEDPOWER, 1);  // Turn on the power to the LED
    digitalWrite(RED, 1);
    digitalWrite(BLUE, 0);
    digitalWrite(GREEN, 0);

    delay(1000);  // Wait for 1 second

    digitalWrite(RGBLEDPOWER, 0);  // Turn off the power to the LED

    return 0;
}

void sig_handler(int signo)
{
    printf("process stop\n");  // Print a message indicating the process is stopping
    digitalWrite(RED, 0);      // Turn off the red LED
    digitalWrite(GREEN, 0);    // Turn off the green LED
    digitalWrite(BLUE, 0);     // Turn off the blue LED
    digitalWrite(RGBLEDPOWER, 0);  // Turn off the power to the LED
    exit(0);  // Exit the program
}

