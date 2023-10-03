#ifndef __RIG_H__
#define __RIG_H__
#include "IO.h"
#include "DateTime.h" // getDays, getHour
#include "Log.h"
#define MAX_DAYS 250

class Rig {
	public:
		Rig(): _paused(true){};

		float lastPH()
		{
			return _lastPh;
		}

		float lastEC()
		{
			return _lastEc;
		}

		void start()
		{
			io.resetStates();
			_paused = false;
		}

		void halt()
		{
			_paused = true;
			_led = false;
			_passvent = false;
			_vent = false;
			_steam = false;
			_co2 = false;
			_pumpison = false;
			_circ = false;
			_mainpump = false;
			_aeropump = false;
			io.haltAll();
			log_v("Rig has been halted");
		}

		void init()
		{
			rig_settings_t ec_set;
			int today = datetime.getDays();
			if (today < 0) {
				log_d("Failed to get days: %d", today);
				return;
			}
			if (today > MAX_DAYS) {
				log_d("Failed to get days: %d", today);
				return;
			}

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
				log_d("EC: first stage");
				ec_set = EC_CYCL1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL2;
				log_d("EC: second stage");
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL3;
				log_d("EC: third stage");
			}
			_set_ec = g_data.getFloat(ec_set);

			rig_settings_t ph_set;

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
				log_d("pH, stage 1");
				ph_set = ACID_1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {

				log_d("pH, stage 2");
				ph_set = ACID_2;
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {

				log_d("pH, stage 3");
				ph_set = ACID_3;
			}
			_set_ph = g_data.getFloat(ph_set);
			_init_done = true;
		}

		bool initDone()
		{
			return _init_done;
		}

		void systemPause()
		{
			_no_user_pause = true;
		}

		bool getSystemPause()
		{
			bool tmp = _no_user_pause;
			_no_user_pause = false;
			return tmp;
		}

		void update()
		{
			if (gClearToMeasure)
				_updateSolutions();
			if (_paused)
				return;

			_updateLight();
			_updateVent();
			_updateSteam();
			_updateCO2();
			_updateAero();
			//_updatePassVent(); TODO: no hardware yet
			_updateRigType();
		}

		bool onPause()
		{
			return _paused;
		}

		bool getLed()
		{
			return _led;
		}

		bool getVent()
		{
			return _vent;
		}

		bool getPassVent()
		{
			return _passvent;
		}

		bool getPump()
		{
			return _mainpump;
		}

		bool getAero()
		{
			return _aeropump;
		}

		bool getCO2()
		{
			return _co2;
		}

		bool getSteam()
		{
			return _steam;
		}

		void setMeasureWindow(bool window=true)
		{
			_measureWindow = window;
		}

		void trigMeasureTime()
		{
			_measuretime = true;
		}

		bool measureTime()
		{
			return _measuretime;
		}

		bool measureDone()
		{
			bool measuredone = _measuredone;
			_measuredone = false;
			return measuredone;
		}

		bool normalizationDone()
		{
			bool nd = _normalization_done;
			_normalization_done = false;
			return nd;
		}

		void resetPump()
		{
			_haltpump = false;
		}

		float getSetEC()
		{
			return _set_ec;
		}

		float getSetPH()
		{
			return _set_ph;
		}

	private:
		// rig flags
		bool _measuredone = false;
		bool _measuretime = false;
		bool _measuretimetrig = false;
		bool _paused;
		bool _window_opened = false;
		bool _window_energized = false;
		bool _led = false;
		bool _vent = false;
		bool _circ = false;
		bool _co2 = false;
		bool _steam = false;
		bool _aeropump = false;
		bool _passvent = false;
		bool _measureWindow = false;

		// getDays error
		bool _error = false;

		// rig timers
		unsigned long _window_timer = 0;
		static constexpr unsigned long WINDOW_INT = 60000;

		// missing hardware flags, so doesn't spam log
		bool _noLight = false;
		bool _noVent = false;
		bool _noCO2 = false;
		bool _noSteam = false;
		bool _noPassVent = false;
		bool _noMainPump = false;
		bool _noAero = false;
		bool _noH2Opump = false;
		bool _noPhPump = false;
		bool _noTdsPump = false;
		bool _badEc = false;
		bool _badPh = false;
		float _set_ec;
		float _set_ph;
		// last normalization time
		time_t _norm_time;
		bool _init_done = false;

	public:
		time_t getNormTime()
		{
			return _norm_time;
		}

		void resetNoLight()
		{
			_noLight = false;
		}

		void resetNoVent()
		{
			_noVent = false;
		}

		void resetNoSteam()
		{
			_noSteam = false;
		}

		void resetNoCO2()
		{
			_noCO2 = false;
		}

		void resetNoPassVent()
		{
			_noPassVent = false;
		}

		void resetNoMainPump()
		{
			_noMainPump = false;
		}

		void resetNoAero()
		{
			_noAero = false;
		}

		void resetNoH2Opump()
		{
			_noH2Opump = false;
		}

		void resetNoPhPump()
		{
			_noPhPump = false;
		}

		void resetNoTdsPump()
		{
			_noTdsPump = false;
		}

		void resetBadPh()
		{
			_badPh = false;
		}

		void resetBadEc()
		{
			_badEc = false;
		}

		void resetCyclVent()
		{
			_ventMillis = 0;
		}

		void resetCyclCirc()
		{
			_circMillis = 0;
		}

		void resetCyclAero()
		{
			_aeroMillis = 0;
		}

		void resetCyclCO2()
		{
			_co2Millis = 0;
		}

		void resetCyclSteam()
		{
			_steamMillis = 0;
		}

	private:
		void _updateSteam()
		{
			_steam = false;
			if (!g_data.getInt(STEAM_ON)) {
				io.driveOut(OUT_STEAM, false);
				return;
			}
			else if (io.noSteam() && !_noSteam) {
				_noSteam = true;
				return;
			}
			else if (!io.noSteam()) {
				resetNoSteam();
			}

			if (g_data.getInt(STEAM_CONST)) {
				_steamConst();
			}
			else if (g_data.getInt(STEAM_CYCL)) {
				_steamCycl();
			}

			io.driveOut(OUT_STEAM, _steam);
		}

		void _steamConst()
		{
			// checkboxes
			bool a = g_data.getInt(STEAM_TIME_LIM);
			bool b = g_data.getInt(STEAM_HUM_LIM);

			int tmp = datetime.getHour();
			if (tmp < 0)
				return;
			// input boxes
			bool x = (g_data.getInt(STEAM_TIME_FROM) <= tmp) && (tmp < g_data.getInt(STEAM_TIME_TO));
			bool y = int(io.getHum()) < g_data.getInt(STEAM_HUM_THRES);

			if (!a)
				x = true;
			if (!b || io.noSteam())
				y = false;

			bool Q = (x || !a) && (y || !b);
			if (g_data.getInt(STEAM_OR))
				Q = x || y;

			if (Q)
				_steam = true;
			else
				_steam = false;
		}

		unsigned long _steamMillis = 0;
		bool _steamIsOn = true;

		void _steamCycl()
		{
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;

			// imput boxes
			int time_from = g_data.getInt(STEAM_CYCL_TIME_FROM);
			int time_to = g_data.getInt(STEAM_CYCL_TIME_TO);
			int on_duration = g_data.getInt(STEAM_DUR);
			int off_duration = g_data.getInt(STEAM_PAUS_DUR);

			bool time_to_vent = (time_from <= tmp) && (tmp < time_to);

			// milliseconds to run the co2
			unsigned long fullcycle_ms = (on_duration+off_duration)*60000;
			unsigned long on_dur_ms = on_duration*60000;

			// reset cycle
			if (millis() - _steamMillis > fullcycle_ms) {
				_steamIsOn = true;
				_steamMillis = millis();
			}

			// switch to pause
			if (_steamIsOn && millis() - _steamMillis > on_dur_ms) {
				_steamIsOn = false;
			}

			_steam = time_to_vent && _steamIsOn;
		}


		void _updateCO2()
		{
			_co2 = false;
			if (!g_data.getInt(CO2_ON)) {
				io.driveOut(OUT_CO2, false);
				return;
			}
			else if (io.noCO2sc() && !_noCO2) {
				_noCO2 = true;
				return;
			}
			else if (!io.noCO2sc()) {
				resetNoCO2();
			}

			if (g_data.getInt(CO2_CONST)) {
				_co2Const();
			}
			else if (g_data.getInt(CO2_CYCL)) {
				_co2Cycl();
			}
		}

		void _co2Const()
		{
			// checkboxes
			bool a = g_data.getInt(CO2_TIME_LIM);
			bool b = g_data.getInt(CO2_LIM);

			int tmp = datetime.getHour();
			if (tmp < 0)
				return;
			// input boxes
			bool x = (g_data.getInt(CO2_TIME_FROM) <= tmp) && (tmp < g_data.getInt(CO2_TIME_TO));
			bool y = int(io.getCO2()) < g_data.getInt(CO2_THRES);

			if (!a)
				x = true;
			if (!b || !io.hasCO2())
				y = false;

			bool Q = x && y;

			if (Q)
				_co2 = true;
			else
				_co2 = false;

			io.driveOut(OUT_CO2, _co2);
		}

		unsigned long _co2Millis = 0;
		bool _co2IsOn = true;

		void _co2Cycl()
		{
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;

			// imput boxes
			int time_from = g_data.getInt(CO2_CYCL_TIME_FROM);
			int time_to = g_data.getInt(CO2_CYCL_TIME_TO);
			int on_duration = g_data.getInt(CO2_DUR);
			int off_duration = g_data.getInt(CO2_PAUS_DUR);

			bool time_to_vent = (time_from <= tmp) && (tmp < time_to);

			// milliseconds to run the co2
			unsigned long fullcycle_ms = (on_duration+off_duration)*60000;
			unsigned long on_dur_ms = on_duration*60000;

			// reset cycle
			if (millis() - _co2Millis > fullcycle_ms) {
				_co2IsOn = true;
				_co2Millis = millis();
			}

			// switch to pause
			if (_co2IsOn && millis() - _co2Millis > on_dur_ms) {
				_co2IsOn = false;
			}

			_co2 = time_to_vent && _co2IsOn;

			io.driveOut(OUT_CO2, _co2);
		}

		void _updateLight()
		{
			_led = false;

			if (!g_data.getInt(LIGHT_ON)) {
				io.driveOut(PWR_PG_LIGHT, false);
				return;
			}

			// check day
			int day = datetime.getDays();
			if (day > 0)
				_error = false;
			if (_error)
				return;
			if (day < 0) {
				_error = true;
				log_v("Something wrong with planting days: %d", day);
				return;
			}

			if (day < g_data.getInt(LIGHT_DAY)) {
				io.driveOut(PWR_PG_LIGHT, false);
				return;
			}

			// check hour
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;
			if (g_data.getInt(LIGHT_FROM) <= tmp && tmp < g_data.getInt(LIGHT_TO)) {
				io.driveOut(PWR_PG_LIGHT, true);
				_led = true;
			}
			else {
				io.driveOut(PWR_PG_LIGHT, false);
			}
		}


		void _ventConst()
		{
			// checkboxes
			bool a = g_data.getInt(VENT_TIME_LIM);
			bool b = g_data.getInt(VENT_TEMP_LIM);
			bool c = g_data.getInt(VENT_HUM_LIM);

			int tmp = datetime.getHour();
			if (tmp < 0)
				return;
			// input boxes
			bool x = (g_data.getInt(VENT_TIME_FROM) <= tmp) && (tmp < g_data.getInt(VENT_TIME_TO));
			bool y = int(io.getTem()) > g_data.getInt(VENT_TEMP_THRES);
			bool z = int(io.getHum()) > g_data.getInt(VENT_HUM_THRES);

			if (!a)
				x = false;
			if (!b || io.noSht())
				y = false;
			if (!c || io.noSht())
				z = false;

			bool Q = (x || !a) && (y || !b) && (z || !c);
			if (g_data.getInt(VENT_OR))
				Q = x || y || z;

			if (!a && !b && !c)
				Q = true;

			if (Q)
				_vent = true;
			else
				_vent = false;

		}

		unsigned long _ventMillis = 0;
		bool _ventIsOn = true;

		// cycling ventilation
		void _ventCycl()
		{
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;

			// imput boxes
			int time_from = g_data.getInt(VENT_CYCL_TIME_FROM);
			int time_to = g_data.getInt(VENT_CYCL_TIME_TO);
			int on_duration = g_data.getInt(VENT_DUR);
			int off_duration = g_data.getInt(VENT_PAUS_DUR);

			bool time_to_vent = (time_from <= tmp) && (tmp < time_to);

			// milliseconds to run the fan
			unsigned long fullcycle_ms = (on_duration+off_duration)*60000;
			unsigned long on_dur_ms = on_duration*60000;

			// reset cycle
			if (millis() - _ventMillis > fullcycle_ms) {
				_ventIsOn = true;
				_ventMillis = millis();
			}

			// switch to pause
			if (_ventIsOn && millis() - _ventMillis > on_dur_ms) {
				_ventIsOn = false;
			}

			_vent = time_to_vent && _ventIsOn;
		}

		// main pump
		void _updateMainPump()
		{
			if (!g_data.getInt(CIRC_ON)) {
				io.driveOut(mainPump, false);
				_mainpump = false;
				return;
			}
			int circWindowCheckBoxOn = g_data.getInt(PUMP_OFF);
			if (circWindowCheckBoxOn && !_rigInitDone) {
				setMainPumpWindow();
			}
			if (_mainpumpwindow && circWindowCheckBoxOn) {
				io.driveOut(mainPump, false);
				_mainpump = false;
				return;
			}

			if (g_data.getInt(CIRC_CONST)) {
				_mainPumpConst();
			}
			else {
				_mainPumpCycl();
			}

			/*
				bool state = !io.getMainLow();
				_mainpump = state;
				io.driveOut(mainPump, state);
			*/
		}

		void _mainPumpConst()
		{
			int tmp = datetime.getHour();
			if (tmp < 0) {
				log_v("Got negative hour: %d", tmp);
				return;
			}

			bool a = g_data.getInt(CIRC_TIME_LIM);
			bool x = (g_data.getInt(CIRC_TIME_FROM) <= tmp) && (tmp < g_data.getInt(CIRC_TIME_TO));
			if (!a)
				x = true;

			if (x) {
				bool state = !io.getMainLow();
				_mainpump = state;
				io.driveOut(mainPump, state);
			}
		}


		unsigned long _circMillis = 0;
		bool _circIsOn = true;

		void _mainPumpCycl()
		{
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;

			// imput boxes
			int time_from = g_data.getInt(CIRC_CYCL_TIME_FROM);
			int time_to = g_data.getInt(CIRC_CYCL_TIME_TO);
			int on_duration = g_data.getInt(CIRC_DUR);
			int off_duration = g_data.getInt(CIRC_PAUS_DUR);

			bool time_to_circ = (time_from <= tmp) && (tmp < time_to);

			// milliseconds to run the fan
			unsigned long fullcycle_ms = (on_duration+off_duration)*60000;
			unsigned long on_dur_ms = on_duration*60000;

			// reset cycle
			if (millis() - _circMillis > fullcycle_ms) {
				_circIsOn = true;
				_circMillis = millis();
			}

			// switch to pause
			if (_circIsOn && millis() - _circMillis > on_dur_ms) {
				_circIsOn = false;
			}

			bool state = !io.getMainLow();
			_circ = time_to_circ && _circIsOn && state;
			_mainpump = _circ;

			io.driveOut(mainPump, _circ);
		}

		void _updateVent()
		{
			_vent = false;
			if (!g_data.getInt(VENT_ON)) {
				io.driveOut(PWR_PG_FAN, false);
				return;
			}
			else if (io.noVent() && !_noVent) {
				_noVent = true;
				return;
			}
			else if (!io.noVent()) {
				resetNoVent();
			}

			if (g_data.getInt(VENT_CONST)) {
				_ventConst();
			}
			else {
				_ventCycl();
			}

			io.driveOut(PWR_PG_FAN, _vent);
		}

		void _updateAero()
		{
			aeroPump = PWR_PG_PORT_H;
			_aeropump = false;

			if (!g_data.getInt(AERO_ON)) {
				io.driveOut(aeroPump, false);
				return;
			}
			else if (io.noAero() && !_noAero) {
				_noAero = true;
				return;
			}
			else if (!io.noVent()) {
				resetNoVent();
			}

			bool aeroWindowCheckBoxOn = g_data.getInt(AERO_PUMP_OFF);
			if (_aeropumpwindow && aeroWindowCheckBoxOn) {
				_aeropump = false;
				io.driveOut(aeroPump, false);
				return;
			}

			if (g_data.getInt(AERO_CONST)) {
				_aeroConst();
			}
			else {
				_aeroCycl();
			}

			io.driveOut(aeroPump, _aeropump);
		}

		unsigned long _aeroMillis = 0;
		bool _aeroIsOn = true;

		void _aeroCycl()
		{
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;

			// imput boxes
			int time_from = g_data.getInt(AERO_CYCL_TIME_FROM);
			int time_to = g_data.getInt(AERO_CYCL_TIME_TO);
			int on_duration = g_data.getInt(AERO_DUR);
			int off_duration = g_data.getInt(AERO_PAUS_DUR);

			bool time_to_aero = (time_from <= tmp) && (tmp < time_to);

			// milliseconds to run aero pump
			unsigned long fullcycle_ms = (on_duration+off_duration)*60000;
			unsigned long on_dur_ms = on_duration*60000;

			// reset cycle
			if (millis() - _aeroMillis > fullcycle_ms) {
				_aeroIsOn = true;
				_aeroMillis = millis();
			}

			// switch to pause
			if (_aeroIsOn && millis() - _aeroMillis > on_dur_ms) {
				_aeroIsOn = false;
			}

			_aeropump = time_to_aero && _aeroIsOn;
		}

		void _aeroConst()
		{
			int tmp = datetime.getHour();
			if (tmp < 0) {
				log_v("Got negative hour: %d", tmp);
				return;
			}

			bool timelimit = g_data.getInt(AERO_TIME_LIM);
			_aeropump = (g_data.getInt(AERO_TIME_FROM) <= tmp) && (tmp < g_data.getInt(AERO_TIME_TO));
			if (!timelimit)
				_aeropump = true;
		}

		void _openWindow()
		{
			io.driveOut(PWR_PG_UP, true);
			_window_energized = true;
			_window_timer = millis();
		}

		void _closeWindow()
		{
			io.driveOut(PWR_PG_DOWN, true);
			_window_energized = true;
			_window_timer = millis();
		}

		void _updatePassVent()
		{
			if (_window_energized || _window_opened) {
				_passvent = true;
			}
			else {
				_passvent = false;
			}

			if (!g_data.getInt(PASSVENT)) {
				return;
			}
			else if (io.noPassVent() && !_noPassVent) {
				_noPassVent = true;
				ezplant_log::log(nullptr, ER_NOPASSVENT);
				log_e("No passive ventilation hardware");
				return;
			}
			else if (!io.noPassVent()) {
				resetNoPassVent();
			}

			if (!_window_energized) {
				io.driveOut(PWR_PG_UP, false);
			}

			if (_window_energized) {
				if (millis() - _window_timer > WINDOW_INT) {
					_window_energized = false;
					_window_opened = !_window_opened;
				}
				return;
			}

			bool a = g_data.getInt(PASSVENT_TIME_LIM);
			bool b = g_data.getInt(PASSVENT_TEMP_LIM);
			bool c = g_data.getInt(PASSVENT_HUM_LIM);
			int tmp = datetime.getHour();
			if (tmp < 0)
				return;
			bool x = (g_data.getInt(PASSVENT_TIME_FROM) <= tmp)
				&& (tmp < g_data.getInt(PASSVENT_TIME_TO));
			bool y = int(io.getTem()) > g_data.getInt(PASSVENT_TEMP_THRES);
			bool z = int(io.getHum()) > g_data.getInt(PASSVENT_HUM_THRES);

			if (!a)
				x = false;
			if (!b || io.noSht())
				y = false;
			if (!c || io.noSht())
				z = false;

			bool Q = x || y || z;

			if (Q && !_window_opened) {
				_openWindow();
			}
			else if (!Q && _window_opened)
				_closeWindow();
		}

		bool _commenceABC = false;
		bool _prepareABC = false;
		bool _commencePH = false;
		bool _preparePH = false;
		float _pumpAseconds = 0;
		float _pumpBseconds = 0;
		float _pumpCseconds = 0;
		int _pumpPHseconds = 0;

		static constexpr unsigned long _begin_interval = 10000;
		unsigned long _begin_mils = 0;
		bool _metersBusy = false;
		float _lastPh;
		float _lastEc;

		bool _localMeas = false;
		bool _localMeas2 = false;
		unsigned long _meas_mils = 0;
		static constexpr unsigned long _meas_interval = 15000;

		// read from settings EC_ON, ACID_ON
		bool _acid = false;
		bool _ec = false;

		void _resetMeasure()
		{
			_measuretime = false;
			_localMeas = false;
			_metersBusy = false;
			_localMeas2 = false;
			_measuredone = true;
			_mainpumpwindow = false;
			_aeropumpwindow = false;
			// single shot flag after reboot
			_rigInitDone = true;
		}

		void _measure()
		{
			_acid = g_data.getInt(ACID_ON);
			_ec = g_data.getInt(EC_ON);

			if (!_acid && !_ec) {
				log_d("Normalisations haven't been set");
				_resetMeasure();
				return;
			}

			if (!_metersBusy) {
#ifdef RIG_DEBUG
				log_d("Planting day: %d", datetime.getDays());
#endif
				log_v("pH and TDS sensors init");
				io.initTdsPhMeters();
				_begin_mils = millis();
				_metersBusy = true;

				// reset all if both sensors absent;
				if (io.noTds() && io.noPh()) {
					log_d("No sensors");
					_resetMeasure();
					return;
				}

				if (!_acid) {
					log_d("pH normalization not set by user");
				}
				if (!_ec) {
					log_d("EC normalization not set by user");
				}
			}

			if (millis() - _begin_mils > _begin_interval && !_localMeas) {
				log_i("Getting pH and EC measurement");

				for (int i = 0; i < 3; i++) {
					_lastPh = io.getPH();
					_lastEc = io.getEC();
				}

				_meas_mils = millis();
				_localMeas = true;
				_localMeas2 = true;
			}

			if (millis() - _meas_mils > _meas_interval && _localMeas2) {
				_lastPh = io.getPH();
				_lastEc = io.getEC();

				log_d("Measurements taken");
				_resetMeasure();

				if (!io.noTds() && _ec) {
					_prepareABC = true;

				}
				else if (!io.noPh() && _acid) {
					_preparePH = true;
				}
			}
		}

		// between a, b, c and ph pumping
		int _sol_interval = 0;
		bool _intervalFlag = false;

		void _setSolInterval(int seconds)
		{
			_sol_interval = seconds * 1000;
		}

		void _updateSolutions()
		{
			int hour = datetime.getHour();
			if (hour < 0)
				return;
			_setSolInterval(g_data.getInt(SOLUTIONS_INT));


			// allowed time interval
			if (g_data.getInt(NORM_AL_TM_LO) <= hour && hour < g_data.getInt(NORM_AL_TM_HI)) {

				// measure pH and EC
				if (_measuretime) {
					_measure();
					return;
				}

				// normalize
				_updateABC();
				_squirtABC();
				_updatePH();
				_squirtPH();
			}
		}


		void _updateABC()
		{
			if (!_prepareABC)
				return;

			if (_commenceABC)
				return;

			// user setting
			if (!_ec) {
				return;
			}
			// hardware presence
			else if (io.noTdsPump() && !_noTdsPump) {
				_noTdsPump = true;
				log_e("No EC normalization hardware");
			}
			else if (!io.noTdsPump()) {
				resetNoTdsPump();
			}

			if (_noTdsPump)
				return;

			// allowed range interval
			if ((g_data.getFloat(ALLOWED_EC_MAX) < _lastEc
					|| _lastEc < g_data.getFloat(ALLOWED_EC_MIN))
					&& !_badEc) {
				_badEc = true;
				ezplant_log::log(
						[this](void*)
						{
							this->resetBadEc();
						}
						, ER_BADTDSSOLUTION);
				log_e("EC value out of range");
				_prepareABC = false;
				_preparePH = true;
				return;
			}

			/* // TODO: add this after adding resseter to logs
			   and reset _badEc if measure is correct
			if (_badEc)
				return;
				*/

			// return if one of the tanks is empty
			bool* dig = io.getDigitalValues();
			bool a = dig[DIG_KEY10];
			bool b = dig[DIG_KEY9];
			bool c = dig[DIG_KEY8];

			if (a || b || c) {
				log_i("A: %d", a);
				log_i("B: %d", b);
				log_i("C: %d", c);
				_prepareABC = false;
				_preparePH = true;
				log_e("One of the tanks is empty. Normalisation forbidden");
				return;
			}

			rig_settings_t ec_set, ec_a, ec_b, ec_c;
			int pumptime = g_data.getInt(EC_PUMPS);
			int today = datetime.getDays();
			if (today < 0) {
				log_d("Failed to get days: %d", today);
				return;
			}

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
				log_d("EC: first stage");
				ec_set = EC_CYCL1;
				ec_a = EC_A1;
				ec_b = EC_B1;
				ec_c = EC_C1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL2;
				ec_a = EC_A2;
				ec_b = EC_B2;
				ec_c = EC_C2;
				log_d("EC: second stage");
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL3;
				ec_a = EC_A3;
				ec_b = EC_B3;
				ec_c = EC_C3;
				log_d("EC: third stage");
			}

			_set_ec = g_data.getFloat(ec_set);
			float set_ec = _set_ec;
			float hyst_ec = g_data.getFloat(EC_HYST);
			float target_ec = set_ec - hyst_ec;

#ifdef RIG_DEBUG
			log_d("Target EC: %.2f", set_ec);
			log_d("Hysteresis EC: %.2f", hyst_ec);
			log_d("Delta: %.2f", set_ec - hyst_ec);
			log_d("Current: %.2f", _lastEc);
#endif

			// if EC more than what's set go to PH
			if (_lastEc >= target_ec) {
				log_d("Current EC is OK");
				_prepareABC = false;
				_preparePH = true;
				return;
			}

			_commenceABC = true;

			//squirt amount set in settings
			_pumpAseconds = g_data.getFloat(ec_a) * pumptime;
			_pumpBseconds = g_data.getFloat(ec_b) * pumptime;
			_pumpCseconds = g_data.getFloat(ec_c) * pumptime;
			_pump_mils = millis();
			_prepareABC = false;
		}

		unsigned long _pump_mils = 0;
		enum {
			A_ON,
			WAIT_A,
			B_ON,
			WAIT_B,
			C_ON,
			WAIT_C
		} pumps = A_ON;

	private:
		bool _a = false;
		bool _b = false;
		bool _c = false;
		bool _pHu = false;
		bool _pHd = false;
	public:

		bool getA()
		{
			return _a;
		}

		bool getB()
		{
			return _b;
		}

		bool getC()
		{
			return _c;
		}

		bool getPU()
		{
			return _pHu;
		}

		bool getPD()
		{
			return _pHd;
		}

		bool getTapPump()
		{
			return _pumpison;
		}

		void _squirtABC()
		{
			if (_paused)
				return;

			if (!_commenceABC)
				return;

			if (pumps == A_ON && !_a) {
				_a = true;
				log_d("Normalizing EC");
				log_v("Tank A");
				// drive pumpA with _pumpAseconds
				io.driveTimed(MB_A, _pumpAseconds);
				_pump_mils = millis();
			}

			if (pumps == A_ON && millis() - _pump_mils > _pumpAseconds*1000) {
				log_d("Switch to wait for B");
				pumps = WAIT_A;
				_a = false;
				_pump_mils = millis();
			}

			if (pumps == WAIT_A && millis() - _pump_mils > _sol_interval) {
				log_d("Switch to B");
				pumps = B_ON;
				_pump_mils = millis();
			}

			if (pumps == B_ON && !_b) {
				_b = true;
				log_d("Tank B");
				// drive pumpB with _pumpBseconds
				io.driveTimed(MB_B, _pumpBseconds);
				//io.driveOut(pumpB, true);
				_pump_mils = millis();
			}

			if (pumps == B_ON && millis() - _pump_mils > _pumpBseconds*1000) {
				log_d("Switch to wait for C");
				pumps = WAIT_B;
				_b = false;
				_pump_mils = millis();
			}

			if (pumps == WAIT_B && millis() - _pump_mils > _sol_interval) {
				log_d("Switch to C");
				pumps = C_ON;
				_pump_mils = millis();
			}

			if (pumps == C_ON && !_c) {
				_c = true;
				log_d("Tank C");
				// drive pumpC with _pumpCseconds
				io.driveTimed(MB_C, _pumpCseconds);
				_pump_mils = millis();
			}

			if (pumps == C_ON && millis() - _pump_mils > _pumpCseconds*1000) {
				log_d("Switch to wait for pH");
				pumps = WAIT_C;
				_c = false;
				_pump_mils = millis();
			}

			if (pumps == WAIT_C && millis() - _pump_mils > _sol_interval) {
				log_d("Switch to PH");
				//_a = _b = _c = false;
				pumps = A_ON;
				_pump_mils = millis();
				_commenceABC = false;
				_preparePH = true;
				_norm_time = datetime.now();
				_normalization_done = true;
			}
		}

		enum {
			NO_PH,
			PH_UP,
			PH_DW,
		} _ph_state;

		void _updatePH()
		{

			if (!_preparePH || _commencePH)
				return;

			// user setting
			if (!_acid) {
				return;
			}
			// hardware presence
			else if (io.noPhPump() && !_noPhPump) {
				_noPhPump = true;
				ezplant_log::log(nullptr, ER_NOPHPUMP);
				log_e("No pH normalization hardware");
			}
			else if (!io.noPhPump()) {
				resetNoPhPump();
			}

			if (_noPhPump)
				return;

			// allowed range interval
			if ((g_data.getFloat(ALLOWED_PH_MAX) < _lastPh
					|| _lastPh < g_data.getFloat(ALLOWED_PH_MIN))
					&& !_badPh) {
				_badPh = true;
				ezplant_log::log(
						[this](void*)
						{
							this->resetBadPh();
						}
						, ER_BADPHSOLUTION);
				log_e("pH value out of range");
				_preparePH = false;
				return;
			}


			rig_settings_t ph_set;
			int today = datetime.getDays();
			if (today < 0) {
				log_d("Failed to get days: %d", today);
				return;
			}

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
				log_d("pH, stage 1");
				ph_set = ACID_1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {

				log_d("pH, stage 2");
				ph_set = ACID_2;
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {

				log_d("pH, stage 3");
				ph_set = ACID_3;
			}

			_set_ph = g_data.getFloat(ph_set);
			float ph = _set_ph;
			float ph_hyst = g_data.getFloat(PH_HYST);

			// if PH is less
			float loLim = ph - ph_hyst;
			float hiLim = ph + ph_hyst;
			if (loLim > _lastPh) {
				log_d("pH is low: %.1f", _lastPh);
#ifdef RIG_DEBUG
				log_d("Target: %.1f", ph);
				log_d("Target - hyst: %.1f", loLim);
#endif
				_ph_state = PH_UP;
				_commencePH = true;
			}
			else if (ph + ph_hyst < _lastPh) {
				log_d("pH is high: %.1f", _lastPh);
#ifdef RIG_DEBUG
				log_d("Target: %.1f", ph);
				log_d("Target + hyst: %.1f", hiLim);
#endif
				_ph_state = PH_DW;
				_commencePH = true;
			}
			else {
				log_d("pH is OK");
#ifdef RIG_DEBUG
				log_d("Target pH: %.1f", ph);
				log_d("Target pH - hyst: %.1f", loLim);
				log_d("Target pH + hyst: %.1f", hiLim);
#endif
				_ph_state = NO_PH;
				_commencePH = false;
			}

			_preparePH = false;
			_pump_mils = millis();
		}


		void _squirtPH()
		{
			if (_paused)
				return;

			if (!_commencePH)
				return;

			if ((_pHu || _pHd) && millis() - _pump_mils > _pumpPHseconds*1000) {
				_pHu = false;
				_pHd = false;
				_commencePH = false;
				return;
			}

			// tank states
			bool* dig = io.getDigitalValues();
			bool up_empty = dig[DIG_KEY7];
			bool down_empty = dig[DIG_KEY6];

			_pumpPHseconds = g_data.getInt(ACID_PUMPS);

			if (_ph_state == PH_UP && !_pHu) {
				log_i("Driving pH up pump");
				if (up_empty) {
					log_e("pH UP Tank is empty");
					_commencePH = false;
					return;
				}

				// drive ph up (A) out with _pumpPHseconds
				io.driveTimed(MB_PU, _pumpPHseconds);
				_pump_mils = millis();
				_pHu = true;
			}
			else if (_ph_state == PH_DW && !_pHd) {
				log_i("Driving pH down pump");
				if (down_empty) {
					log_e("pH DOWN tank is empty");
					_commencePH = false;
					return;
				}

				// drive ph down (B) out with _pumpPHseconds
				io.driveTimed(MB_PD, _pumpPHseconds);
				_pump_mils = millis();
				_pHd = true;
			}
			_norm_time = datetime.now();
			_normalization_done = true;
		}

		void _updateRigType()
		{
			// requests change all the time, i'm pretty sure at some point
			// I will have to put back this functionality

			/*
			if (!_rigInitDone)
				return;
				*/

			switch (g_rig_type) {
				default: break;
				case RIG_CLASSIC: _classic(); break;
				case RIG_FLOOD: _flood(); break;
				case RIG_AERO: _aero(); break;
				case RIG_DRIP: _drip(); break;
				case RIG_OPENG: _openg(); break;
				case RIG_GREENH: _greenh(); break;
				case RIG_MIXSOL: _mixsol(); break;
			}
		}

		bool _mainpumpwindow = false;
		bool _aeropumpwindow = false;
		bool _rigInitDone = false;
	public:
		void setMainPumpWindow()
		{
			_mainpumpwindow = true;
		}

		void setAeroPumpWindow()
		{
			_aeropumpwindow = true;
		}

	private:
		int pumpA, pumpB, pumpC, phUpPump, phDwPump, waterPump, mainPump, aeroPump;
		bool _prevstate = false;
		bool _normalization_done = false;

		// h2o pump (tap pump)
		bool _pumpison = false;

		bool _mainpump = false;
		bool _haltpump = false;
		unsigned long _pumpMils = 0;
		unsigned long _pump_timeout = 60000;

		// h2o pump hyst
		bool _pumpHystFlag = false;
		unsigned long _pumpHystMils = 0;
		static constexpr unsigned long _pumpHyst = 5000;
		bool _no_user_pause = false;

		void _setPumpTimeOut(int minutes)
		{
			_pump_timeout = minutes * 60 * 1000;
		}

		void _driveH2Opump()
		{
			io.driveOut(PWR_PG_PORT_F, !io.getDigital(DIG_KEY5));
		}

		void _stopH2Opump()
		{
			if (_pumpison && millis() - _pumpMils > _pumpHyst)
				io.driveOut(PWR_PG_PORT_F, false);
		}

		void _haltH2Opump()
		{
			io.driveOut(PWR_PG_PORT_F, false);
		}

		void _classic()
		{
			pumpA = PWR_PG_PORT_A;
			pumpB = PWR_PG_PORT_B;
			pumpC = PWR_PG_PORT_C;
			phUpPump = PWR_PG_PORT_D;
			phDwPump = PWR_PG_PORT_E;
			waterPump = PWR_PG_PORT_F;
			mainPump = PWR_PG_PORT_G;

			// H2O pump
			if (!_haltpump && !_noH2Opump) {
				if ((io.getMainMid() & 1) && !_measuretime) {
					_driveH2Opump();
				}
				else if (!io.getMainMid() || !io.getMainHi() || _measuretime) {
					_stopH2Opump();
				}
			}
			else if (!_noH2Opump) {
				// indicate that pump is halted
				_haltH2Opump();
			}

			// reset flag if pump is off
			if (!io.getOut(waterPump)) {
				_pumpison = false;
			}

			// raise flag if pump is on
			if (!_pumpison && io.getOut(waterPump)) {
				_pumpison = true;
				// remember when
				_pumpMils = millis();
			}

			_setPumpTimeOut(g_data.getInt(PUMP_TIMEOUT));

			if (!_haltpump && _pumpison && millis() - _pumpMils > _pump_timeout && !io.noH2Opump()) {
				_haltpump = true;
				ezplant_log::log(
						[this](void*)
						{
							this->resetPump();
						}
						, ER_PUMP);
				log_e("H20 pump: emergency timeout shut down");
			}

			// main pump
			_updateMainPump();

		}

		void _layer()
		{
		}

		void _flood()
		{
		}

		void _aero()
		{
		}

		void _drip()
		{
		}

		void _openg()
		{
		}

		void _greenh()
		{
		}

		void _mixsol()
		{
		}

} g_rig;

#endif
