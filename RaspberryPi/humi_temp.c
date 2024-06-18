/*
* dht22.c:
* Simple test program to test the wiringPi functions
* Based on the existing dht11.c
* Amended by technion@lolware.net
*/
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <time.h>

//#include "locking.h"
#define MAXTIMINGS 85
int ret_humid, ret_temp;
static int DHTPIN = 11; // Change to 11
static int dht22_dat[5] = {0,0,0,0,0};
static uint8_t sizecvt(const int read)
{
 /* digitalRead() and friends from wiringpi are defined as returning a value
 < 256. However, they are returned as int() types. This is a safety function
*/
 if (read > 255 || read < 0)
 {
 printf("Invalid data from wiringPi library\n");
 exit(EXIT_FAILURE);
 }
 return (uint8_t)read;
}
int read_dht22_dat()
{
 uint8_t laststate = HIGH;
 uint8_t counter = 0;
 uint8_t j = 0, i;
 dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] =
0;
// pull pin down for 18 milliseconds
 pinMode(DHTPIN, OUTPUT);
 digitalWrite(DHTPIN, HIGH);
 delay(10);
 digitalWrite(DHTPIN, LOW);
 delay(18);
 // then pull it up for 40 microseconds
 digitalWrite(DHTPIN, HIGH);
 delayMicroseconds(40);
 // prepare to read the pin
 pinMode(DHTPIN, INPUT);
// detect change and read data
for ( i=0; i< MAXTIMINGS; i++) {
 counter = 0;
 while (sizecvt(digitalRead(DHTPIN)) == laststate) {
 counter++;
 delayMicroseconds(3);

 if (counter == 255) {
 break;
 }
 }
 laststate = sizecvt(digitalRead(DHTPIN));
 if (counter == 255) break;
 // ignore first 3 transitions
 if ((i >= 4) && (i%2 == 0)) {
 // shove each bit into the storage bytes
 dht22_dat[j/8] <<= 1;
 if (counter > 16)
 dht22_dat[j/8] |= 1;
 j++;
 }
}
 // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
 // print it out if data is good
 if ((j >= 40) &&
 (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] +
dht22_dat[3]) & 0xFF)) ) {
 float t, h;
 h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
 h /= 10;
 t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
 t /= 10.0;
 if ((dht22_dat[2] & 0x80) != 0) t *= -1;
ret_humid = (int)h;
ret_temp = (int)t;
printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
//printf("Humidity = %d Temperature = %d\n", ret_humid, ret_temp);
 return ret_humid;
}
 else
 {
 printf("Data not good, skip\n");
 return 0;
 }
}
int main(void)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "192.168.43.46";
    char *user = "root";
    char *password = "root";
    char *database = "raspberry";

    if (wiringPiSetup() == -1)
        exit(EXIT_FAILURE);
    if (setuid(getuid()) < 0)
    {
        perror("Dropping privileges failed\n");
        exit(EXIT_FAILURE);
    }

    int success = 0;
    do
    {
        if (read_dht22_dat() == 0)
        {
            printf("Data not good, skip\n");
            delay(500); // wait 0.5sec to refresh
            continue;
        }
        int received_temp = ret_temp;
        int received_humid = ret_humid;
	return received_temp;
        conn = mysql_init(NULL);

        // Connect to database
        if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
        {
            fprintf(stderr, "%s\n", mysql_error(conn));
            return 0;
        }

        // Get current time
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char current_time[20];
        sprintf(current_time, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

        // Send SQL query to insert temperature
        char query_temp[128];
        sprintf(query_temp, "INSERT INTO temperature(temperature, currenttime) VALUES(%d,'%s')", received_temp, current_time);
        if (mysql_query(conn, query_temp))
        {
            fprintf(stderr, "%s\n", mysql_error(conn));
            return 0;
        }

        // Send SQL query to insert humidity
        char query_humid[128];
        sprintf(query_humid, "INSERT INTO humidity(humidity, currenttime) VALUES(%d,'%s')", received_humid, current_time);
        if (mysql_query(conn, query_humid))
        {
            fprintf(stderr, "%s\n", mysql_error(conn));
            return 0;
        }

        // Close connection
        mysql_close(conn);

        success = 1; // Set success to 1 if data is good and stored in database

    } while (!success); // Repeat until success is 1

    return 0;
}
