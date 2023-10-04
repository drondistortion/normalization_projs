// esp core - 2.0.11
// arduino ide - 1.8.19
#include <iarduino_Modbus.h> // 1.0.1
#include <iarduino_MB_Pump.h> // 1.0.3
#include <iarduino_MB_TDS.h> // 1.1.3
#include <iarduino_I2C_4LED.h> // 1.0.2
#include <BfButton.h>

#define ALL_SEGMENTS 5

#define SETTINGS\
	X(PUMP_A_TIME, "A :")\
	X(PUMP_B_TIME, "B :")\
	X(PUMP_C_TIME, "C :")\
	X(INTERVAL, "nt:")\
	X(PAUSE, "Pd:")\

typedef uint16_t setting_t;

constexpr float MAX_EC_ALLOWED = 5.0;
constexpr float MIN_EC_ALLOWED = 0.1;
constexpr setting_t DEFAULT_PUMP_TIME = 10;
constexpr setting_t DEFAULT_INTERVAL = 30;
constexpr setting_t INTERVAL_DELTA = 5;
constexpr setting_t DEFAULT_PAUSE = 1;
constexpr setting_t MAX_PUMP_TIME = 90;
constexpr setting_t PUMP_TIME_ACCEL = 10;
constexpr setting_t PUMP_TIME_HIGH_DELTA = 10;
constexpr setting_t MAX_ALLOWED_PAUSE = 10;
constexpr setting_t MAX_ALLOWED_INTERVAL = 90;
constexpr setting_t MIN_ALLOWED_ITERVAL = 5;
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

float g_ec_target = 1.2;

iarduino_I2C_4LED disp;
ModbusClient modbus(Serial2, MB_DE);
iarduino_MB_Pump pump(modbus);
iarduino_MB_TDS ec_sensor(modbus);

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
} settings_state_t;

settings_state_t& operator++(settings_state_t& s)
{
	switch (s) {
		default:
#define X(INDEX, STRING) \
		case INDEX: s = static_cast<settings_state_t>(INDEX + 1); s == N_SETTING ? s = PUMP_A_TIME : 0; return s;
			    SETTINGS
#undef X
	}
}

settings_state_t current_item = PUMP_A_TIME;

setting_t settings[N_SETTING]{0};

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
	ec_sensor.begin();
}

unsigned long submenu_millis = 0;
unsigned long enter_submenu_ts = 0;
bool g_already_in_settings = false;

void handleMenu()
{
	if (menu_state == NORMAL && (isLeftPressed() || isRightPressed())) {
		handleNormalMenu();
	}
	else if (menu_state == CHANGE_TARGET) {
		handleChangeTargetMenu();
		exitOnTimeOut();
		g_already_in_settings = false;
	}
	else if (menu_state == SETTINGS_MENU) {
		handleSettingsMenu();
		exitOnTimeOut();
	}
}

void handleNormalMenu()
{
	menu_state = CHANGE_TARGET;
	submenu_millis = millis();
	g_already_in_settings = false;
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


void handleSettingsMenu()
{
	if (!isLeftReleased() && !isRightReleased() && !g_already_in_settings) {
		delay(200);
		return;
	}

	g_already_in_settings = true;

	if (isLeftPressed() || isLeftHolding()) {
		submenu_millis = millis();
		selectItem();
	}
	if (isRightPressed() || isRightHolding()) {
		submenu_millis = millis();
		changeItem();
	}

	if (areBothHolding()) {
		menu_state = NORMAL;
		g_already_in_settings = false;
	}
}

void exitOnTimeOut()
{
	if (millis() - submenu_millis > SETTINGS_TIMEOUT)
		menu_state = NORMAL;
}

void selectItem()
{
	++current_item;
	if (current_item > N_SETTING)
		current_item = PUMP_A_TIME;
}

void changeItem()
{
	switch (current_item) {
		default: break;
		case PUMP_A_TIME: changePumpTime(PUMP_A_TIME); break;
		case PUMP_B_TIME: changePumpTime(PUMP_B_TIME); break;
		case PUMP_C_TIME: changePumpTime(PUMP_C_TIME); break;
		case PAUSE: changePause(); break;
		case INTERVAL: changeInterval(); break;
	}
}

void incrementTargetLevel()
{
	g_ec_target += 0.1;
	if (g_ec_target > MAX_EC_ALLOWED)
		g_ec_target = MAX_EC_ALLOWED;
}

void decrementTargetLevel()
{
	g_ec_target -= 0.1;
	if (g_ec_target < MIN_EC_ALLOWED)
		g_ec_target = MIN_EC_ALLOWED;
}

void changePumpTime(setting_t s)
{
	static setting_t delta = 1;
	setting_t tmp = settings[s];
	if (tmp + delta > PUMP_TIME_ACCEL)
		delta = PUMP_TIME_HIGH_DELTA;
	if (tmp + delta > MAX_PUMP_TIME) {
		tmp = delta = 1;
	}
	tmp += delta;
	settings[s] = tmp;
}

void changePause()
{
	setting_t tmp = settings[PAUSE];
	tmp++;
	if (tmp > MAX_ALLOWED_PAUSE)
		tmp = 0;
	settings[PAUSE] = tmp;
}

void changeInterval()
{
	setting_t tmp = settings[INTERVAL];
	tmp += INTERVAL_DELTA;
	if (tmp > MAX_ALLOWED_INTERVAL)
		tmp = MIN_ALLOWED_ITERVAL;
	settings[INTERVAL] = tmp;
}

void loadSettings()
{
	settings[PUMP_A_TIME] = DEFAULT_PUMP_TIME;
	settings[PUMP_B_TIME] = DEFAULT_PUMP_TIME;
	settings[PUMP_C_TIME] = DEFAULT_PUMP_TIME;
	settings[INTERVAL] = DEFAULT_INTERVAL;
	settings[PAUSE] = DEFAULT_PAUSE;
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

bool isLeftReleased()
{
	if (digitalRead(BUTTON_LEFT) == LOW)
		return true;
	else
		return false;
}

bool isRightReleased()
{
	if (digitalRead(BUTTON_RIGHT) == LOW)
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
		disp.blink(ALL_SEGMENTS, false);
		gDisplayString = " 1_2";
	}
	else if (menu_state == CHANGE_TARGET) {
		disp.blink(ALL_SEGMENTS, true);
		int i = int(g_ec_target);
		int d = 10*(g_ec_target - i);
		gDisplayString = (String)" "+i+"_"+d;
	}
	else if (menu_state == SETTINGS_MENU) {
		disp.blink(ALL_SEGMENTS, false);
		setting_t curr_setting = settings[current_item];
		String disp_setting = curr_setting >= 10 ? String(curr_setting) : " " + String(curr_setting);
		gDisplayString = String(settings_strings[current_item]) + disp_setting;
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
