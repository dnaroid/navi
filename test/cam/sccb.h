#pragma once

#include <Arduino.h>

/*
Serial Camera Control Bus

Used to send commands to OV7670 etc


This is not speed critical so the standard digitalWrite etc are used
*/

class cSCCB
{
public:

	cSCCB() { m_active = false; }

	void begin(uint8_t pin_sda, uint8_t pin_scl, uint32_t delay_us = 100);
	void start();
	void stop();
	bool write(uint8_t m_data);
	uint8_t read();
	uint8_t readSlaveRegister(uint8_t addr, uint8_t data);
	bool writeSlaveRegister(uint8_t addr, uint8_t reg, uint8_t data);

private:
	bool m_active;
	uint32_t m_delay_us;
	uint8_t m_pin_sda;
	uint8_t m_pin_scl;
};