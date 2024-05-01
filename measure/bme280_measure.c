#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include "bme280.h" // Include the BME280 library header file
#define BME280_DEV "/dev/bme280"
#define LONG_SIGNED_INT_NUM (25)

int main() {
    int retval = 0;
    // Open connection to the database

    sqlite3 *db;
    int bme280_dev_fd;
    struct bme280_data sensor_data; // Structure to hold sensor data
    struct bme280_dev sensor; // Structure to hold BME280 device
    char temp_buffer[LONG_SIGNED_INT_NUM] = "123";
    uint8_t num_bytes_read = 0;

    // Initialize the BME280 device
    sensor.dev_id = BME280_I2C_ADDR_PRIM;
    sensor.intf = BME280_I2C_INTF;
    sensor.read = user_i2c_read;
    sensor.write = user_i2c_write;
    sensor.delay_ms = user_delay_ms;

    if (bme280_init(&sensor) != BME280_OK) {
        fprintf(stderr, "Failed to initialize BME280\n");
        return -1;
    }

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
        // Read temperature, humidity, and pressure from BME280 sensor
        if (bme280_get_sensor_data(BME280_ALL, &sensor_data, &sensor) != BME280_OK) {
            fprintf(stderr, "Failed to get sensor data from BME280\n");
            retval = -1;
            goto close_and_exit;
        }

        // Print temperature, humidity, and pressure
        printf("Temperature: %.2f°C\n", sensor_data.temperature);
        printf("Humidity: %.2f%%\n", sensor_data.humidity);
        printf("Pressure: %.2f hPa\n", sensor_data.pressure);

        // Insert sensor data into SQLite database
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
        int timestamp = time(NULL);
        sqlite3_bind_int(stmt, 1, timestamp);
        sqlite3_bind_double(stmt, 2, sensor_data.temperature);
        sqlite3_bind_double(stmt, 3, sensor_data.humidity);
        sqlite3_bind_double(stmt, 4, sensor_data.pressure);

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
