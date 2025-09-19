📘 Bilgewater Level Monitor

Platform: M5Stack Core (ESP32)
Sensor: JSN-SR04T (ultrasonic distance sensor)
Goal: Measure the distance from the sensor (mounted at the top of the bilge/tank) down to the water surface, and display the resulting depth on the M5Stack LCD.

⚡️ Wiring
JSN-SR04T	M5Stack Core (ESP32)	Notes
5V	5V	Power
GND	GND	Common ground
TRIG	GPIO26	Output
ECHO	GPIO16	Input (through 10k/20k divider to drop 5 V → 3.3 V)

⚠️ The sensor has a blind zone of ~25 cm — anything closer gives unreliable results.
Mount it at least 25 cm above the highest water level.

⚙️ Features

Reads ultrasonic distance and converts to depth

Median filtering to remove outliers

Pegs any reading < 25 cm as “full” to avoid false values

Displays distance and depth on the M5Stack LCD

Configurable tank height (TANK_HEIGHT_CM in main.cpp)

🛠 Setup

Clone this repo

Open it in VS Code + PlatformIO

Connect your M5Stack Core

Click Upload

📁 Project Structure
.
├── platformio.ini
├── .gitignore
├── README.md
└── src/
    └── main.cpp

📌 Notes

Ping interval is set to 1 second to keep it silent

Use a stilling tube (open at the bottom) to reduce splash/foam errors

Supply the JSN-SR04T with a clean 5 V source

📷 Example

TRIG=GPIO26 ECHO=GPIO16 (with 10k/20k divider)
Mounted vertically at the top of the bilge or tank.

🚀 Future ideas

Output to Signal K or MQTT

On-screen % full gauge

Automatic bilge alerts