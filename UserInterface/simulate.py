import subprocess
import random
import time
import datetime

# Function to create database table
def create_table():
    # Run sqlite3 command to create the table
    subprocess.run(['/usr/bin/sqlite3', 'finalProject.db',
                    'CREATE TABLE IF NOT EXISTS sensor_data (timestamp INTEGER, temperature REAL, humidity REAL, pressure REAL);'])

# Function to generate random sensor data and insert into database
def insert_data():
    # Generate timestamp
    timestamp = int( time.time() )

    # Convert timestamp to a datetime object
    dt_object = datetime.datetime.fromtimestamp( timestamp )

    # Generate random sensor data
    temperature = round( random.uniform( 20.0, 30.0 ), 2 )
    humidity = round( random.uniform( 40.0, 60.0 ), 2 )
    pressure = round( random.uniform( 900.0, 1100.0 ), 2 )

    # Run sqlite3 command to insert data into database
    subprocess.run(['/usr/bin/sqlite3', 'finalProject.db',
                    f'INSERT INTO sensor_data (timestamp, temperature, humidity, pressure) VALUES ({timestamp}, {temperature}, {humidity}, {pressure});'])

    # Print the inserted data with the date in iOS format
    print( "Data inserted successfully:" )
    print( "Date:", dt_object.strftime( '%Y-%m-%d %H:%M:%S' ) )  # iOS date format
    print( "Temperature:", temperature )
    print( "Humidity:", humidity )
    print( "Pressure:", pressure )


# Main function to generate and insert data periodically
def main():
    create_table()
    while True:
        insert_data()
        # Wait for 5 seconds before inserting next data
        time.sleep( 5 )

if __name__ == "__main__":
    main()
