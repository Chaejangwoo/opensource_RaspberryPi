#include <mysql.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

void run_command(const char *command, int times) {
    for (int i = 0; i < times; i++) {
        if (fork() == 0) {
            execl("/bin/sh", "sh", "-c", command, NULL);
            exit(0);  // Exit the child process
        }
        sleep(1);    // Wait for 1 second between each run
    }
}

int main() {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "192.168.43.46";
    char *user = "root";
    char *password = "root";
    char *database = "raspberry";

    conn = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    /* Send SQL query */
    if (mysql_query(conn, "SELECT pumpon, pumptime, lighton, lighttime, fanon, fantime FROM flag WHERE opened = 1")) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    /* Fetch the result of the query */
    row = mysql_fetch_row(res);

    /* If a result is found */
    if (row != NULL) {
        int pumpon = atoi(row[0]);
        int pumptime = atoi(row[1]);
        int lighton = atoi(row[2]);
        int lighttime = atoi(row[3]);
	int fanon = atoi(row[4]);
	int fantime = atoi(row[5]);


        if (pumpon == 1) {
            if (fork() == 0) {
                run_command("/home/pi/smartfarm/pumpon", pumptime);
                exit(0);
            }
        }

        if (lighton == 1) {
            if (fork() == 0) {
                run_command("/home/pi/smartfarm/rgbtest", lighttime);
                exit(0);
            }
        }

        if (fanon == 1) {
            if (fork() == 0) {
                run_command("/home/pi/smartfarm/fanon", fantime);
                exit(0);
            }
        }


        /* Calculate the remaining time */
        int remaining_time = 60 - (pumptime > lighttime ? (fantime > pumptime ? fantime : pumptime ): (fantime > lighttime ? fantime : lighttime ) );
        if (remaining_time > 0) {
            sleep(remaining_time);
        }

    }

    /* Wait for all child processes to finish */
    while (wait(NULL) > 0);

    /* Delete rows where opened is 1 */
    if (mysql_query(conn, "DELETE FROM flag WHERE opened = 1")) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    /* Close connection */
    mysql_free_result(res);
    mysql_close(conn);

    return 0;
}

