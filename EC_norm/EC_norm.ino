// esp core - 2.0.11
// arduino ide - 1.8.19
#include <iarduino_Modbus.h> // 1.0.1
#include <iarduino_MB_Pump.h> // 1.0.3
#include <iarduino_MB_TDS.h> // 1.1.3
#include <iarduino_I2C_4LED.h> // 1.0.2
#include <thread>
#include <atomic>
#include <condition_variable>

using namespace std;

constexpr unsigned BUTTON_LEFT = 13;
constexpr unsigned BUTTON_RIGHT = 18;
constexpr unsigned MB_RX = 25;
constexpr unsigned MB_TX = 26;
constexpr unsigned MB_DE = 32;
constexpr unsigned DEBOUNCE_TIME = 20;
constexpr unsigned REPEAT_INTERVAL = 100;
constexpr unsigned REPEAT_AFTER = 1000;

iarduino_I2C_4LED disp;
ModbusClient modbus(Serial2, MB_DE);
iarduino_MB_Pump pump(modbus);
iarduino_MB_TDS ec_sensor(modbus);

atomic<bool> g_left_button_debounced;
atomic<bool> g_right_button_debounced;
atomic<bool> g_still_holding;
atomic<bool> g_button_pressed;
bool g_user_interacted = false;
unsigned g_count = 0;
bool g_right_button, g_left_button;
mutex input_mutex;
condition_variable input_condition;

enum {
	LEFT_BTN,
	RIGHT_BTN,
	N_BUTTONS
};

bool buttons[N_BUTTONS];

typedef unique_lock<mutex> lock_t;

void setup()
{
	initButtons();
	initDisplay();
	//initModbus();
	loadSettings();
	thread(handleRepeat).detach();
}

void loop()
{
	handleInput();
	handleDisplay();
	handleRig();
}

void initButtons()
{
	g_still_holding = true;
	g_button_pressed = false;
	g_left_button_debounced = false;
	g_right_button_debounced = false;
	pinMode(BUTTON_LEFT, INPUT);
	pinMode(BUTTON_RIGHT, INPUT);
	attachInterrupt(BUTTON_LEFT, buttonPressed, RISING);
	attachInterrupt(BUTTON_RIGHT, buttonPressed, RISING);
}

void initDisplay()
{
	disp.begin();
	disp.turn(true);
	disp.print(1111);
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
	if (g_button_pressed) {
		g_button_pressed = false;
		thread(debounce).detach();
	}

	if (g_right_button_debounced) {
		g_right_button = true;
		g_right_button_debounced = false;
		g_user_interacted = true;
		input_condition.notify_one();
	}
	else if (g_left_button_debounced) {
		g_left_button = true;
		g_left_button_debounced = false;
		g_user_interacted = true;
		input_condition.notify_one();
	}

	if (g_still_holding) {
		buttons[LEFT_BTN] = g_left_button;
		buttons[RIGHT_BTN] = g_right_button;
		g_still_holding = false;
	}

	if (buttons[LEFT_BTN]) {
		g_count--;
		buttons[LEFT_BTN] = false;
	}
	else if (buttons[RIGHT_BTN]) {
		g_count++;
		buttons[RIGHT_BTN] = false;
	}
}

void handleRepeat()
{
	for (;;) {
		lock_t lock(input_mutex);
		input_condition.wait(lock);
		delay(REPEAT_AFTER);
		while (g_left_button || g_right_button) {
			g_still_holding = true;
			if (digitalRead(BUTTON_LEFT))
				g_left_button = true;
			else 
				g_left_button = false;

			if (digitalRead(BUTTON_RIGHT))
				g_right_button = true;
			else
				g_right_button = false;

			delay(REPEAT_INTERVAL);
		}
	}
}

void handleDisplay()
{
	if (g_user_interacted)
		disp.print(g_count);
}

void handleRig()
{
	//Serial.println(ec_sensor.getID());
	//Serial.println(pump.getID());
	//delay(1000);
}

void ARDUINO_ISR_ATTR buttonPressed()
{
	g_button_pressed = true;
}

void debounce()
{
	delay(DEBOUNCE_TIME);
	if (digitalRead(BUTTON_LEFT)) {
		g_left_button_debounced = true;
		g_still_holding = true;
	}
	
	if (digitalRead(BUTTON_RIGHT)) {
		g_right_button_debounced = true;
		g_still_holding = true;
	}
}
