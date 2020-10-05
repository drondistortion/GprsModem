#ifndef gprsModem_h
#define gprsModem_h

#include <Arduino.h>
#include <Stream.h>
#include <Client.h>
#include <SoftwareSerial.h>

#include "IPAddress.h"
//#include "Dns.h"

constexpr char* START = "AT+CIPSTART=";
//constexpr char* PASS_THRU_MODE = "AT+CIPTMODE=1";

#define GPRS_TIMEOUT 4000
#define SER_TIMEOUT 4000

class GprsModem {
	public:
		GprsModem(const HardwareSerial& serial): _serial(serial), _native_serial(true){;}
		GprsModem(const SoftwareSerial& serial): _s_serial(serial), _native_serial(false){;}
		bool begin();
	private:
		uint32_t _checkRate(const bool&);
		bool _native_serial;
		bool _begin(const bool&);
		const HardwareSerial& _serial;
		const SoftwareSerial& _s_serial;
};

class GprsClient: public Client {
	friend class GprsModem;
	public:
		GprsClient(const HardwareSerial& serial): _serial(serial) {_timeout = GPRS_TIMEOUT;};
		GprsClient(const SoftwareSerial& serial): _serial(serial) {_timeout = GPRS_TIMEOUT;};
		int connect(const char* host, uint16_t port);
		int connect(IPAddress ip, uint16_t port);
		int connect(const char* host, uint16_t port, const char* protocol);
		int available();
		size_t write(uint8_t);
		size_t write(const uint8_t *buf, size_t size);
		int read();
		void stop();
		bool begin();
		int read(uint8_t* buf, size_t size);

		// c++ bullshit:
		//int read(uint8_t* buf, size_t size) {return 0;}
		int availableForWrite() { return 0;}
		void flush() {};
		int peek() { return 0; }
		uint8_t connected() { return 0; }
		operator bool() { return false; }

	private:
		String _protocol = "TCP";
		const Stream& _serial;
		static bool waitResp(unsigned long time, const String& aresp, const Stream& stream)
		{
			unsigned long timer = millis();

			String buf = "";

			while (millis() - timer < time) {
				if (stream.available()) {
					timer = millis();
					char c = stream.read();
					buf += String(c);
				}
			}

			/*
			   if (aresp == "CONNECT OK") {
			   int index = buf.indexOf("M:");
			   Serial.print("index of socket:\t");
			   Serial.println(index);
			   if (index > -1) {
			   String subs = buf.substring(index+2, index+3);
			   Serial.println(subs);
			   mux = subs.toInt();
			   }
			   }
			   */

			//Serial.println(buf);
			/*
			   if (buf.indexOf((String)"ERROR")) {
			   SerialAT.println("AT+CIPCLOSE");
			   SerialMon.println("closing");
			   return false;
			   }
			   */
			if (aresp != nullptr)
				//return buf.indexOf((String)"\r\n" + aresp + "\r\n") > -1 ? true : false;
				return buf.indexOf((String)aresp) > -1 ? true : false;
			else
				return true;
		}



};

#endif
