/*
 * 20190412.028
 * SD Card
 *
 * File: main.c
 * Processor: PIC12F1840
 * Author: wizlab.it
 *
 * Read SD Card (raw device access) on Linux
 * # xxd -l 1200 -c 32 /dev/mmcblk0
 *
 */

#include "main.h"

/*==============================================================================
 * Main routine
 *  - Initialize system
 *  - Loop forever
 *============================================================================*/
void main(void) {
    init();
    SD_SPI_Init();
    SD_Card_Init();

    //Looooooop
    while(1) loop();
}


/*==============================================================================
 * Loop routine
 *============================================================================*/
void loop(void) {
    //Verify if card has been activated
    if(!SD_Card_IsActive()) {
        while(1) {
            _LED = !_LED;
            __delay_ms(200);
        }
    }









    //Write 2 single blocks
    if(SD_Card_RWInit(0x00000000, _SD_WRITE_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
            SD_SPI_Write(0xE0);
        }
        SD_Card_RWEnd();
    }

    if(SD_Card_RWInit(0x00000200, _SD_WRITE_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
            SD_SPI_Write(0xE1);
        }
        SD_Card_RWEnd();
    }





    //Write card size
    if(SD_Card_RWInit(0x00000600, _SD_WRITE_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        uint32_t cardSize = SD_Card_GetSize();
        uint8_t *p = (uint8_t *)&cardSize;
        for(uint16_t i=_SD_BLOCK_SIZE; i>0; i-=16) {
            SD_SPI_Write(*(p + 3));
            SD_SPI_Write(*(p + 2));
            SD_SPI_Write(*(p + 1));
            SD_SPI_Write(*p);
            for(uint8_t j=0; j!=12; j++) {
                SD_SPI_Write(0x00);
            }
        }
        SD_Card_RWEnd();
    }





    //Write a block then read it and check the sum (total is 1032)
    if(SD_Card_RWInit(0x00000800, _SD_WRITE_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        SD_SPI_Write(0x09);
        for(uint16_t i=0; i<(_SD_BLOCK_SIZE - 2); i++) {
            SD_SPI_Write(0x01);
        }
        SD_SPI_Write(0x02);
        SD_Card_RWEnd();
    }

    uint16_t sum = 0;
    if(SD_Card_RWInit(0x00000800, _SD_READ_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
            sum += SD_SPI_Read();
        }
        SD_Card_RWEnd();
    }
    if(sum == 521) {
        for(uint8_t i=0; i<10; i++) {
            _LED = !_LED;
            __delay_ms(50);
        }
        _LED = 0;
        __delay_ms(500);
    }





    //Write 1 multi block
    if(SD_Card_RWInit(0x00000A00, _SD_WRITE_FLAG, _SD_BLOCK_MULTI_FLAG)) {
        for(uint8_t j=0x10; j<0x15; j++) {
            SD_Card_RWStartMulti();
            for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
                SD_SPI_Write(j);
            }
            SD_Card_RWStopMulti();
        }
        SD_Card_RWEnd();
    }










    //Write 1 multi block (4 sectors) then multiread the 2 central sectors and calculate the sum (2578)
    if(SD_Card_RWInit(0x0000B000, _SD_WRITE_FLAG, _SD_BLOCK_MULTI_FLAG)) {
        for(uint8_t j=1; j<10; j++) {
            SD_Card_RWStartMulti();
            SD_SPI_Write(0x04);
            for(uint16_t i=0; i<(_SD_BLOCK_SIZE - 2); i++) {
                SD_SPI_Write(j);
            }
            SD_SPI_Write(0x07);
            SD_Card_RWStopMulti();
        }
        SD_Card_RWEnd();
    }

    uint16_t sum2 = 0;
    if(SD_Card_RWInit(0x0000B200, _SD_READ_FLAG, _SD_BLOCK_MULTI_FLAG)) {
        for(uint8_t j=0; j<2; j++) {
            SD_Card_RWStartMulti();
            for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
                sum2 += SD_SPI_Read();
            }
            SD_Card_RWStopMulti();
        }
        SD_Card_RWEnd();
    }
    if(sum2 == 2572) {
        for(uint8_t i=0; i<10; i++) {
            _LED = !_LED;
            __delay_ms(100);
        }
        _LED = 0;
        __delay_ms(500);
    }










    //End forever
    while(1) {
        _LED = !_LED;
        __delay_ms(50);
    }
}