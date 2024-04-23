/*
 * File: aesdsocket.c
 * Author: Kanin McGuire
 * Date: 2/25/2024
 * Description: C application for creating a socket-based server to receive and send data,
 *              with sys/log logging.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "queue.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sqlite3.h"

#define USE_AESD_CHAR_DEVICE 1

// Define the path to the data file
#if USE_AESD_CHAR_DEVICE
    #define DATA_FILE "/dev/aesdchar"
#else
    #define DATA_FILE "/var/tmp/aesdsocketdata"
#endif

// Declare global variables for the socket file descriptor and file pointer
int serverSocket;
FILE *filePointer;
int fd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t timestampThread;

#define DATABASE_FILE "finalProject.db"
sqlite3 *db;

// Structure to hold thread information
struct ThreadInfo
{
    pthread_t threadId;
    int clientSocket;
    bool threadComplete;
    SLIST_ENTRY( ThreadInfo ) entries;
};

// Declare the head of the singly linked list
SLIST_HEAD( ThreadHead, ThreadInfo ) threadHead;

// Signal handler function to catch SIGINT and SIGTERM signals
void signalHandler ( int sig )
{
    // Check if the signal is SIGINT or SIGTERM
    if ( sig == SIGINT || sig == SIGTERM )
    {
        // Log a message indicating the signal caught
        syslog( LOG_INFO, "Caught signal, exiting" );

        // Attempt to close the socket file descriptor
        if ( serverSocket != -1 )
        {
            shutdown( serverSocket, SHUT_RDWR );
        }

        // Close the syslog connection
        closelog();
        pthread_cancel( timestampThread );

        // Iterate over the thread list and join each thread
        struct ThreadInfo *currentThread, *nextThread;
        SLIST_FOREACH_SAFE( currentThread, &threadHead, entries, nextThread )
        {
            pthread_join( currentThread->threadId, NULL );
            SLIST_REMOVE( &threadHead, currentThread, ThreadInfo, entries );
            free( currentThread );
        }
        // Exit
        exit( 0 );
    }
}

// Function to retrieve the last 10 entries from the database and send them to the client
int sendLast10Entries( int clientSocket )
{
    char *sql = "SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 10;";
    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    if ( sqlite3_prepare_v2( db, sql, -1, &stmt, 0 ) != SQLITE_OK )
    {
        syslog( LOG_ERR, "SQL error: %s", sqlite3_errmsg( db ) );
        return -1;
    }

    // Create a buffer to hold the data to send
    char buffer[1024];

    // Initialize buffer position
    int bufferPos = 0;

    // Send each row to the client
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        const unsigned char *timestamp = sqlite3_column_text( stmt, 0 );
        const unsigned char *temperature = sqlite3_column_text( stmt, 1 );
        const unsigned char *humidity = sqlite3_column_text( stmt, 2 );
        const unsigned char *pressure = sqlite3_column_text( stmt, 3 );

        // Format the data and append it to the buffer
        bufferPos += snprintf( buffer + bufferPos, sizeof( buffer ) - bufferPos,
                              "Timestamp: %s, Temperature: %s, Humidity: %s, Pressure: %s\n",
                              timestamp, temperature, humidity, pressure );
    }

    // Finalize the statement
    sqlite3_finalize( stmt );

    // Send the buffer data over the socket connection
    if ( send( clientSocket, buffer, strlen( buffer ), 0 ) == -1 )
    {
        syslog( LOG_ERR, "Failed to send data: %s", strerror( errno ) );
        return -1;
    }

    // write to the File
    if ( write( fd, buffer, strlen( buffer ) ) == -1 )
    {
        // Handle error
        syslog( LOG_ERR, "Failed to write to file %s: %s", DATA_FILE, strerror( errno ) );
        return -1;
    }
    return 0;
}

// Function to handle client connections
void *handleClient ( void *arg )
{
    // Get thread information from the argument
    struct ThreadInfo *threadInfo = ( struct ThreadInfo * ) arg;
    int clientSocket = threadInfo->clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t addrSize = sizeof( clientAddr );

    // Get client address information
    if ( getpeername( clientSocket, ( struct sockaddr * )&clientAddr, &addrSize ) == -1 )
    {
        // Log an error if getting client address fails
        syslog( LOG_ERR, "Failed to get client address: %s", strerror( errno ) );
        close( clientSocket );
        threadInfo->threadComplete = true;
        pthread_exit( NULL );
    }

    // Declare variables to store client IP address
    char ipAddress[INET_ADDRSTRLEN];

    // Convert the client IP address to a string format
    inet_ntop( AF_INET, &clientAddr.sin_addr, ipAddress, sizeof( ipAddress ) );

    // Log a message indicating the accepted connection
    syslog( LOG_INFO, "Accepted connection from %s", ipAddress );

    // Declare buffer and variables for receiving data from the client
    char buffer[1024];
    ssize_t bytesReceived;
    fd = open( DATA_FILE, O_CREAT | O_RDWR | O_APPEND, 0744 );
    if ( fd == -1 )
    {
        // Handle error
        syslog( LOG_ERR, "Failed to open file %s: %s", DATA_FILE, strerror( errno ) );
        pthread_mutex_unlock( &mutex );
        close( clientSocket );
        threadInfo->threadComplete = true;
        pthread_exit( NULL );
    }

    // Receive data from the client
    while ( ( bytesReceived = recv( clientSocket, buffer, sizeof( buffer ), 0 ) ) > 0 )
    {
        // Print the received command
        printf( "Received command from client: %s\n", buffer );

        // Check if the received command is "get10"
        if ( strncmp( buffer, "get10", 5 ) == 0 )
        {
            printf( "Received command: %s\n", buffer );
            // Call function to retrieve the last 10 entries from the database and send them to the client
            if ( sendLast10Entries( clientSocket ) )
            {
                close( fd );
                pthread_mutex_unlock( &mutex );
                close( clientSocket );
                threadInfo->threadComplete = true;
                pthread_exit( NULL );
            }
        }
        else
        {
            // Handle other commands as needed
            // Echo back the received data to the client
            send( clientSocket, buffer, bytesReceived, 0 );
        }
    }
    pthread_mutex_lock( &mutex );

    // Open the file in read mode to send the content back to the client
    fd = open( DATA_FILE, O_RDWR );
    if ( fd == -1 )
    {
        // Log an error message if the file cannot be opened
        syslog( LOG_ERR, "Failed to open file %s: %s", DATA_FILE, strerror( errno ) );
        pthread_mutex_unlock( &mutex );
        close( clientSocket );
        threadInfo->threadComplete = true;
        pthread_exit( NULL );
    }

    // Sending the full content of the file to the client
    while ( ( bytesReceived = read( fd, buffer, sizeof( buffer ) ) ) > 0 )
    {
        send( clientSocket, buffer, bytesReceived, 0 );
    }

    // Close the file
    close( fd );
    pthread_mutex_unlock( &mutex );

    // Close the client socket
    close( clientSocket );

    // Log a message indicating the closed connection
    syslog( LOG_INFO, "Closed connection from %s", ipAddress );
    threadInfo->threadComplete = true;
    pthread_exit( NULL );
}

void *appendTimestamp ( void *arg )
{
    struct timespec currentTime;
    struct tm timeInfo;
    char timestamp[128];

    while ( 1 )
    {
        // Get current time with nanosecond precision
        if ( clock_gettime( CLOCK_REALTIME, &currentTime ) == -1 )
        {
            // Log an error if getting current time fails
            syslog( LOG_ERR, "Failed to get current time: %s", strerror( errno ) );
            pthread_exit( NULL );
        }

        // Convert the timespec to tm structure
        if ( localtime_r( &currentTime.tv_sec, &timeInfo ) == NULL )
        {
            // Log an error if converting time fails
            syslog( LOG_ERR, "Failed to convert time: %s", strerror( errno ) );
            pthread_exit( NULL );
        }

        // Format the timestamp string
        strftime( timestamp, sizeof( timestamp ), "timestamp:%a, %d %b %Y %T %z", &timeInfo );

        pthread_mutex_lock( &mutex );
        // Open the file in append mode
        filePointer = fopen( DATA_FILE, "a" );
        if ( filePointer == NULL )
        {
            // Log an error if opening file fails
            syslog( LOG_ERR, "Failed to open file %s: %s", DATA_FILE, strerror( errno ) );
            pthread_mutex_unlock( &mutex );
            pthread_exit( NULL );
        }

        // Write the timestamp to the file
        size_t len = strlen( timestamp );
        if ( fwrite( timestamp, 1, len, filePointer ) != len )
        {
            // Log an error if writing timestamp fails
            syslog( LOG_ERR, "Failed to write timestamp to file" );
            fclose( filePointer );
            pthread_mutex_unlock( &mutex );
            pthread_exit( NULL );
        }

        // Write newline character
        if ( fwrite( "\n", 1, 1, filePointer ) != 1 )
        {
            // Log an error if writing newline character fails
            syslog( LOG_ERR, "Failed to write newline character to file" );
            fclose( filePointer );
            pthread_mutex_unlock( &mutex );
            pthread_exit( NULL );
        }

        fclose( filePointer );
        pthread_mutex_unlock( &mutex );

        // Sleep for 10 seconds
        struct timespec sleepTime = {10, 0}; // 10 seconds
        nanosleep( &sleepTime, NULL );
    }
}

// Main function
int main ( int argc, char *argv[] )
{
    // Open the syslog with specified options
    openlog( "aesdsocket", LOG_PID | LOG_CONS, LOG_USER );

#if ( USE_AESD_CHAR_DEVICE == 0 )
    // Remove the data file if it exists
    if ( remove( DATA_FILE ) == -1 && errno != ENOENT )
    {
        // Log an error message if the file cannot be removed
        syslog( LOG_ERR, "Failed to remove file %s: %s", DATA_FILE, strerror( errno ) );
        closelog();
        exit( -1 );
    }
#endif
    // Variable to determine if the program runs in daemon mode
    bool isDaemonMode = false;

    // Check if the program is running in daemon mode
    if ( argc > 1 && strcmp( argv[1], "-d" ) == 0 )
    {
        isDaemonMode = true;
    }

    // Register signal handlers for SIGINT and SIGTERM
    if ( signal( SIGINT, signalHandler ) == SIG_ERR || signal( SIGTERM, signalHandler ) == SIG_ERR )
    {
        // Log an error message if signal handlers cannot be registered
        syslog( LOG_ERR, "Failed to register signal handler: %s", strerror( errno ) );
        closelog();
        exit( -1 );
    }

    // Initialize the head of the list
    SLIST_INIT( &threadHead );

    // Declare variables for address info
    struct addrinfo hints, *serviceAddr, *p;

    // Initialize hints structure
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get address info
    int status;
    if ( ( status = getaddrinfo( NULL, "9000", &hints, &serviceAddr ) ) != 0 )
    {
        // Log an error message if address info cannot be obtained
        syslog( LOG_ERR, "Failed to get address info: %s", strerror( errno ) );
        closelog();
        exit( -1 );
    }

    // Initialize SQLite database
    if ( sqlite3_open( DATABASE_FILE, &db ) )
    {
        syslog( LOG_ERR, "Can't open database: %s", sqlite3_errmsg( db ) );
        sqlite3_close( db );
        closelog();
        exit( 1 );
    }

    // Flag to check if binding is successful
    bool isBindingSuccessful = false;

    // Iterate through the address info and bind to the first available address
    for ( p = serviceAddr; p != NULL; p = p->ai_next )
    {
        // Create socket
        serverSocket = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
        if ( serverSocket == -1 )
        {
            // Log an error message if socket creation fails
            syslog( LOG_ERR, "Failed to create socket: %s", strerror( errno ) );
            continue;
        }

        // Set socket options
        if ( setsockopt( serverSocket, SOL_SOCKET, SO_REUSEADDR, &( int ){1}, sizeof( int ) ) == -1 )
        {
            // Log an error message if setting socket options fails
            syslog( LOG_ERR, "Failed to set socket options: %s", strerror( errno ) );
            close( serverSocket );
            continue;
        }

        // Bind to the address
        if ( bind( serverSocket, p->ai_addr, p->ai_addrlen ) == -1 )
        {
            // Log an error message if binding fails
            syslog( LOG_ERR, "Failed to bind: %s", strerror( errno ) );
            close( serverSocket );
            continue;
        }

        // Set the flag to true if binding is successful
        isBindingSuccessful = true;
        break;
    }

    // Free the address info
    freeaddrinfo( serviceAddr );

    // Check if binding was successful
    if ( !isBindingSuccessful )
    {
        // Log an error message if binding fails
        syslog( LOG_ERR, "Failed to bind to any address" );
        close( serverSocket );
        closelog();
        exit( -1 );
    }

    // If running in daemon mode, fork the process
    if ( isDaemonMode )
    {
        pid_t pid = fork();
        if ( pid == -1 )
        {
            // Log an error message if forking fails
            close( serverSocket );
            closelog();
            exit( -1 );
        }
        else if ( pid != 0 )
        {
            // If parent process, exit successfully
            close( serverSocket );
            closelog();
            exit( 0 );
        }
    }

    // Start listening for incoming connections
    if ( listen( serverSocket, 10 ) == -1 )
    {
        // Log an error message if listening fails
        syslog( LOG_ERR, "Failed to listen: %s", strerror( errno ) );
        closelog();
        exit( -1 );
    }

#if ( USE_AESD_CHAR_DEVICE == 0 )
    // Create thread for appending timestamp
    if ( pthread_create( &timestampThread, NULL, appendTimestamp, NULL ) != 0 )
    {
        // Log an error message if creating timestamp thread fails
        syslog( LOG_ERR, "Failed to create timestamp thread" );
        closelog();
        exit( -1 );
    }
#endif

    while ( 1 )
    {
        // Accept and handle incoming client connections
        struct sockaddr_in clientAddr;
        socklen_t addrSize = sizeof( clientAddr );

        int clientSocket = accept( serverSocket, ( struct sockaddr * ) &clientAddr, &addrSize );
        if ( clientSocket == -1 )
        {
            // Log an error message if accepting connection fails
            syslog( LOG_ERR, "Failed to accept: %s", strerror( errno ) );
            continue;
        }

        // Create a new thread info structure
        struct ThreadInfo *threadInfo = ( struct ThreadInfo * )malloc( sizeof( struct ThreadInfo ) );
        if ( threadInfo == NULL )
        {
            syslog( LOG_ERR, "Failed to allocate memory" );
            close( clientSocket );
            continue;
        }

        threadInfo->clientSocket = clientSocket;
        threadInfo->threadComplete = false;
        // Create thread to handle client
        if ( pthread_create( &threadInfo->threadId, NULL, handleClient, threadInfo ) != 0 )
        {
            syslog( LOG_ERR, "Failed to create client handling thread" );
            close( clientSocket );
            free( threadInfo );
            continue;
        }

        // Insert the thread info structure into the list
        SLIST_INSERT_HEAD( &threadHead, threadInfo, entries );

        // Join complete threads
        struct ThreadInfo *currentThread, *nextThread;
        SLIST_FOREACH_SAFE( currentThread, &threadHead, entries, nextThread )
        {
            if ( currentThread->threadComplete )
            {
                pthread_join( currentThread->threadId, NULL );
                SLIST_REMOVE( &threadHead, currentThread, ThreadInfo, entries );
                free( currentThread );
            }
        }
    }
}