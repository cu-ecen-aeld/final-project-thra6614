#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define BME280_DEV "/dev/bme280" 
#define LONG_SIGNED_INT_NUM (11)
int main() {
    int bme280_dev_fd;
    char temp_buffer[LONG_SIGNED_INT_NUM];
    uint8_t num_bytes_read = 0;
    int retval = 0;
    long signed int temperature;
    char *endptr;
    // Open I2C device file
    bme280_dev_fd = open(BME280_DEV, O_CREAT | O_RDWR, 0744);
    if (bme280_dev_fd < 0) 
    {
        perror("Failed to open I2C device file");
        return -1;
    }
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
    temperature = strtol(temp_buffer, &endptr, (LONG_SIGNED_INT_NUM - 1));
    
    if(errno)
    {
        perror("Failed to convert string to a numerical value, errno: "); 
        printf("%s", strerror(errno));
        retval = -1;
        goto close_and_exit;
    }
    if((*endptr != '\0'))
    {
        printf("%c\n\r", *endptr);
        perror("Failed to convert string to a numerical value, endptr != 0");
        retval = -1;
        goto close_and_exit;
    }
    // Print temperature
    printf("Temperature: %ld.%ldC\n", temperature/100, temperature % 100);
close_and_exit:
    // Close I2C device file
    close(bme280_dev_fd);
    return retval;
}
