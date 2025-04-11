import sys
import json
import serial
import serial.tools.list_ports
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QLineEdit, QVBoxLayout, QWidget,
    QDialog, QDialogButtonBox, QLabel, QPushButton, QFileDialog, QHBoxLayout,
    QComboBox, QFormLayout
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

#######################################
# Flight Parameters Configuration Dialog
#######################################
class FlightParamsDialog(QDialog):
    def __init__(self, params_path="flight_params.json"):
        super().__init__()
        self.setWindowTitle("Configure Flight Parameters")
        self.params_path = params_path
        self.params = self.load_params()

        self.layout = QVBoxLayout()
        self.formLayout = QFormLayout()

        # Create line edits for numeric parameters.
        self.accel_edit = QLineEdit(str(self.params.get("ACCEL_THRESHOLD", "")))
        self.light_edit = QLineEdit(str(self.params.get("LIGHT_THRESHOLD", "")))
        self.disreef_alt_edit = QLineEdit(str(self.params.get("DISREEF_ALT", "")))
        self.veloc_edit = QLineEdit(str(self.params.get("VELOC_THRESHOLD", "")))
        self.poll_freq_edit = QLineEdit(str(self.params.get("POLL_FREQ", "")))

        self.formLayout.addRow("Accel Threshold:", self.accel_edit)
        self.formLayout.addRow("Light Threshold:", self.light_edit)
        self.formLayout.addRow("Disreef Alt:", self.disreef_alt_edit)
        self.formLayout.addRow("Velocity Threshold:", self.veloc_edit)
        self.formLayout.addRow("Poll Frequency:", self.poll_freq_edit)

        # Create checkboxes for boolean 'log' parameters.
        self.log_checkboxes = {}
        log_params = self.params.get("log", {})
        for key, value in log_params.items():
            checkbox = checkbox()
            checkbox.setChecked(value)
            self.log_checkboxes[key] = checkbox
            self.formLayout.addRow(f"Log {key}:", checkbox)

        self.layout.addLayout(self.formLayout)

        # Standard dialog buttons.
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        self.buttonBox.accepted.connect(self.save_params)
        self.buttonBox.rejected.connect(self.reject)
        self.layout.addWidget(self.buttonBox)

        self.setLayout(self.layout)

    def load_params(self):
        try:
            with open(self.params_path, "r") as f:
                return json.load(f)
        except Exception as e:
            print("Error loading parameters:", e)
            return {}

    def save_params(self):
        try:
            self.params["ACCEL_THRESHOLD"] = float(self.accel_edit.text())
            self.params["LIGHT_THRESHOLD"] = int(self.light_edit.text())
            self.params["DISREEF_ALT"] = float(self.disreef_alt_edit.text())
            self.params["VELOC_THRESHOLD"] = float(self.veloc_edit.text())
            self.params["POLL_FREQ"] = int(self.poll_freq_edit.text())

            for key, checkbox in self.log_checkboxes.items():
                self.params.setdefault("log", {})[key] = checkbox.isChecked()

            with open(self.params_path, "w") as f:
                json.dump(self.params, f, indent=4)
            self.accept()
        except Exception as e:
            print("Error saving parameters:", e)
            self.reject()

####################################
# Main Terminal Window with Extras
####################################
class TerminalWindow(QMainWindow):
    def __init__(self, port, baudrate):
        super().__init__()
        self.setWindowTitle("Reefie Terminal")
        self.resize(600, 500)

        # Serial connection.
        self.serial = serial.Serial(port, baudrate, timeout=0.1)
        self.exporting = False
        self.fileData = b""

        # Terminal Widgets.
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.input = QLineEdit()
        self.input.returnPressed.connect(self.handleInput)

        # Button to open flight parameters configuration.
        self.configButton = QPushButton("Configure Flight Params")
        self.configButton.clicked.connect(self.openFlightParamsDialog)

        # Layout: output, input and config button.
        layout = QVBoxLayout()
        layout.addWidget(self.output)
        layout.addWidget(self.input)
        layout.addWidget(self.configButton)

        centralWidget = QWidget()
        centralWidget.setLayout(layout)
        self.setCentralWidget(centralWidget)

        # Timer to poll serial port.
        self.timer = QTimer()
        self.timer.timeout.connect(self.readSerial)
        self.timer.start(100)

        self.prompt("Type LIST to list Reefie files.\n" +
                    "Other commands include EXPORT <num>, DELETE <num>, GETPARAMS, and SETPARAMS commands.")

    def prompt(self, message):
        self.output.append(message)

    def handleInput(self):
        cmd = self.input.text().strip()
        self.input.clear()
        self.output.append(">> " + cmd)
        self.sendCommand(cmd)

    def sendCommand(self, command):
        # Send command to MCU.
        self.serial.write((command + "\n").encode('utf-8'))
        # If sending updated flight params, the protocol might be:
        # SETPARAMS_BEGIN, then the JSON contents, then SETPARAMS_END.
        # That can be triggered from the flight params dialog if needed.

    def readSerial(self):
        if self.serial.in_waiting:
            try:
                data = self.serial.read(self.serial.in_waiting)
                text = data.decode('utf-8', errors='replace')
                self.output.append(text)

                # If exporting file data, handle markers.
                if "EXPORT_BEGIN" in text:
                    self.exporting = True
                    self.fileData = b""
                if self.exporting:
                    self.fileData += data
                    if "EXPORT_END" in text:
                        self.exporting = False
                        full_text = self.fileData.decode('utf-8', errors='replace')
                        start = full_text.find("EXPORT_BEGIN")
                        end = full_text.find("EXPORT_END")
                        if start != -1 and end != -1:
                            content = full_text[start:end].split('\n', 1)
                            file_content = content[1].strip() if len(content) > 1 else ""
                            save_path, _ = QFileDialog.getSaveFileName(
                                self, "Save Exported File", "", "CSV Files (*.csv);;All Files (*)"
                            )
                            if save_path:
                                with open(save_path, "w", encoding="utf-8") as f:
                                    f.write(file_content)
                                self.output.append(f"File saved to: {save_path}")
                        self.prompt("Export complete.")
            except Exception as e:
                self.output.append(f"Error reading serial: {e}")

    def openFlightParamsDialog(self):
        dialog = FlightParamsDialog("flight_params.json")
        if dialog.exec():
            self.output.append("Flight parameters updated locally.")
            # Optionally, send new parameters to the MCU.
            try:
                with open("flight_params.json", "r") as f:
                    params_str = f.read()
                # Send with markers so the MCU can update its file.
                self.sendCommand("SETPARAMS_BEGIN")
                # You might want to split the JSON over several sends depending on size.
                self.sendCommand(params_str)
                self.sendCommand("SETPARAMS_END")
                self.output.append("Flight parameters sent to MCU.")
            except Exception as e:
                self.output.append(f"Error sending flight parameters: {e}")
        else:
            self.output.append("Flight parameters update cancelled.")

if __name__ == '__main__':
    app = QApplication(sys.argv)

    # Show COM port selection dialog.
    comDialog = COMPortSelectionDialog()
    if comDialog.exec():
        selected_port = comDialog.selected_port()
    else:
        sys.exit("No COM port selected.")

    # Create and show the terminal window.
    window = TerminalWindow(selected_port, 115200)
    window.show()
    sys.exit(app.exec())
