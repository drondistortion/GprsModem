#include "GprsModem.h"

//#define H_CAST (*(HardwareSerial*)_serial)
//#define S_CAST (*(SoftwareSerial*)_serial)
#define H_SPEED 115200
#define S_SPEED 9600

bool GprsModem::begin()
{
	return _begin(_native_serial);
}

bool GprsModem::_begin(const bool& flag)
{
	uint32_t rate = _checkRate(flag);


	if (rate == -1)
		return false;
	else {
		if (flag && (rate != H_SPEED)) {
			_serial.println((String)"ATZ+IPR=" + H_SPEED);
			GprsClient::waitResp(2000UL, "OK", _serial);
			_serial.end();
			delay(100);
			_serial.begin(H_SPEED);
		}
		else if (!flag && (rate != S_SPEED)) {
			_s_serial.println((String)"ATZ+IPR=" + S_SPEED);
			GprsClient::waitResp(2000UL, "OK", _s_serial);
			_serial.end();
			delay(1000);
			_s_serial.begin(S_SPEED);
		}
	}
	return true;


/*
	if (flag) {
		_serial.end();
		delay(100);
		_serial.setTimeout(SER_TIMEOUT);
		_serial.begin(115200);
	}
	else {
		//_s_serial.println(AT+k
		_s_serial.begin(115200);
	}
	if (rate != -1)
		return true;
	else
		return false;
*/
}


uint32_t GprsModem::_checkRate(const bool& flag)
{
	static uint32_t rates[] = {
		115200, 9600, 57600, 38400, 19200, 74400, 74880,
		230400, 460800, 2400, 4800, 14400, 28800
	};

	for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
		uint32_t rate = rates[i];
		if (flag)
			_serial.begin(rate);
		else
			_s_serial.begin(rate);

		delay(10);

		//String resp = "";

		for (uint8_t j = 0; j < 5; j++) {

			if (flag) {
				_serial.println("AT");
				delay(10);
				//resp = _serial.readString();
				if (GprsClient::waitResp(2000UL, "OK", _serial))
						return rate;
			}
			else {
				_s_serial.println("AT");
				delay(10);
				//resp = _s_serial.readString();
				if (GprsClient::waitResp(2000UL, "OK", _s_serial)) {
					Serial.println("Softserial");
					return rate;
				}
			}

			/*
			if (resp.indexOf("OK") > -1) {
				return rate;
			}
			*/
		}
	}
	return -1;
}

bool GprsClient::begin()
{
	//_serial.begin(115200);

	//_serial.setTimeout(SER_TIMEOUT);
	//delay(500);

	_serial.println("AT+CGATT=1");
	if (!waitResp(2000L, "+CGATT:1", _serial)) {
		//Serial.println("bang!");
		return false;
	}
	_serial.println("AT+CGDCONT=1,\"IP\",\"internet\"");
	if (!waitResp(2000L, "OK", _serial))
		return false;
	_serial.println("AT+CGACT=1,1");
	if (!waitResp(2000L, "OK", _serial))
		return false;
	/*
	_serial.println("AT+CIPMUX=1");
	if (!waitResp(2000L, "OK", _serial))
		return false;
		*/
	return true;
}

#define CONNECT_STATUS "CONNECT OK"

int GprsClient::connect(const char* host, uint16_t port)
{
	//Serial.println(_serial.getTimeout());
	String reqstr = (String)START + "\"" + _protocol + "\""+ ",\"" + host + "\"," + port;
	//String reqstr = (String)START + ",\"" + host + "\"," + port;
	_serial.println(reqstr);

	while(!_serial.available());

	if (!waitResp(2000ul, String(CONNECT_STATUS), _serial))
		return 0;

	reqstr = "AT+CIPTMODE=1";
	_serial.println(reqstr);

	while(!_serial.available());


	if (waitResp(2000ul, "OK", _serial))
		return 1;
	return 0;
}

int GprsClient::connect(const char* host, uint16_t port, const char* protocol)
{
	//Serial.println(_serial.getTimeout());

	_protocol = protocol;
	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + host + "\"," + port;
	//String reqstr = (String)START + ",\"" + host + "\"," + port;
	_serial.println(reqstr);

	while(!_serial.available());

	if (!waitResp(2000ul, String(CONNECT_STATUS), _serial))
		return 0;

	reqstr = "AT+CIPTMODE=1";
	_serial.println(reqstr);

	while(!_serial.available());


	if (waitResp(2000ul, "OK", _serial))
		return 1;
	return 0;
}

int GprsClient::connect(IPAddress ip, uint16_t port)
{
#if defined(ESP8266) || defined(ESP32)
	if (ip == IPAddress((uint32_t)0) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#else
	if (ip == IPAddress(0ul) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#endif
	String addr = "";
	for (size_t i = 0; i < 4; i++) {
		if (i)
			addr += '.';
		addr += ip[i];
	}

	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + addr + "\"," + port;
	_serial.println(reqstr);

	while(!_serial.available());

	if (!waitResp(2000ul, String(CONNECT_STATUS), _serial))
		return 0;

	reqstr = "AT+CIPTMODE=1";
	_serial.println(reqstr);

	while(!_serial.available());


	if (waitResp(2000ul, "OK", _serial))
		return 1;
	return 0;
}


size_t GprsClient::write(uint8_t B)
{
	return _serial.write(B);
}

size_t GprsClient::write(const uint8_t* buf, size_t size)
{
	return _serial.write(buf, size);
}

int GprsClient::available()
{
	//return _serial.available();
	//delay(1);
	unsigned long timer = micros();
	while(micros() - timer < 1000UL) {
		if (_serial.available())
			return _serial.available();
	}
	return -1;
}

int GprsClient::read()
{
	return _serial.read();
}

int GprsClient::read(uint8_t* buf, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		buf[i] = (uint8_t)_serial.read();
	}

	return (int)size;
}

void GprsClient::stop()
{
	_serial.println("AT+CIPCLOSE");
	while (!_serial.available());
	waitResp(2000ul, "OK", _serial);
}
