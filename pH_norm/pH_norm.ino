// esp core - 2.0.11
// arduino ide - 1.8.19
#include <iarduino_Modbus.h> // 1.0.1
#include <iarduino_MB_Pump.h> // 1.0.3
#include <iarduino_MB_pH.h> // 1.1.3
#include <iarduino_I2C_4LED.h> // 1.0.2
#include <ezButton.h>

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

iarduino_I2C_4LED disp;
ModbusClient modbus(Serial2, MB_DE);
iarduino_MB_Pump pump(modbus);
iarduino_MB_pH ph_sensor(modbus);

ezButton leftButton(BUTTON_LEFT, INPUT);
ezButton rightButton(BUTTON_RIGHT, INPUT);

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
	leftButton.setDebounceTime(DEBOUNCE_TIME);
	rightButton.setDebounceTime(DEBOUNCE_TIME);	
	attachInterrupt(BUTTON_LEFT, buttonPressed, RISING);
	attachInterrupt(BUTTON_RIGHT, buttonPressed, RISING);
}

void initDisplay()
{
	disp.begin();
	disp.turn(true);
	disp.print(8888);
	disp.point(0,1);
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
	if (g_left_released || g_left_holding)
		g_count--;
	if (g_right_released || g_right_holding)
		g_count++;
}

void loadSettings()
{
}

void initFileSystem()
{
}

void handleInput()
{
	leftButton.loop();
	rightButton.loop();

	if (isPressed(leftButton))
		g_left_pressed = true;
	else
		g_left_pressed = false;

	if (isHolding(leftButton) && repeatInterval())
		g_left_holding = true;
	else
		g_left_holding = false;

	if (isPressed(rightButton))
		g_right_pressed = true;
	else
		g_right_pressed = false;

	if (isHolding(rightButton) && repeatInterval())
		g_right_holding = true;
	else
		g_right_holding = false;

	g_left_released = isReleased(leftButton);
	g_right_released = isReleased(rightButton);

	if (isHolding(leftButton) && isHolding(rightButton))
		g_count = 0;
}

unsigned long holdMillis = 0;

bool isPressed(ezButton& button)
{
	if (button.isPressed() && button.getState() == HIGH) {
		holdMillis = millis();
		return true;
	}
	else
		return false;
}

bool isReleased(ezButton& button)
{
	if (button.isReleased() && button.getState() == LOW) {
		holdMillis = millis();
		return true;
	}
	else 
		return false;
}

bool isHolding(ezButton& button)
{
	if (button.getState() == HIGH && millis() - holdMillis > HOLD_TIME) { 
		return true;
	}
	else {
		return false;
	}
}

unsigned long repeatMillis = 0;

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
