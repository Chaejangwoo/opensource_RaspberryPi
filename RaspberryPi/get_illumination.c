#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <time.h>
#include <wiringPiSPI.h>


#define CS_MCP3208 8 // GPIO 8
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000 // 1Mhz

// SPI communication with RPi and get sensor data
int read_mcp3208_adc(unsigned char adcChannel) {
    unsigned char buff[3];
    int adcValue = 0;
    buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
    buff[1] = ((adcChannel & 0x07) << 6);
    buff[2] = 0x00;
    digitalWrite(CS_MCP3208, 0);
    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
    buff[1] = 0x0f & buff[1];
    adcValue = (buff[1] << 8) | buff[2];
    digitalWrite(CS_MCP3208, 1);
    delay(1000); // 추가: 1초 동안 대기
    return adcValue;
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

    unsigned char adcChannel_light = 0; // 추가: adcChannel_light 변수 선언

    if (wiringPiSetupGpio() == -1)
        exit(EXIT_FAILURE);
    if (setuid(getuid()) < 0)
    {
        perror("Dropping privileges failed\n");
        exit(EXIT_FAILURE);
    }

    int success = 0;
    do
    {
        int received_light = read_mcp3208_adc(adcChannel_light); // 수정: 센서 데이터를 received_light에 저장
	return received_light;
        if (received_light == 0)
        {
            printf("Data not good, skip\n");
            delay(500); // wait 0.5sec to refresh
            continue;
        }

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

        // Send SQL query to insert illumination
        char query_light[128];
        sprintf(query_light, "INSERT INTO illumination(illumination, currenttime) VALUES(%d,'%s')", received_light, current_time);
        if (mysql_query(conn, query_light))
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
