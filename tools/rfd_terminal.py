import sys
import json
import serial
import serial.tools.list_ports
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QLineEdit, QVBoxLayout, QWidget,
    QDialog, QDialogButtonBox, QLabel, QPushButton, QFileDialog, QHBoxLayout, QComboBox
)
from PyQt6.QtCore import QTimer

############################
# COM Port Selection Dialog
############################
class COMPortSelectionDialog(QDialog):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Select COM Port")
        layout = QVBoxLayout()
        layout.addWidget(QLabel("Select a COM port:"))
        self.combo = QComboBox()
        ports = serial.tools.list_ports.comports()
        if ports:
            for port in ports:
                self.combo.addItem(port.device)
        else:
            self.combo.addItem("No COM Ports Found")
        layout.addWidget(self.combo)
        self.okButton = QPushButton("OK")
        self.okButton.clicked.connect(self.accept)
        layout.addWidget(self.okButton)
        self.setLayout(layout)
    def selected_port(self):
        return self.combo.currentText()

############################
# Main Terminal Window
############################
class TerminalWindow(QMainWindow):
    def __init__(self, port, baudrate):
        super().__init__()
        self.setWindowTitle("Reefie Terminal")
        self.resize(600, 500)
        # Serial connection.
        self.serial = serial.Serial(port, baudrate, timeout=0.1)
        self.connected = False  # File management connection state.
        self.exporting = False
        self.fileData = b""
        # Terminal Widgets.
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.input = QLineEdit()
        self.input.returnPressed.connect(self.handleInput)
        # Layout.
        layout = QVBoxLayout()
        layout.addWidget(self.output)
        layout.addWidget(self.input)
        centralWidget = QWidget()
        centralWidget.setLayout(layout)
        self.setCentralWidget(centralWidget)
        # Timer to poll serial port.
        self.timer = QTimer()
        self.timer.timeout.connect(self.readSerial)
        self.timer.start(100)
        # Heartbeat status.
        self.prompt("Waiting for board to become available (FILE_MGMT_READY)...")
    def prompt(self, message):
        self.output.append(message)
    def handleInput(self):
        cmd = self.input.text().strip()
        self.input.clear()
        self.output.append(">> " + cmd)
        self.sendCommand(cmd)
    def sendCommand(self, command):
        if self.connected:
            self.serial.write((command + "\n").encode('utf-8'))
        else:
            self.output.append("Board not ready for file management.")
    def readSerial(self):
        if self.serial.in_waiting:
            try:
                data = self.serial.read(self.serial.in_waiting)
                text = data.decode('utf-8', errors='replace')
                self.output.append(text)
                # Look for heartbeat that indicates availability.
                if "FILE_MGMT_READY" in text:
                    if not self.connected:
                        self.connected = True
                        self.output.append("Board now available for file management commands.")
            except Exception as e:
                self.output.append(f"Error reading serial: {e}")
    # Optionally, you could add reconnect logic if the heartbeat stops.
    
if __name__ == '__main__':
    app = QApplication(sys.argv)
    comDialog = COMPortSelectionDialog()
    if comDialog.exec():
        selected_port = comDialog.selected_port()
    else:
        sys.exit("No COM port selected.")
    window = TerminalWindow(selected_port, 115200)
    window.show()
    sys.exit(app.exec())
