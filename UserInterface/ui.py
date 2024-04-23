import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QTabWidget
from PyQt5.QtCore import QTimer
import sqlite3
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from datetime import datetime


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
    def __init__( self ):
        super().__init__()
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

        # Update the graph when the screen is updated
        conn = sqlite3.connect( 'finalProject.db' )
        cursor = conn.cursor()
        # Execute a query to retrieve the last entries from the 'sensor_data' table
        cursor.execute( 'SELECT timestamp, temperature, humidity, pressure FROM sensor_data ORDER BY timestamp DESC LIMIT 10' )
        data = cursor.fetchall()
        conn.close()

        if data:
            # Extract timestamp and data for the selected tab
            timestamps = [datetime.fromtimestamp( row[0] ) for row in data]
            if current_tab_index == 0:  # Temperature tab
                temperatures = [row[1] for row in data]
                self.graphWidget_temperature.plot_temperature( timestamps, temperatures )
            elif current_tab_index == 1:  # Humidity tab
                humidities = [row[2] for row in data]
                self.graphWidget_humidity.plot_humidity( timestamps, humidities )
            elif current_tab_index == 2:  # Pressure tab
                pressures = [row[3] for row in data]
                self.graphWidget_pressure.plot_pressure( timestamps, pressures )
            elif current_tab_index == 3:  # All tab
                temperatures = [row[1] for row in data]
                humidities = [row[2] for row in data]
                pressures = [row[3] for row in data]
                self.graphWidget_all.plot_data( timestamps, temperatures, humidities, pressures )

if __name__ == "__main__":
    app = QApplication( sys.argv )
    mainWindow = MainWindow()
    mainWindow.show()
    sys.exit( app.exec_() )
