// esp core - 2.0.11
// arduino ide - 1.8.19
#include <iarduino_Modbus.h> // 1.0.1
#include <iarduino_MB_Pump.h> // 1.0.3
#include <iarduino_MB_pH.h> // 1.1.3
#include <iarduino_I2C_4LED.h> // 1.0.2
#include <BfButton.h>

#define SETTINGS\
	X(INTERVAL, "I")\
	X(DURATION, "d")\

constexpr unsigned SETTINGS_TIMEOUT = 10000;
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

float g_ph_target = 7.1;

iarduino_I2C_4LED disp;
ModbusClient modbus(Serial2, MB_DE);
iarduino_MB_Pump pump(modbus);
iarduino_MB_pH ph_sensor(modbus);

BfButton leftButton(BfButton::STANDALONE_DIGITAL, BUTTON_LEFT, false, HIGH);
BfButton rightButton(BfButton::STANDALONE_DIGITAL, BUTTON_RIGHT, false, HIGH);

typedef enum {
	NORMAL,
	CHANGE_TARGET,
	SETTINGS_MENU
} state_t;

state_t menu_state = NORMAL;

typedef enum {
#define X(INDEX, STRING) INDEX,
	SETTINGS
#undef X
	N_SETTING
} settings_t;

settings_t current_item = INTERVAL;

uint16_t settings[N_SETTING]{0};

const char* settings_strings[N_SETTING] = {
#define X(INDEX, STRING) STRING,
	SETTINGS
#undef X
};

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
	disp.turn(1);
	disp.frequ(3);
	disp.print(8888);
	disp.point(0,1);
	delay(1500);
}

void initModbus()
{
	Serial2.begin(9600, SERIAL_8N1, MB_RX, MB_TX); 
	modbus.begin();
	pump.begin();
	ph_sensor.begin();
}

unsigned long submenu_millis = 0;
unsigned long enter_submenu_ts = 0;

void handleMenu()
{
	if (menu_state == NORMAL && g_user_interacted)  // change that
		menu_state = CHANGE_TARGET;
	else if (menu_state == CHANGE_TARGET) {
		handleChangeTargetMenu();
		exitOnTimeOut();
	}
	else if (menu_state == SETTINGS_MENU) {
		handleSettingsMenu();
		exitOnTimeOut();
	}
}

void handleChangeTargetMenu()
{
	if (isLeftPressed() || isLeftHolding()) {
		submenu_millis = millis();
		decrementTargetLevel();
	}
	if (isRightPressed() || isRightHolding()) {
		submenu_millis = millis();
		incrementTargetLevel();
	}

	if (areBothHolding()) {
		menu_state = SETTINGS_MENU;
		enter_submenu_ts = millis();
	}
}

void exitOnTimeOut()
{
	if (millis() - submenu_millis > SETTINGS_TIMEOUT)
		menu_state = NORMAL;
}

void handleSettingsMenu()
{
	if (isLeftPressed() || isLeftHolding()) {
		submenu_millis = millis();
		selectItem();
	}
	if (isRightPressed() || isRightHolding()) {
		submenu_millis = millis();
		changeItem();
	}

	/*
	if (areBothHolding()) {
		menu_state = NORMAL;
	}
	*/
}

void selectItem()
{
	if (current_item == INTERVAL)
		current_item = DURATION;
	else if (current_item == DURATION)
		current_item = INTERVAL;
}

void changeItem()
{
	switch (current_item) {
		default: break;
		case DURATION: changeDuration(); break;
		case INTERVAL: changeInterval(); break;
	}
}

void changeDuration()
{
	uint16_t tmp = settings[DURATION];
	tmp++;
	if (tmp > 10)
		tmp = 0;
	settings[DURATION] = tmp;
}

void changeInterval()
{
	uint16_t tmp = settings[INTERVAL];
	tmp += 5;
	if (tmp > 60)
		tmp = 5;
	settings[INTERVAL] = tmp;
}

void loadSettings()
{
	settings[INTERVAL] = 30;
	settings[DURATION] = 1;
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

bool isLeftHolding()
{
	if (digitalRead(BUTTON_LEFT) == LOW) {
		g_user_interacted = false;
		g_left_holding = false;
	}

	if (g_left_holding && repeatInterval())
		return true;
	else
		return false;
}

bool isRightHolding()
{
	if (digitalRead(BUTTON_RIGHT) == LOW) {
		g_user_interacted = false;
		g_right_holding = false;
	}

	if (g_right_holding && repeatInterval())
		return true;
	else
		return false;
}

bool areBothHolding()
{
	if (g_right_holding && g_left_holding)
		return true;
	else
		return false;
}

unsigned long repeat_millis = 0;

bool repeatInterval()
{
	if (millis() - repeat_millis > REPEAT_TIME) {
		repeat_millis = millis();
		return true;
	}
	else
		return false;
}

String gDisplayString;

void handleDisplay()
{
	if (menu_state == NORMAL) {
		disp.blink(5, false);
		gDisplayString = " 8_1";
	}
	else if (menu_state == CHANGE_TARGET) {
		disp.blink(5, true);
		int i = int(g_ph_target);
		int d = 10*(g_ph_target - i);
		gDisplayString = (String)" "+i+"_"+d;
	}
	else if (menu_state == SETTINGS_MENU) {
		disp.blink(5, false);
		gDisplayString = String(settings_strings[current_item]) +" "+ String(settings[current_item]);
	}
	disp.print(gDisplayString);
}

void handleRig()
{
	//Serial.println(ec_sensor.getID());
	//Serial.println(pump.getID());
	//disp.print(1111);
	//delay(1000);
}

void incrementTargetLevel()
{
	g_ph_target += 0.1;
}

void decrementTargetLevel()
{
	g_ph_target -= 0.1;
}

void ARDUINO_ISR_ATTR buttonPressed()
{
	g_user_interacted = true;
}

void leftPressHandler(BfButton* button, BfButton::press_pattern_t pattern)
{
	switch (pattern) {
		default: break;
		case BfButton::SINGLE_PRESS: g_left_pressed = true; break;
		case BfButton::LONG_PRESS: g_left_holding = true; repeat_millis = millis(); break;
	}
}

void rightPressHandler(BfButton* button, BfButton::press_pattern_t pattern)
{
	switch (pattern) {
		default: break;
		case BfButton::SINGLE_PRESS: g_right_pressed = true; break;
		case BfButton::LONG_PRESS: g_right_holding = true; repeat_millis = millis(); break;
	}
}
