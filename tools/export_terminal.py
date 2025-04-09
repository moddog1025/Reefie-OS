import sys
import serial
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QLineEdit, QVBoxLayout, QWidget,
    QDialog, QDialogButtonBox, QLabel, QPushButton, QFileDialog, QHBoxLayout
)
from PyQt6.QtCore import QTimer


class FileActionDialog(QDialog):
    def __init__(self, file_index):
        super().__init__()
        self.setWindowTitle("File Action")
        self.selectedAction = None

        layout = QVBoxLayout()
        label = QLabel(f"File number {file_index} selected. Choose action:")
        layout.addWidget(label)

        btnLayout = QHBoxLayout()
        exportBtn = QPushButton("EXPORT")
        deleteBtn = QPushButton("DELETE")
        exportBtn.clicked.connect(self.exportClicked)
        deleteBtn.clicked.connect(self.deleteClicked)
        btnLayout.addWidget(exportBtn)
        btnLayout.addWidget(deleteBtn)
        layout.addLayout(btnLayout)
        self.setLayout(layout)

    def exportClicked(self):
        self.selectedAction = "EXPORT"
        self.accept()

    def deleteClicked(self):
        self.selectedAction = "DELETE"
        self.accept()


class TerminalWindow(QMainWindow):
    def __init__(self, port, baudrate):
        super().__init__()
        self.setWindowTitle("Reefie Terminal")
        self.resize(600, 400)

        # Serial connection
        self.serial = serial.Serial(port, baudrate, timeout=0.1)
        self.exporting = False
        self.fileData = b""

        # Terminal Widgets
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.input = QLineEdit()
        self.input.returnPressed.connect(self.handleInput)

        layout = QVBoxLayout()
        layout.addWidget(self.output)
        layout.addWidget(self.input)

        centralWidget = QWidget()
        centralWidget.setLayout(layout)
        self.setCentralWidget(centralWidget)

        # Timer to poll serial port
        self.timer = QTimer()
        self.timer.timeout.connect(self.readSerial)
        self.timer.start(100)

        self.prompt("Type LIST to list Reefie files.")

    def prompt(self, message):
        self.output.append(message)

    def handleInput(self):
        cmd = self.input.text().strip()
        self.input.clear()
        self.output.append(">> " + cmd)

        # If the input is a number, pop up the file action dialog.
        if cmd.isdigit():
            file_index = int(cmd)
            dialog = FileActionDialog(file_index)
            if dialog.exec():
                action = dialog.selectedAction
                if action == "EXPORT":
                    self.sendCommand(f"EXPORT {file_index}")
                    # Prepare to capture export data.
                    self.exporting = True
                    self.fileData = b""
                elif action == "DELETE":
                    self.sendCommand(f"DELETE {file_index}")
        else:
            self.sendCommand(cmd)

    def sendCommand(self, command):
        self.serial.write((command + "\n").encode('utf-8'))

    def readSerial(self):
        if self.serial.in_waiting:
            try:
                data = self.serial.read(self.serial.in_waiting)
                text = data.decode('utf-8', errors='replace')
                self.output.append(text)

                # If exporting, collect file data.
                if "EXPORT_BEGIN" in text:
                    self.exporting = True
                    self.fileData = b""
                if self.exporting:
                    self.fileData += data
                    if "EXPORT_END" in text:
                        self.exporting = False
                        full_text = self.fileData.decode('utf-8', errors='replace')
                        # Extract content between EXPORT_BEGIN and EXPORT_END markers.
                        start = full_text.find("EXPORT_BEGIN")
                        end = full_text.find("EXPORT_END")
                        if start != -1 and end != -1:
                            # Assume file content is on the lines after EXPORT_BEGIN and before EXPORT_END.
                            content = full_text[start:end].split('\n', 1)
                            if len(content) > 1:
                                file_content = content[1].strip()
                            else:
                                file_content = ""
                            save_path, _ = QFileDialog.getSaveFileName(
                                self, "Save Exported File", "", "CSV Files (*.csv);;All Files (*)"
                            )
                            if save_path:
                                with open(save_path, "w", encoding="utf-8") as f:
                                    f.write(file_content)
                                self.output.append(f"File saved to: {save_path}")
            except Exception as e:
                self.output.append(f"Error reading serial: {e}")


if __name__ == '__main__':
    app = QApplication(sys.argv)

    # Change 'COM3' to the appropriate port where Reefie is connected.
    window = TerminalWindow("COM11", 115200)
    window.show()
    sys.exit(app.exec())
