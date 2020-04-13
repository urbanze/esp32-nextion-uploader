#include "nxt.h"
#include "nxt.cpp"



extern uint8_t Rbin_s[] asm("_binary_tester_tft_start");//Embedded .tft file to upload
extern uint8_t Rbin_e[] asm("_binary_tester_tft_end");


extern "C" void app_main()
{
    NXT nxt; //Create object
    nxt.init(UART_NUM_1, 115200, 22, 23); //Init UART

    if (nxt.connect(197844)) //Try to connect in Nextion with 115200bps (set in .init)
    {
        int32_t ctr = 0;

        //Send all bytes and wait function return 0
        while (nxt.send(Rbin_s+ctr, 4096))
            {ctr += 4096;}

        ESP_LOGI(__func__, "END");
    }
}
