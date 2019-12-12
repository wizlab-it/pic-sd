/*
 * 20190412.070
 * SD Card
 *
 * File: SPI.c
 * Processor: PIC12F1840
 * Author: wizlab.it
 */

#include "SD.h"

void SD_SPI_Init(void) {
    //Set Alternate PIN Functions
    APFCONbits.SDOSEL = 0;  //SDO on RA0 (pin 7)
    APFCONbits.SSSEL = 0;   //SS on RA3 (pin 4)

    //Configure pins to work as SPI
    TRISAbits.TRISA4 = 0;   //Output, SPI CS
    TRISAbits.TRISA2 = 1;   //Input, SPI SDI
    TRISAbits.TRISA0 = 0;   //Output, SPI SCK Master Mode
    TRISAbits.TRISA0 = 0;   //Output, SPI SDO
    _SD_SPI_CS = 1;         //Disable slaves

    //Configure Serial Port
    SSP1STATbits.CKE = 1;           //Transmit from active to idle
    SSP1CON1bits.CKP = 0;           //Idle is low level
    SSP1CON1bits.SSPM = 0b0010;     //SPI Master mode, clock = FOSC / 64
    SSP1CON1bits.SSPEN = 1;         //Serial port is enabled
}

void SD_SPI_Clock(uint8_t count) {
    for(uint8_t i=0; i!=count; i++) {
        SD_SPI_Write(0xFF);
    }
}

uint8_t SD_SPI_Write(uint8_t byte) {
    SSP1BUF = byte;             //Send byte
    while(!SSP1STATbits.BF);    //Wait until cycle complete
    return SSP1BUF;
}

uint8_t SD_SPI_Read(void) {
    uint8_t response;
    response = SSP1BUF;
    response = SD_SPI_Write(0xFF);
    return response;
}

void SD_Card_Enable(void) {
    SD_SPI_Clock(8);    //Send clocks to card sync
    _SD_SPI_CS = 0;     //Enable card

    //Check if busy from a previous action
    SD_SPI_Clock(1);
    SD_Card_WaitIfBusy();
}

void SD_Card_Disable(void) {
    _SD_SPI_CS = 1;     //Disable card
    SD_SPI_Clock(1);    //Send clocks to flush bus
}

uint8_t SD_Card_Command(uint8_t cmd, uint32_t a) {
    uint8_t response;
    uint8_t payload[5];
    uint8_t crc = 0;

    //Build command payload (1 cmd byte + 4 argument bytes) and calculate CRC
    payload[0] = cmd | 0x40;
    payload[1] = (uint8_t) (a >> 24);
    payload[2] = (uint8_t) (a >> 16);
    payload[3] = (uint8_t) (a >> 8);
    payload[4] = (uint8_t) a;
    crc = SD_Card_Crc7(0, payload, 5);

    //Send command (1 dummy byte + command payload + CRC)
    SD_SPI_Write(0xFF);
    for(uint8_t i=0; i<5; i++) SD_SPI_Write(payload[i]);
    SD_SPI_Write(crc);

    //Wait for a response
    for(uint8_t i=0; i<16; i++) {
        response = SD_SPI_Read();
        if(response != 0xFF) break;
    }

    return response;
}

uint8_t SD_Card_Crc7(uint8_t crc, uint8_t *data, uint8_t len) {
    for(uint16_t i=0; i<len; i++) {
        uint8_t c = *data++;
        for(uint8_t j=0; j<8; j++) {
            crc <<= 1;
            if((c ^ crc) & 0x80) crc ^= 0x09;
            c <<= 1;
        }
    }
    crc <<= 1;
    return ++crc;
}

uint16_t SD_Card_Crc16(uint16_t crc, uint8_t *data, uint16_t len) {
    for(uint16_t i=0; i<len; i++) {
        crc = SD_Card_Crc16Byte(crc, *data++);
    }
    return crc;
}

uint16_t SD_Card_Crc16Byte(uint16_t crc, uint8_t c) {
    crc ^= (uint16_t)(c << 8);
    for(uint8_t j=0; j<8; j++) {
        if((crc & 0x8000) != 0) {
            crc = (uint16_t)((crc << 1) ^ 0x1021);
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

void SD_Card_Init(void) {
    //Init flags
    SD_FLAGS.cardResetOK = 0;
    SD_FLAGS.cardInitOK = 0;
    SD_FLAGS.isCardActive = 0;

    //Wait 2ms and then enable card
    __delay_ms(100);
    SD_Card_Enable();

    //Send reset command and wait for 0x01 (idle status)
    for(uint8_t i=250; i!=0; i--) {
        if(SD_Card_Command(_SD_CMD_RESET, 0x00000000) == 0x01) {
            SD_FLAGS.cardResetOK = 1;
            break;
        }
        __delay_ms(10);
    }

    //If reset was fine, then send init command: try at first with SDC command, then MMC. Wait for 0x00 (active status)
    if(SD_FLAGS.cardResetOK == 1) {
        for(uint8_t i=250; i!=0; i--) {
            uint8_t response = SD_Card_Command(_SD_CMD_INIT_SDC, 0x00000000);
            if(response & 0x04) response = SD_Card_Command(_SD_CMD_INIT, 0x00000000);
            if(response == 0x00) {
                SD_FLAGS.cardInitOK = 1;
                break;
            }
            __delay_ms(10);
        }
    }

    //If init was fine, then set block size
    if(SD_FLAGS.cardInitOK == 1) {
        for(uint8_t i=250; i!=0; i--) {
            if(SD_Card_Command(_SD_CMD_SET_BLOCKLEN, _SD_BLOCK_SIZE) == 0x00) {
                SD_FLAGS.cardBlockSizeOK = 1;
                break;
            }
        }
    }

    //If block size has been set, then initialization process was successful so read CID and CSD, and set Active flag
    if(SD_FLAGS.cardBlockSizeOK == 1) {
        SD_Card_ReadReg16(_SD_CMD_READ_CID, (uint8_t *)&SD_CID);
        SD_Card_ReadReg16(_SD_CMD_READ_CSD, (uint8_t *)&SD_CSD);
        SD_FLAGS.isCardActive = 1;
    }

    SD_Card_Disable();
}

void SD_Card_ReadReg16(uint8_t reg, uint8_t *dst) {
    if(SD_Card_Command(reg, 0x00000000) == 0x00) {
        SD_Card_WaitStartToken();

        //Read data (16 bytes)
        dst += 16;
        for(uint8_t i=0; i<16; i++) {
            *--dst = SD_SPI_Read();
        }

        //End read
        SD_Card_Command(_SD_CMD_END_READ, 0x00000000);
    }
}

uint32_t SD_Card_GetSize(void) {
    if(SD_CSD.v1.csd_ver == 0) {
        uint8_t read_bl_len = SD_CSD.v1.read_bl_len;
        uint16_t c_size = (SD_CSD.v1.c_size_high << 10) | (SD_CSD.v1.c_size_mid << 2) | SD_CSD.v1.c_size_low;
        uint8_t c_size_mult = (SD_CSD.v1.c_size_mult_high << 1) | SD_CSD.v1.c_size_mult_low;
        return ((uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7)) * _SD_BLOCK_SIZE;
    } else if (SD_CSD.v2.csd_ver == 1) {
        uint32_t c_size = ((uint32_t)SD_CSD.v2.c_size_high << 16) | (SD_CSD.v2.c_size_mid << 8) | SD_CSD.v2.c_size_low;
        return ((c_size + 1) << 10) * _SD_BLOCK_SIZE;
    }
    return 0;
}

uint16_t SD_Card_ProcessCRC(void) {
    //Read CRC (2 bytes)
    uint16_t crc = SD_SPI_Read() << 8;
    crc += SD_SPI_Read();
    return crc;
}

uint8_t SD_Card_IsActive(void) {
    return SD_FLAGS.isCardActive;
}

void SD_Card_WaitIfBusy(void) {
    for(uint8_t i=0; i<250; i++) {
        if(SD_SPI_Read() != 0x00) return;
        __delay_ms(1);
    }
}

void SD_Card_WaitStartToken(void) {
    for(uint8_t i=0; i<250; i++) {
        if(SD_SPI_Read() == _SD_BLOCK_SINGLE_TOKEN) return;
        __delay_ms(1);
    }
}

uint8_t SD_Card_RWInit(uint32_t addr, uint8_t readOrWrite, uint8_t singleOrMultiBlock) {
    SD_Card_Enable();

    //Set flags
    SD_FLAGS.readOrWrite = readOrWrite;
    SD_FLAGS.singleOrMultiBlock = singleOrMultiBlock;

    //Initiate R/W process
    if(readOrWrite == _SD_WRITE_FLAG) {
        if(singleOrMultiBlock == _SD_BLOCK_MULTI_FLAG) {
            if(SD_Card_Command(_SD_CMD_WRITE_MULTI, addr) == 0x00) {
                return 1;
            }
        } else {
            if(SD_Card_Command(_SD_CMD_WRITE_SINGLE, addr) == 0x00) {
                SD_SPI_Write(_SD_BLOCK_SINGLE_TOKEN);
                return 1;
            }
        }
    } else {
        if(singleOrMultiBlock == _SD_BLOCK_MULTI_FLAG) {
            SD_Card_Command(_SD_CMD_READ_MULTI, addr);
            return 1;
        } else {
            SD_Card_Command(_SD_CMD_READ_SINGLE, addr);
            SD_Card_WaitStartToken();
            return 1;
        }
    }

    //If here, initialization failed, so disable card and exit
    SD_Card_Disable();
    return 0;
}

uint16_t SD_Card_RWEnd(void) {
    //If single block, process CRC
    uint16_t crc = 0;
    if(SD_FLAGS.singleOrMultiBlock == _SD_BLOCK_SINGLE_FLAG) {
        crc = SD_Card_ProcessCRC();
    }

    //Send stop R/W block command
    SD_Card_Command(((SD_FLAGS.readOrWrite == _SD_WRITE_FLAG) ? _SD_CMD_END_WRITE : _SD_CMD_END_READ), 0x00000000);
    SD_Card_Disable();
    return crc;
}

uint8_t SD_Card_ReadBlock(uint32_t addr, uint8_t *dst) {
    if(SD_Card_RWInit(addr, _SD_READ_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
            *dst++ = SD_SPI_Read();
        }
        SD_Card_RWEnd();
        return _SD_OK_FLAG;
    }

    //If here read failed
    return _SD_ERR_FLAG;
}

uint8_t SD_Card_WriteBlock(uint32_t addr, uint8_t *src) {
    if(SD_Card_RWInit(addr, _SD_WRITE_FLAG, _SD_BLOCK_SINGLE_FLAG)) {
        for(uint16_t i=0; i<_SD_BLOCK_SIZE; i++) {
            SD_SPI_Write(*src++);
        }
        SD_Card_RWEnd();
        return _SD_OK_FLAG;
    }

    //If here write failed
    return _SD_ERR_FLAG;
}

void SD_Card_RWStartMulti(void) {
    if(SD_FLAGS.readOrWrite == _SD_WRITE_FLAG) {
        SD_Card_WaitIfBusy();                   //Wait if busy
        SD_SPI_Write(_SD_BLOCK_MULTI_TOKEN);    //Send start token
    } else {
        SD_Card_WaitStartToken();
    }
}

uint16_t SD_Card_RWStopMulti(void) {
    uint16_t crc = SD_Card_ProcessCRC();

    //If write, then check data response
    if(SD_FLAGS.readOrWrite == _SD_WRITE_FLAG) {
        SD_SPI_Clock(1);
        SD_SPI_Read();
    }

    return crc;
}