// esp core - 2.0.11
// arduino ide - 1.8.19
#include <iarduino_Modbus.h> // 1.0.1
#include <iarduino_MB_Pump.h> // 1.0.3
#include <iarduino_MB_pH.h> // 1.1.3
#include <iarduino_I2C_4LED.h> // 1.0.2
#include <BfButton.h>

constexpr unsigned BUTTON_LEFT = 13;
constexpr unsigned BUTTON_RIGHT = 18;
constexpr unsigned MB_RX = 25;
constexpr unsigned MB_TX = 26;
constexpr unsigned MB_DE = 32;
constexpr unsigned DEBOUNCE_TIME = 50;
constexpr unsigned HOLD_TIME = 1000;
constexpr unsigned REPEAT_TIME = 200;

volatile bool g_user_interacted = false;
bool g_left_pressed = false;
bool g_right_pressed = false;
bool g_left_released = false;
bool g_right_released = false;
bool g_left_holding = false;
bool g_right_holding = false;
unsigned g_count = 0;

float g_ph = 7.1;

iarduino_I2C_4LED disp;
ModbusClient modbus(Serial2, MB_DE);
iarduino_MB_Pump pump(modbus);
iarduino_MB_pH ph_sensor(modbus);

BfButton leftButton(BfButton::STANDALONE_DIGITAL, BUTTON_LEFT, false, HIGH);
BfButton rightButton(BfButton::STANDALONE_DIGITAL, BUTTON_RIGHT, false, HIGH);

void setup()
{
	initButtons();
	initDisplay();
	//initModbus();
	initFileSystem();
	loadSettings();
}

void loop()
{
	handleInput();
	handleDisplay();
	handleMenu();
	handleRig();
}

void initButtons()
{
	leftButton.onPress(leftPressHandler).onPressFor(leftPressHandler, HOLD_TIME);
	rightButton.onPress(rightPressHandler).onPressFor(rightPressHandler, HOLD_TIME);
	attachInterrupt(BUTTON_LEFT, buttonPressed, RISING);
	attachInterrupt(BUTTON_RIGHT, buttonPressed, RISING);
}

void initDisplay()
{
	disp.begin();
	disp.turn(true);
	disp.print(8888);
	disp.point(0,1);
	delay(500);
}

void initModbus()
{
	Serial2.begin(9600, SERIAL_8N1, MB_RX, MB_TX); 
	modbus.begin();
	pump.begin();
	ph_sensor.begin();
}

void handleMenu()
{
	if (isLeftPressed() || isLeftHolding())
		g_count--;
	if (isRightPressed() || isRightHolding())
		g_count++;
	if (areBothHolding())
		g_count = 0;
}

void loadSettings()
{
}

void initFileSystem()
{
}

void handleInput()
{
	leftButton.read();
	rightButton.read();
}

bool isLeftPressed()
{
	bool tmp = g_left_pressed;
	g_left_pressed = false;
	return tmp;
}

bool isRightPressed()
{
	bool tmp = g_right_pressed;
	g_right_pressed = false;
	return tmp;
}

bool right_holding = false;
bool left_holding = false;

bool isLeftHolding()
{
	if (digitalRead(BUTTON_LEFT) == LOW)
		g_left_holding = false;
	return g_left_holding;
}

bool isRightHolding()
{
	if (digitalRead(BUTTON_RIGHT) == LOW)
		g_right_holding = false;
	return g_right_holding;
}

bool areBothHolding()
{
	if (g_right_holding && g_left_holding)
		return true;
	else
		return false;
}

bool repeatInterval()
{
	if (millis() - repeatMillis > REPEAT_TIME) {
		repeatMillis = millis();
		return true;
	}
	else
		return false;
}

void handleDisplay()
{
	disp.print(g_count);
}

void handleRig()
{
	//Serial.println(ec_sensor.getID());
	//Serial.println(pump.getID());
	//disp.print(1111);
	//delay(1000);
}

void ARDUINO_ISR_ATTR buttonPressed()
{
	g_user_interacted = true;
}

void leftPressHandler(BfButton* button, BfButton::press_pattern_t pattern)
{
	switch (pattern) {
		case BfButton::SINGLE_PRESS: g_left_pressed = true;
		case BfButton::LONG_PRESS: left_holding = true;
	}
}

void rightPressHandler(BfButton* button, BfButton::press_pattern_t pattern)
{
	switch (pattern) {
		case BfButton::SINGLE_PRESS: g_right_pressed = true;
		case BfButton::LONG_PRESS: right_holding = true;
	}
}
