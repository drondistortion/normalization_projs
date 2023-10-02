// esp core - 2.0.11
// arduino ide - 1.8.19
#include <iarduino_Modbus.h> // 1.0.1
#include <iarduino_MB_Pump.h> // 1.0.3
#include <iarduino_MB_TDS.h> // 1.1.3
#include <iarduino_I2C_4LED.h> // 1.0.2

constexpr unsigned BUTTON_LEFT = 18;
constexpr unsigned BUTTON_RIGHT = 13;
constexpr unsigned MB_RX = 25;
constexpr unsigned MB_TX = 26;
constexpr unsigned MB_DE = 32;

iarduino_I2C_4LED disp;
ModbusClient modbus(Serial2, MB_DE);
iarduino_MB_Pump pump(modbus);
iarduino_MB_TDS ec_sensor(modbus);

void setup()
{
	initButtons();
	initDisplay();
	initModbus();
	loadSettings();
}

void loop()
{
	handleInput();
	handleRig();
}

void initButtons()
{
	pinMode(BUTTON_LEFT, INPUT);
	pinMode(BUTTON_RIGHT, INPUT);
}

void initDisplay()
{
	disp.begin();
}

void initModbus()
{
	Serial2.begin(9600, SERIAL_8N1, MB_RX, MB_TX); 
	modbus.begin();
	pump.begin();
	ec_sensor.begin();
}

void loadSettings()
{
}

void handleInput()
{
}

void handleRig()
{
	Serial.println(ec_sensor.getID());
	Serial.println(pump.getID());
	disp.print(1111);
	delay(1000);
}
