"""
OpCode Labs – Serial LED Dashboard
Frame format: [SOF=0x65][CMD=1B][CONFIG=2B][DATA=4B] = 8 bytes total

Commands:
  SET_LED1 = 0x01  →  CONFIG[0] = LED index(0), CONFIG[1] = toggle freq (Hz)
  SET_LED2 = 0x02  →  CONFIG = 0x0000
  SET_LED3 = 0x03  →  CONFIG = 0x0000
  DATA field = 4-byte payload (e.g. ON/OFF state in byte 0)
"""

import sys
import struct
import datetime
import serial
import serial.tools.list_ports

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QComboBox, QTextEdit, QGroupBox,
    QFrame, QSizePolicy, QStatusBar, QDoubleSpinBox, QLineEdit
)
from PyQt5.QtGui import QRegExpValidator
from PyQt5.QtCore import QRegExp
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QRegExp
from PyQt5.QtGui import QFont, QColor, QPalette, QTextCharFormat, QBrush, QRegExpValidator


# ─── Frame Builder ────────────────────────────────────────────────────────────

SOF = 0x41

CMD_SET_LED1 = 0x31
CMD_SET_LED2 = 0xD2
CMD_SET_LED3 = 0xD3


def build_frame(cmd: int, config: int, data: int) -> bytes:
    """Pack: SOF(1) + CMD(1) + CONFIG(2,LE) + DATA(4,LE) = 8 bytes"""
    return struct.pack('<BBHI', SOF, cmd, config, data)


# ─── Serial Reader Thread ──────────────────────────────────────────────────────

class SerialReader(QThread):
    data_received = pyqtSignal(bytes)
    error_occurred = pyqtSignal(str)

    def __init__(self, port: serial.Serial):
        super().__init__()
        self._port = port
        self._running = True

    def run(self):
        while self._running:
            try:
                if self._port.is_open and self._port.in_waiting:
                    raw = self._port.read(self._port.in_waiting)
                    if raw:
                        self.data_received.emit(raw)
                self.msleep(20)
            except serial.SerialException as e:
                self.error_occurred.emit(str(e))
                break

    def stop(self):
        self._running = False
        self.wait()


# ─── Main Window ──────────────────────────────────────────────────────────────

DARK_BG    = "#1E1E2E"
PANEL_BG   = "#252535"
BORDER     = "#3A3A5C"
ACCENT     = "#7C6AF7"
ACCENT2    = "#5BC8AF"
LED1_COLOR = "#F9A825"
LED2_COLOR = "#42A5F5"
LED3_COLOR = "#EF5350"
TEXT_MAIN  = "#E0E0F0"
TEXT_DIM   = "#8888AA"
SUCCESS    = "#66BB6A"
DANGER     = "#EF5350"


STYLE = f"""
QMainWindow, QWidget {{
    background-color: {DARK_BG};
    color: {TEXT_MAIN};
    font-family: 'Consolas', 'Courier New', monospace;
}}
QGroupBox {{
    border: 1px solid {BORDER};
    border-radius: 8px;
    margin-top: 10px;
    padding: 8px;
    font-size: 11px;
    color: {TEXT_DIM};
}}
QGroupBox::title {{
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 0 6px;
    color: {ACCENT};
    font-weight: bold;
}}
QPushButton {{
    border-radius: 6px;
    padding: 8px 16px;
    font-size: 12px;
    font-weight: bold;
    border: none;
}}
QPushButton#btn_led1 {{
    background-color: {LED1_COLOR};
    color: #1a1a1a;
}}
QPushButton#btn_led1:hover  {{ background-color: #FFD54F; }}
QPushButton#btn_led1:pressed {{ background-color: #F57F17; }}
QPushButton#btn_led2 {{
    background-color: {LED2_COLOR};
    color: #1a1a1a;
}}
QPushButton#btn_led2:hover  {{ background-color: #90CAF9; }}
QPushButton#btn_led2:pressed {{ background-color: #1565C0; }}
QPushButton#btn_led3 {{
    background-color: {LED3_COLOR};
    color: #fff;
}}
QPushButton#btn_led3:hover  {{ background-color: #EF9A9A; }}
QPushButton#btn_led3:pressed {{ background-color: #B71C1C; }}
QPushButton#btn_connect {{
    background-color: {SUCCESS};
    color: #1a1a1a;
    min-width: 100px;
}}
QPushButton#btn_connect:hover {{ background-color: #A5D6A7; }}
QPushButton#btn_disconnect {{
    background-color: {DANGER};
    color: #fff;
    min-width: 100px;
}}
QPushButton#btn_disconnect:hover {{ background-color: #EF9A9A; }}
QPushButton#btn_clear {{
    background-color: {BORDER};
    color: {TEXT_MAIN};
}}
QPushButton#btn_clear:hover {{ background-color: #555580; }}
QPushButton#btn_refresh {{
    background-color: {PANEL_BG};
    color: {ACCENT2};
    border: 1px solid {ACCENT2};
    padding: 4px 10px;
    font-size: 11px;
}}
QPushButton:disabled {{
    background-color: #333350;
    color: #555570;
}}
QComboBox, QSpinBox, QDoubleSpinBox {{
    background-color: {PANEL_BG};
    color: {TEXT_MAIN};
    border: 1px solid {BORDER};
    border-radius: 4px;
    padding: 4px 8px;
    font-size: 12px;
}}
QComboBox::drop-down {{ border: none; }}
QComboBox QAbstractItemView {{
    background-color: {PANEL_BG};
    color: {TEXT_MAIN};
    selection-background-color: {ACCENT};
}}
QTextEdit {{
    background-color: #12121E;
    color: {ACCENT2};
    border: 1px solid {BORDER};
    border-radius: 6px;
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 11px;
    padding: 6px;
}}
QLabel#lbl_status_dot {{
    font-size: 16px;
}}
QStatusBar {{
    background-color: {PANEL_BG};
    color: {TEXT_DIM};
    font-size: 11px;
}}
"""


class Dashboard(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OpCode Labs — Serial LED Dashboard")
        self.setMinimumSize(760, 580)

        self._serial: serial.Serial | None = None
        self._reader: SerialReader | None  = None
        self._led_states = [False, False, False]   # toggle state per LED
        self._frame_count = 0

        self._build_ui()
        self.setStyleSheet(STYLE)
        self._refresh_ports()
        self._set_connected(False)

    # ── UI Construction ────────────────────────────────────────────────────

    def _build_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        root = QVBoxLayout(central)
        root.setSpacing(10)
        root.setContentsMargins(14, 14, 14, 10)

        # ── Connection bar ──
        conn_box = QGroupBox("Serial Connection")
        conn_layout = QHBoxLayout(conn_box)
        conn_layout.setSpacing(8)

        self.cmb_port = QComboBox()
        self.cmb_port.setMinimumWidth(130)

        self.cmb_baud = QComboBox()
        for b in ["9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"]:
            self.cmb_baud.addItem(b)
        self.cmb_baud.setCurrentText("115200")
        self.cmb_baud.setMinimumWidth(90)

        btn_refresh = QPushButton("⟳ Scan")
        btn_refresh.setObjectName("btn_refresh")
        btn_refresh.clicked.connect(self._refresh_ports)

        self.btn_connect = QPushButton("Connect")
        self.btn_connect.setObjectName("btn_connect")
        self.btn_connect.clicked.connect(self._connect)

        self.btn_disconnect = QPushButton("Disconnect")
        self.btn_disconnect.setObjectName("btn_disconnect")
        self.btn_disconnect.clicked.connect(self._disconnect)

        self.lbl_dot = QLabel("●")
        self.lbl_dot.setObjectName("lbl_status_dot")
        self.lbl_conn_text = QLabel("Disconnected")
        self.lbl_conn_text.setStyleSheet(f"color:{TEXT_DIM}; font-size:12px;")

        conn_layout.addWidget(QLabel("Port:"))
        conn_layout.addWidget(self.cmb_port)
        conn_layout.addWidget(QLabel("Baud:"))
        conn_layout.addWidget(self.cmb_baud)
        conn_layout.addWidget(btn_refresh)
        conn_layout.addSpacing(12)
        conn_layout.addWidget(self.btn_connect)
        conn_layout.addWidget(self.btn_disconnect)
        conn_layout.addSpacing(16)
        conn_layout.addWidget(self.lbl_dot)
        conn_layout.addWidget(self.lbl_conn_text)
        conn_layout.addStretch()
        root.addWidget(conn_box)

        # ── LED Controls ──
        leds_box = QGroupBox("LED Commands")
        leds_layout = QHBoxLayout(leds_box)
        leds_layout.setSpacing(12)

        # LED1
        led1_frame = self._make_led_panel(
            "LED 1", LED1_COLOR, "btn_led1",
            has_freq=True
        )
        # LED2
        led2_frame = self._make_led_panel("LED 2", LED2_COLOR, "btn_led2")
        # LED3
        led3_frame = self._make_led_panel("LED 3", LED3_COLOR, "btn_led3")

        leds_layout.addWidget(led1_frame, 1)
        leds_layout.addWidget(led2_frame, 1)
        leds_layout.addWidget(led3_frame, 1)
        root.addWidget(leds_box)

        # ── Log ──
        log_box = QGroupBox("Frame Log")
        log_layout = QVBoxLayout(log_box)

        log_hdr = QHBoxLayout()
        self.lbl_frame_count = QLabel("Frames TX: 0")
        self.lbl_frame_count.setStyleSheet(f"color:{TEXT_DIM}; font-size:11px;")
        btn_clear = QPushButton("Clear")
        btn_clear.setObjectName("btn_clear")
        btn_clear.setFixedWidth(70)
        btn_clear.clicked.connect(self._clear_log)
        log_hdr.addWidget(self.lbl_frame_count)
        log_hdr.addStretch()
        log_hdr.addWidget(btn_clear)
        log_layout.addLayout(log_hdr)

        self.log = QTextEdit()
        self.log.setReadOnly(True)
        self.log.setMinimumHeight(180)
        log_layout.addWidget(self.log)
        root.addWidget(log_box, 1)

        # ── Status bar ──
        self.statusBar().showMessage("Ready — select a COM port and connect.")

    def _make_led_panel(self, title, color, obj_name, has_freq=False):
        frame = QFrame()
        frame.setStyleSheet(
            f"QFrame {{ background:{PANEL_BG}; border:1px solid {BORDER}; border-radius:8px; padding:8px; }}"
        )
        layout = QVBoxLayout(frame)
        layout.setSpacing(6)

        # Coloured title label
        lbl = QLabel(title)
        lbl.setAlignment(Qt.AlignCenter)
        lbl.setStyleSheet(f"color:{color}; font-size:14px; font-weight:bold; border:none;")
        layout.addWidget(lbl)

        idx = int(title[-1]) - 1   # 0,1,2

        if has_freq:
            freq_row = QHBoxLayout()
            freq_lbl = QLabel("Freq (Hz):")
            freq_lbl.setStyleSheet(f"color:{TEXT_DIM}; font-size:11px; border:none;")
            self.spin_freq = QDoubleSpinBox()
            self.spin_freq.setRange(0.1, 100.0)
            self.spin_freq.setSingleStep(0.5)
            self.spin_freq.setValue(1.0)
            self.spin_freq.setDecimals(1)
            self.spin_freq.setFixedWidth(80)
            freq_row.addWidget(freq_lbl)
            freq_row.addWidget(self.spin_freq)
            freq_row.addStretch()
            layout.addLayout(freq_row)

        # Data hex input
        data_row = QHBoxLayout()
        data_lbl = QLabel("Data (32-bit):")
        data_lbl.setStyleSheet(f"color:{TEXT_DIM}; font-size:11px; border:none;")
        hex_edit = QLineEdit("00000001")
        hex_edit.setMaxLength(8)
        hex_edit.setFixedWidth(100)
        hex_edit.setPlaceholderText("DEADBEEF")
        hex_edit.setValidator(QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,8}")))
        hex_edit.setStyleSheet(
            f"background:{PANEL_BG}; color:{TEXT_MAIN}; border:1px solid {BORDER};"
            f"border-radius:4px; padding:4px 8px; font-size:12px; font-family:Consolas;"
        )
        # store reference
        setattr(self, f"hex_data{idx+1}", hex_edit)
        data_row.addWidget(data_lbl)
        data_row.addWidget(hex_edit)
        data_row.addStretch()
        layout.addLayout(data_row)

        btn = QPushButton(f"▶  SET LED{idx+1}")
        btn.setObjectName(obj_name)
        btn.clicked.connect(lambda _, i=idx: self._send_led(i))
        setattr(self, f"btn_led{idx+1}", btn)
        layout.addWidget(btn)

        return frame

    # ── Port helpers ───────────────────────────────────────────────────────

    def _refresh_ports(self):
        self.cmb_port.clear()
        ports = serial.tools.list_ports.comports()
        for p in sorted(ports):
            self.cmb_port.addItem(f"{p.device}  —  {p.description}", p.device)
        if not ports:
            self.cmb_port.addItem("No ports found")

    def _set_connected(self, state: bool):
        self.btn_connect.setEnabled(not state)
        self.btn_disconnect.setEnabled(state)
        self.cmb_port.setEnabled(not state)
        self.cmb_baud.setEnabled(not state)
        for i in range(1, 4):
            getattr(self, f"btn_led{i}").setEnabled(state)
        if state:
            self.lbl_dot.setStyleSheet(f"color:{SUCCESS}; font-size:16px;")
            self.lbl_conn_text.setText(f"Connected  [{self.cmb_port.currentData() or self.cmb_port.currentText().split()[0]}]")
            self.lbl_conn_text.setStyleSheet(f"color:{SUCCESS}; font-size:12px;")
        else:
            self.lbl_dot.setStyleSheet(f"color:{DANGER}; font-size:16px;")
            self.lbl_conn_text.setText("Disconnected")
            self.lbl_conn_text.setStyleSheet(f"color:{TEXT_DIM}; font-size:12px;")

    # ── Serial connect/disconnect ──────────────────────────────────────────

    def _connect(self):
        port = self.cmb_port.currentData() or self.cmb_port.currentText().split()[0]
        baud = int(self.cmb_baud.currentText())
        try:
            self._serial = serial.Serial(port, baud, timeout=0.1)
            self._reader = SerialReader(self._serial)
            self._reader.data_received.connect(self._on_rx)
            self._reader.error_occurred.connect(self._on_serial_error)
            self._reader.start()
            self._set_connected(True)
            self.statusBar().showMessage(f"Connected to {port} @ {baud} baud")
            self._log_info(f"[CONNECT] {port} @ {baud} baud")
        except serial.SerialException as e:
            self._log_error(f"[ERROR] Cannot open {port}: {e}")
            self.statusBar().showMessage(f"Error: {e}")

    def _disconnect(self):
        if self._reader:
            self._reader.stop()
            self._reader = None
        if self._serial and self._serial.is_open:
            self._serial.close()
        self._serial = None
        self._set_connected(False)
        self.statusBar().showMessage("Disconnected.")
        self._log_info("[DISCONNECT] Port closed")

    # ── Send frames ────────────────────────────────────────────────────────

    def _send_led(self, idx: int):
        if not self._serial or not self._serial.is_open:
            self._log_error("[TX ERROR] Not connected")
            return

        hex_edit = getattr(self, f"hex_data{idx+1}")
        try:
            data_val = int(hex_edit.text() or "0", 16) & 0xFFFFFFFF
        except ValueError:
            self._log_error("[TX ERROR] Invalid hex value in DATA field")
            return

        if idx == 0:           # LED1 — config encodes freq
            raw_freq = self.spin_freq.value()
            # encode as fixed-point ×10 in uint16 (0–6553.5 Hz range)
            freq_encoded = int(raw_freq * 10) & 0xFFFF
            config = freq_encoded
            cmd    = CMD_SET_LED1
        elif idx == 1:
            config = 0x0000
            cmd    = CMD_SET_LED2
        else:
            config = 0x0000
            cmd    = CMD_SET_LED3

        frame = build_frame(cmd, config, data_val)
        try:
            self._serial.write(frame)
            self._frame_count += 1
            self.lbl_frame_count.setText(f"Frames TX: {self._frame_count}")
            self._log_tx(cmd, config, data_val, frame)
        except serial.SerialException as e:
            self._log_error(f"[TX ERROR] {e}")

    # ── RX handler ────────────────────────────────────────────────────────

    def _on_rx(self, raw: bytes):
        ts = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
        hex_str = " ".join(f"{b:02X}" for b in raw)
        line = f"[{ts}]  RX  {len(raw):3d}B   {hex_str}"
        self._append_log(line, color=ACCENT2)

    def _on_serial_error(self, msg: str):
        self._log_error(f"[SERIAL ERROR] {msg}")
        self._disconnect()

    # ── Log helpers ───────────────────────────────────────────────────────

    def _log_tx(self, cmd, config, data, raw):
        ts  = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
        cmd_names = {CMD_SET_LED1: "SET_LED1", CMD_SET_LED2: "SET_LED2", CMD_SET_LED3: "SET_LED3"}
        hex_str = " ".join(f"{b:02X}" for b in raw)
        line = (
            f"[{ts}]  TX  {cmd_names.get(cmd, f'CMD={cmd:#04x}')}  "
            f"CONFIG={config:#06x}  DATA={data:#010x}   │  {hex_str}"
        )
        self._append_log(line, color=LED1_COLOR if cmd == CMD_SET_LED1
                                       else LED2_COLOR if cmd == CMD_SET_LED2
                                       else LED3_COLOR)

    def _log_info(self, msg):
        self._append_log(msg, color=TEXT_DIM)

    def _log_error(self, msg):
        self._append_log(msg, color=DANGER)

    def _append_log(self, text, color=None):
        cursor = self.log.textCursor()
        cursor.movePosition(cursor.End)
        fmt = QTextCharFormat()
        if color:
            fmt.setForeground(QBrush(QColor(color)))
        cursor.setCharFormat(fmt)
        cursor.insertText(text + "\n")
        self.log.setTextCursor(cursor)
        self.log.ensureCursorVisible()

    def _clear_log(self):
        self.log.clear()
        self._frame_count = 0
        self.lbl_frame_count.setText("Frames TX: 0")

    def closeEvent(self, event):
        self._disconnect()
        event.accept()


# ─── Entry point ──────────────────────────────────────────────────────────────

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    win = Dashboard()
    win.show()
    sys.exit(app.exec_())