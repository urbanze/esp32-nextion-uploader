#ifndef nxt_H
#define nxt_H

#include <esp_err.h>
#include <esp_log.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <string.h>



/**
  	@file nxt.h
	@author Jose Morais

	Classe para uso do Nextion Screen Uploader com ESP32 e IDF


	MIT License

	Copyright (c) 2020 José Morais

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

class NXT
{
	private:
		uart_port_t _uart; ///< UART port number in use internally
		int16_t _partial = 0; ///< Partial 4KB packet control (wait nextion 0x05)
		int32_t _sent = 0; ///< Bytes sent
		int32_t _filesize = 0; ///< Total file size
		char _bff[128] = {0}; ///< Internal read buffer

		void write(uint8_t *bff, uint16_t size); //Write raw buffer
		void write(const char *bff); //Write char with tail
		void read(); //Read response from nextion
		int16_t available(); //Available bytes to read


	public:
		void init(uart_port_t uart_num, int32_t baud_rate, int8_t pin_tx, int8_t pin_rx); //Init UART
		
		int8_t connect(int32_t file_size); //Connect (check) in nextion
		int8_t send(uint8_t *bff, uint16_t size); //Send bytes to nextion (already in download mode)

};


#endif
