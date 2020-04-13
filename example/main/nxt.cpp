#include "nxt.h"

/**
 * @brief Internal UART raw write.
 *
 * @param [*bff]: Array of bytes to send.
 * @param [size]: Number of bytes to send.
 */
void NXT::write(uint8_t *bff, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        const char c = bff[i];
		uart_write_bytes(_uart, &c, 1);
    }

    uart_wait_tx_done(_uart, pdMS_TO_TICKS(500));
}

/**
 * @brief Internal UART string write.
 *
 * @param [*bff]: Array of bytes (char) to send.
 */
void NXT::write(const char *bff)
{
    int16_t size = strlen(bff);

	uart_write_bytes(_uart, bff, size);


	const char tmp[3] = {0xFF, 0xFF, 0xFF};
	uart_write_bytes(_uart, tmp, 3);

    uart_wait_tx_done(_uart, pdMS_TO_TICKS(500)); 
}

/**
 * @brief Internal UART read.
 */
void NXT::read()
{
    uint8_t tmp[127] = {0};

    for (uint16_t i = 0; i < 200; i++)
	{
		vTaskDelay(pdMS_TO_TICKS(10));

		if (available() > 0)
		{
			uart_read_bytes(_uart, tmp, 127, pdMS_TO_TICKS(5));
			
			memset(_bff, 0, 128);
			snprintf(_bff, 128, "%s", tmp);
            return;
        }
    }
}

/**
 * @brief Internal UART available bytes to read.
 *
 * @return Available bytes.
 */
int16_t NXT::available()
{
	size_t total = 0;
	uart_get_buffered_data_len(_uart, &total);

	return total;
}



/**
 * @brief Init UART to upload firmware in Nextion.
 * 
 * @param [uart_num]: Free UART port (UART_NUM_0, UART_NUM_1...).
 * @param [baud_rate]: Baud rate (default 115200).
 * @param [pin_tx]: UART TX pin.
 * @param [pin_rx]: UART RX pin.
 */
void NXT::init(uart_port_t uart_num, int32_t baud_rate, int8_t pin_tx, int8_t pin_rx)
{
    uart_config_t uart_cfg;
	_uart = uart_num;

	uart_cfg.baud_rate = baud_rate;
	uart_cfg.data_bits = UART_DATA_8_BITS;
	uart_cfg.parity = UART_PARITY_DISABLE;
	uart_cfg.stop_bits = UART_STOP_BITS_1;
	uart_cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
	uart_cfg.rx_flow_ctrl_thresh = 0;
	uart_cfg.use_ref_tick = 0;

	ESP_ERROR_CHECK(uart_param_config(_uart, &uart_cfg));
	ESP_ERROR_CHECK(uart_set_pin(_uart, gpio_num_t(pin_tx), gpio_num_t(pin_rx), UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	ESP_ERROR_CHECK(uart_driver_install(_uart, 512, 0, 0, NULL, 0));
}

/**
 * @brief Try to connect and prepare Nextion to download firmware over UART.
 * 
 * Send() need connect() return 1.
 * 
 * @param [file_size]: Total file size that will send to Nextion.
 * 
 * @return Fail (0) or Sucess (1).
 */
int8_t NXT::connect(int32_t file_size)
{
	_filesize = file_size;

    write("DRAKJHSUYDGBNCJHGJKSHBDN");
	read();
	if (_bff[0] != 0x1A)
		{ESP_LOGE(__func__, "Wrong baud rate"); return 0;}


	write("connect");
    read();
	if (strstr(_bff, "comok") == NULL)
		{ESP_LOGE(__func__, "Wrong response 1"); return 0;}
	
	ESP_LOGI(__func__, "Nextion info: %s", _bff);

	snprintf(_bff, 128, "whmi-wri %d,921600,0", file_size);
	write((const char*)_bff);
	uart_set_baudrate(_uart, 921600);
	read();
	for (uint8_t i = 0; i < 128; i++)
	{
		if (_bff[i] == 0x05)
		{
			ESP_LOGI(__func__, "Nextion ready");
			
			return 1;
		}
	}
	
	ESP_LOGE(__func__, "Wrong response 2");
	return 0;
}

/**
 * @brief Send packets to Nextion and wait 0x05 over 4KB sent.
 * 
 * Send() need connect() return 1.
 * 
 * @param [*bff]: Array of bytes to send.
 * @param [size]: Number of bytes to send.
 * 
 * @return Fail (0), All bytes sent (0) or Sucess (1).
 */
int8_t NXT::send(uint8_t *bff, uint16_t size)
{
	if (_sent + size > _filesize)
		{size = _filesize - _sent;}

	if (_partial + size > 4096)
	{
		int16_t tmp = 4096 - _partial;

		write(bff, tmp);
		read();
		if (_bff[0] != 0x05)
			{ESP_LOGE(__func__, "ERROR"); return 0;}

		size -= tmp;
		write(bff + tmp, size);
		_partial = size;
		_sent += tmp;
	}
	else
	{
		write(bff, size);
		_partial += size;
	}

	_sent += size;

	if (_sent == _filesize)
	{
		//ESP_LOGI(__func__, "Sent all bytes");
		_sent = 0;
		_partial = 0;

		uart_set_baudrate(_uart, 115200);
		vTaskDelay(pdMS_TO_TICKS(1600));
		uart_flush(_uart);
		write("DRAKJHSUYDGBNCJHGJKSHBDN");
		read();
		return 0;
	}
	
	return 1;
}
