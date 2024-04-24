import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QTabWidget
from PyQt5.QtCore import QTimer
import sqlite3
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from datetime import datetime
import socket

class GraphWidget( QWidget ):
    def __init__( self, parent=None ):
        super().__init__( parent )

        # Create a Figure object
        self.figure = plt.figure()

        # Create a Canvas object
        self.canvas = FigureCanvas( self.figure )

        # Create a vertical layout
        layout = QVBoxLayout()
        layout.addWidget( self.canvas )
        self.setLayout( layout )

    def plot_temperature( self, timestamps, temperatures ):
        # Clear previous plot
        self.figure.clear()

        # Add subplot for temperature
        ax = self.figure.add_subplot( 111 )

        # Plot temperature data
        ax.plot( timestamps, temperatures, label='Temperature', color='red' )

        # Set labels and title
        ax.set_xlabel( 'Timestamp' )
        ax.set_ylabel( 'Temperature ( °C )' )
        ax.set_title( 'Temperature Data' )
        ax.legend()

        # Rotate x-axis labels for better readability
        ax.tick_params( axis='x', rotation=45 )

        # Adjust margins for better layout
        self.figure.subplots_adjust( left=0.1, right=0.9, top=0.9, bottom=0.2 )

        # Draw the plot
        self.canvas.draw()

    def plot_humidity( self, timestamps, humidities ):
        # Clear previous plot
        self.figure.clear()

        # Add subplot for humidity
        ax = self.figure.add_subplot( 111 )

        # Plot humidity data
        ax.plot( timestamps, humidities, label='Humidity', color='blue' )

        # Set labels and title
        ax.set_xlabel( 'Timestamp' )
        ax.set_ylabel( 'Humidity ( % )' )
        ax.set_title( 'Humidity Data' )
        ax.legend()

        # Rotate x-axis labels for better readability
        ax.tick_params( axis='x', rotation=45 )

        # Adjust margins for better layout
        self.figure.subplots_adjust( left=0.1, right=0.9, top=0.9, bottom=0.2 )

        # Draw the plot
        self.canvas.draw()

    def plot_pressure( self, timestamps, pressures ):
        # Clear previous plot
        self.figure.clear()

        # Add subplot for pressure
        ax = self.figure.add_subplot( 111 )

        # Plot pressure data
        ax.plot( timestamps, pressures, label='Pressure', color='green' )

        # Set labels and title
        ax.set_xlabel( 'Timestamp' )
        ax.set_ylabel( 'Pressure ( Pa )' )
        ax.set_title( 'Pressure Data' )
        ax.legend()

        # Rotate x-axis labels for better readability
        ax.tick_params( axis='x', rotation=45 )

        # Adjust margins for better layout
        self.figure.subplots_adjust( left=0.1, right=0.9, top=0.9, bottom=0.2 )

        # Draw the plot
        self.canvas.draw()


    def plot_data( self, timestamps, temperatures, humidities, pressures ):
        # Clear previous plot
        self.figure.clear()

        # Add primary subplot for temperature and humidity
        ax1 = self.figure.add_subplot( 111 )

        # Plot temperature and humidity data
        ax1.plot( timestamps, temperatures, label='Temperature', color='red' )
        ax1.plot( timestamps, humidities, label='Humidity', color='blue' )

        # Set labels and title for primary y-axis
        ax1.set_xlabel( 'Timestamp' )
        ax1.set_ylabel( 'Temperature ( °C ) / Humidity ( % )' )
        ax1.set_title( 'Sensor Data' )
        ax1.legend( loc='upper left' )

        # Create secondary y-axis for pressure
        ax2 = ax1.twinx()
        ax2.plot( timestamps, pressures, label='Pressure', color='green' )
        ax2.set_ylabel( 'Pressure ( Pa )' )
        ax2.legend( loc='upper right' )

        # Rotate x-axis labels for better readability
        ax1.tick_params( axis='x', rotation=45 )  # Rotate x-axis labels
        ax1.xaxis.set_tick_params( labelrotation=45 )  # Adjust the rotation of x-axis labels

        # Adjust margins to add padding to the outside of the graph
        self.figure.subplots_adjust( left=0.1, right=0.9, top=0.9, bottom=0.2 )

        # Draw the plot
        self.canvas.draw()


class MainWindow( QMainWindow ):
    def __init__( self ):
        super().__init__()
        self.initUI()
        self.updateScreen()
        # Create a QTimer instance to update the screen periodically
        self.timer = QTimer( self )
        self.timer.timeout.connect( self.updateScreen )  # Connect timeout signal to function
        self.timer.start( 15000 )  # Start timer with a timeout interval of 15,000 milliseconds ( 15 seconds )


class MainWindow( QMainWindow ):
    def __init__( self, ip_address ):
        super().__init__()
        self.ip_address = ip_address
        self.initUI()
        self.updateScreen()
        # Create a QTimer instance to update the screen periodically
        self.timer = QTimer( self )
        self.timer.timeout.connect( self.updateScreen )  # Connect timeout signal to function
        self.timer.start( 15000 )  # Start timer with a timeout interval of 15,000 milliseconds ( 15 seconds )

    def initUI( self ):
        self.setGeometry( 100, 100, 800, 600 )  # Set window size and position
        self.setWindowTitle( 'MonitorMasterHW' )  # Set window title

        # Create a tab widget
        self.tabWidget = QTabWidget( self )

        # Create tabs for temperature, humidity, and pressure
        self.tab_temperature = QWidget()
        self.tab_humidity = QWidget()
        self.tab_pressure = QWidget()
        self.tab_all = QWidget()

        # Add tabs to the tab widget
        self.tabWidget.addTab( self.tab_temperature, "Temperature" )
        self.tabWidget.addTab( self.tab_humidity, "Humidity" )
        self.tabWidget.addTab( self.tab_pressure, "Pressure" )
        self.tabWidget.addTab( self.tab_all, "All" )

        # Create layouts for each tab
        self.layout_temperature = QVBoxLayout( self.tab_temperature )
        self.layout_humidity = QVBoxLayout( self.tab_humidity )
        self.layout_pressure = QVBoxLayout( self.tab_pressure )
        self.layout_all = QVBoxLayout( self.tab_all )

        # Create a GraphWidget for each tab
        self.graphWidget_temperature = GraphWidget( self.tab_temperature )
        self.graphWidget_humidity = GraphWidget( self.tab_humidity )
        self.graphWidget_pressure = GraphWidget( self.tab_pressure )
        self.graphWidget_all = GraphWidget( self.tab_all )

        # Add GraphWidget to the layout of each tab
        self.layout_temperature.addWidget( self.graphWidget_temperature )
        self.layout_humidity.addWidget( self.graphWidget_humidity )
        self.layout_pressure.addWidget( self.graphWidget_pressure )
        self.layout_all.addWidget( self.graphWidget_all )

        # Set the central widget as the tab widget
        self.setCentralWidget( self.tabWidget )

        # Connect tab change signal to updateScreen method
        self.tabWidget.currentChanged.connect( self.updateScreen )

    def updateScreen( self ):
        # Determine which tab is selected
        current_tab_index = self.tabWidget.currentIndex()

        # Make socket call to 10.0.0.160 port 9000 with the command "get10"
        with socket.socket( socket.AF_INET, socket.SOCK_STREAM ) as s:
            s.connect( ( self.ip_address, 9000 ) )
            s.sendall( b'get10' )
            received_data = s.recv( 1536 )

        # Parse received data and plot accordingly
        data_lines = received_data.decode( 'utf-8' ).split( '\n' )
        timestamps = []
        temperatures = []
        humidities = []
        pressures = []

        for line in data_lines:
            if line:
                parts = line.split( ',' )
                if len( parts ) == 4:  # Ensure there are enough parts in the line
                    try:
                        timestamp = int( parts[0].split( ':' )[1].strip() )  # Extract timestamp from the line
                        temperature = float( parts[1].split( ':' )[1].strip() )  # Extract temperature from the line
                        humidity = float( parts[2].split( ':' )[1].strip() )  # Extract humidity from the line
                        pressure = float( parts[3].split( ':' )[1].strip() )  # Extract pressure from the line

                        timestamps.append( timestamp )  # Append timestamp to timestamps list
                        temperatures.append( temperature )  # Append temperature to temperatures list
                        humidities.append( humidity )  # Append humidity to humidities list
                        pressures.append( pressure )  # Append pressure to pressures list
                    except ( IndexError, ValueError ) as e:
                        print( "Invalid data format:", line )  # Print error message for invalid data
                        continue
                else:
                    print( "Invalid data format:", line )  # Print error message for invalid data

        # Update the graph when the screen is updated
        if current_tab_index == 0:  # Temperature tab
            self.graphWidget_temperature.plot_temperature( timestamps, temperatures )
        elif current_tab_index == 1:  # Humidity tab
            self.graphWidget_humidity.plot_humidity( timestamps, humidities )
        elif current_tab_index == 2:  # Pressure tab
            self.graphWidget_pressure.plot_pressure( timestamps, pressures )
        elif current_tab_index == 3:  # All tab
            self.graphWidget_all.plot_data( timestamps, temperatures, humidities, pressures )





if __name__ == "__main__":
    # Check if IP address is provided as command-line argument
    if len( sys.argv ) < 2:
        print( "Usage: python script.py <IP_address>" )
        sys.exit( 1 )

    # Get IP address from command-line argument
    ip_address = sys.argv[1]

    app = QApplication( sys.argv )
    mainWindow = MainWindow( ip_address )
    mainWindow.show()
    sys.exit( app.exec_() )
