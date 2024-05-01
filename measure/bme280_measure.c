#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#define BME280_DEV "/dev/bme280"
#define LONG_SIGNED_INT_NUM (25)

int main() {
    int retval = 0;
    // Open connection to the database

    
    int bme280_dev_fd;
    char temp_buffer[LONG_SIGNED_INT_NUM] = "123";
    uint8_t num_bytes_read = 0;
    
    long signed int temperaturef, pressf;
    char *endptr;
    // Open I2C device file
    bme280_dev_fd = open(BME280_DEV, O_CREAT | O_RDWR, 0744);
    if (bme280_dev_fd < 0)
    {
        perror("Failed to open I2C device file");
        return -1;
    }
    if (sqlite3_open("finalProject.db", &db) != SQLITE_OK) {
        // If the database doesn't exist, create it
        if (sqlite3_open("finalProject.db", &db) == SQLITE_OK) {
            // Perform any initial setup here if needed
        } else {
            fprintf(stderr, "Failed to open/create database: %s\n", sqlite3_errmsg(db));
            return 1;
        }
    }  
    while (1) {          
        // Read temperature from BME280 sensor
        num_bytes_read = read(bme280_dev_fd, temp_buffer, LONG_SIGNED_INT_NUM);
        if (num_bytes_read < 0)
        {
            perror("Failed to read temperature from BME280 sensor");
            retval = -1;
            goto close_and_exit;
        }

        //printf("Value returned into the temperature buffer = %s\n", temp_buffer);

        // Convert temperature obtained in the buffer into int
        temperaturef = strtol(temp_buffer, &endptr, 10);
        pressf = strtol(endptr, &endptr, 10);
        if(errno)
        {
            perror("Failed to convert string to a numerical value, errno: ");
            printf("%s", strerror(errno));
            retval = -1;
            goto close_and_exit;
        } else if (endptr == temp_buffer) {
            perror("No digits were found in the string");
            retval = -1;
            goto close_and_exit;
        }
        if((*endptr != '\0') && (*endptr != 0x20))
        {
            printf("%d\n\r", (int)*endptr);
            perror("Failed to convert string to a numerical value, endptr != 0");
            retval = -1;
            goto close_and_exit;
        }
        // Print temperature
        printf("Temperature: %ld.%ldC\n", temperaturef/100, temperaturef % 100);
        printf("Pressure: %ld.%ldC\n", pressf/100, pressf % 100);
        sqlite3 *db;
        char *errMsg = 0;

        

        // SQL statement for table creation if it doesn't exist
        const char *createTableSQL = "CREATE TABLE IF NOT EXISTS sensor_data ("
                                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                     "timestamp INTEGER,"
                                     "temperature REAL,"
                                     "humidity REAL,"
                                     "pressure REAL);";

        // Execute the SQL statement to create the table
        if (sqlite3_exec(db, createTableSQL, NULL, 0, &errMsg) != SQLITE_OK) {
            fprintf(stderr, "Failed to create table: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return 1;
        }

        // SQL statement for insertion
        const char *insertSQL = "INSERT INTO sensor_data (timestamp, temperature, humidity, pressure) VALUES (?, ?, ?, ?)";

        // Prepare the SQL statement
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }

        // Bind values to the prepared statement
        // Example values, replace with your own
        int timestamp = time(NULL);
        double temperature = temperaturef/100;
        double humidity = -1;
        double pressure = pressf/100;

        sqlite3_bind_int(stmt, 1, timestamp);
        sqlite3_bind_double(stmt, 2, temperature);
        sqlite3_bind_double(stmt, 3, humidity);
        sqlite3_bind_double(stmt, 4, pressure);

        // Execute the SQL statement
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }

        // Finalize the statement and close the database connection
        sqlite3_finalize(stmt);




        // Delay for 5 seconds
        sleep(5);
    }
    close_and_exit:
        // Close I2C device file
        sqlite3_close(db);
        close(bme280_dev_fd);    
    return retval;
}
