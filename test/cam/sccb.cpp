#include <sccb.h>


void cSCCB::begin(uint8_t pin_sda, uint8_t pin_scl, uint32_t delay_us)
{
	m_pin_sda = pin_sda;
	m_pin_scl = pin_scl;
	m_delay_us = delay_us;
	m_active = true;

	pinMode(m_pin_sda, OUTPUT);
	pinMode(m_pin_scl, OUTPUT);

	digitalWrite(m_pin_sda, HIGH);
	digitalWrite(m_pin_scl, HIGH);
	delayMicroseconds(m_delay_us);
}

void cSCCB::start(void)
{
	if (!m_active) return;

	digitalWrite(m_pin_sda, HIGH);
	delayMicroseconds(m_delay_us);
	digitalWrite(m_pin_scl, HIGH);
	delayMicroseconds(m_delay_us);
	digitalWrite(m_pin_sda, LOW);
	delayMicroseconds(m_delay_us);
	digitalWrite(m_pin_scl, LOW);
	delayMicroseconds(m_delay_us);
}

void cSCCB::stop(void) //SCCB
{
	if (!m_active) return;

	digitalWrite(m_pin_sda, LOW);
	delayMicroseconds(m_delay_us);
	digitalWrite(m_pin_scl, HIGH);
	delayMicroseconds(m_delay_us);
	digitalWrite(m_pin_sda, HIGH);
	delayMicroseconds(m_delay_us);
}

bool  cSCCB::write(uint8_t m_data)
{
	if (!m_active) return false;

	bool tem = true;

	for (uint8_t j = 0x80; j; j >>= 1) // msb 1st
	{
		if (m_data & j)
		{
			digitalWrite(m_pin_sda, HIGH);
		}
		else
		{
			digitalWrite(m_pin_sda, LOW);
		}
		delayMicroseconds(m_delay_us);
		digitalWrite(m_pin_scl, HIGH);
		delayMicroseconds(m_delay_us);
		digitalWrite(m_pin_scl, LOW);
		delayMicroseconds(m_delay_us);
	}

	delayMicroseconds(m_delay_us);
	pinMode(m_pin_sda, INPUT); //m_pin_sda????????(OV7670)???
	digitalWrite(m_pin_sda, LOW); //???????
	delayMicroseconds(m_delay_us);

	digitalWrite(m_pin_scl, HIGH);
	delayMicroseconds(m_delay_us);

	tem = !digitalRead(m_pin_sda); // check

	digitalWrite(m_pin_scl, LOW);
	delayMicroseconds(m_delay_us);
	pinMode(m_pin_sda, OUTPUT);

	return tem;
}

uint8_t cSCCB::read()
{
	uint8_t data = 0;

	pinMode(m_pin_sda, INPUT);
	delayMicroseconds(m_delay_us);

	for (uint8_t mask = 0x80; mask; mask >>= 1)
	{
		digitalWrite(m_pin_scl, HIGH); // I2C_CLK = HIGH;
		delayMicroseconds(m_delay_us);

		if (digitalRead(m_pin_sda))
		{
			data |= mask; //  I2C_DATA;
		}

		delayMicroseconds(m_delay_us);
		digitalWrite(m_pin_scl, LOW); //	I2C_CLK = LOW;
		delayMicroseconds(m_delay_us);
	}

	pinMode(m_pin_sda, OUTPUT);

	return data;
}

bool cSCCB::writeSlaveRegister(uint8_t addr, uint8_t reg, uint8_t data)
{
	if (!m_active) return false;

	start();

	if (write(addr) == 0)
	{
		Serial.println(" Write Error 0x42");
		stop();
		return(false);
	}
	delayMicroseconds(m_delay_us);

	if (write(reg) == 0)
	{
		stop();
		return(false);
	}
	delayMicroseconds(m_delay_us);

	if (!write(data))
	{
		stop();
		return(false);
	}
	stop();

	return(true);
}

uint8_t cSCCB::readSlaveRegister(uint8_t addr, uint8_t data)
{
	uint8_t inData;

	start();

	if (!write(addr))
	{
		stop();
		return(0);
	}

	if (!write(data))
	{
		stop();
		return(0);
	}

	stop();

	start();

	if (!write(addr | 1))
	{
		stop();
		return(0);
	}

	inData = read();

	stop();

	return inData;
}
