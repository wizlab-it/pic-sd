/*
 * 20190412.074
 * SD Card
 *
 * File: SPI.h
 * Processor: PIC12F1840
 * Author: wizlab.it
 */

#ifndef SPI_H
#define	SPI_H

#include "commons.h"

#define _SD_SPI_CS      PORTAbits.RA4   //SPI CS

#define _SD_CMD_RESET           0
#define _SD_CMD_INIT            1
#define _SD_CMD_INIT_SDC        41
#define _SD_CMD_READ_CSD        9
#define _SD_CMD_READ_CID        10
#define _SD_CMD_END_READ        12
#define _SD_CMD_END_WRITE       13
#define _SD_CMD_SET_BLOCKLEN    16
#define _SD_CMD_READ_SINGLE     17
#define _SD_CMD_READ_MULTI      18
#define _SD_CMD_WRITE_SINGLE    24
#define _SD_CMD_WRITE_MULTI     25

#define _SD_OK_FLAG                 0
#define _SD_ERR_FLAG                1
#define _SD_READ_FLAG               0
#define _SD_WRITE_FLAG              1
#define _SD_BLOCK_SIZE              512
#define _SD_BLOCK_SINGLE_FLAG       0
#define _SD_BLOCK_MULTI_FLAG        1
#define _SD_BLOCK_SINGLE_TOKEN      0xFE
#define _SD_BLOCK_MULTI_TOKEN       0xFC

struct {
    unsigned unused : 2;
    unsigned readOrWrite : 1;
    unsigned singleOrMultiBlock : 1;
    unsigned cardBlockSizeOK : 1;
    unsigned cardResetOK : 1;
    unsigned cardInitOK : 1;
    unsigned isCardActive : 1;
} SD_FLAGS;

struct {
    //byte 15
    unsigned end : 1;
    unsigned crc : 7;
    //byte 13-14
    uint16_t date;
    //byte 9-12
    uint32_t serial;
    //byte 8
    unsigned fwrev : 4;
    unsigned hwrev : 4;
    //byte 3-7
    unsigned char nameInverse[5];
    //byte 1-2
    uint16_t oemid;
    //byte 0
    uint8_t manfid;
} SD_CID;

typedef struct {
    //byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
    //byte 14
    unsigned reserved5: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    //byte 13
    unsigned reserved4 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    //byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved3 : 2;
    unsigned wp_grp_enable : 1;
    //byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    //byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned c_size_mult_low : 1;
    //byte 9
    unsigned c_size_mult_high : 2;
    unsigned vdd_w_cur_max : 3;
    unsigned vdd_w_curr_min : 3;
    //byte 8
    unsigned vdd_r_curr_max : 3;
    unsigned vdd_r_curr_min : 3;
    unsigned c_size_low :2;
    //byte 7
    uint8_t c_size_mid;
    //byte 6
    unsigned c_size_high : 2;
    unsigned reserved2 : 2;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    //byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    //byte 4
    uint8_t ccc_high;
    //byte 3
    uint8_t tran_speed;
    //byte 2
    uint8_t nsac;
    //byte 1
    uint8_t taac;
    //byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
} _SD_CSDv1;

typedef struct {
    //byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
    //byte 14
    unsigned reserved7: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    //byte 13
    unsigned reserved6 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    //byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved5 : 2;
    unsigned wp_grp_enable : 1;
    //byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    //byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned reserved4 : 1;
    //byte 9
    uint8_t c_size_low;
    //byte 8
    uint8_t c_size_mid;
    //byte 7
    unsigned reserved3 : 2;
    unsigned c_size_high : 6;
    //byte 6
    unsigned reserved2 : 4;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    //byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    //byte 4
    uint8_t ccc_high;
    //byte 3
    uint8_t tran_speed;
    //byte 2
    uint8_t nsac;
    //byte 1
    uint8_t taac;
    //byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
} _SD_CSDv2;

union {
    _SD_CSDv1 v1;
    _SD_CSDv2 v2;
} SD_CSD;

void SD_SPI_Init(void);
void SD_SPI_Clock(uint8_t count);
uint8_t SD_SPI_Write(uint8_t byte);
uint8_t SD_SPI_Read(void);

void SD_Card_Enable(void);
void SD_Card_Disable(void);
uint8_t SD_Card_Command(uint8_t cmd, uint32_t arg);
uint8_t SD_Card_Crc7(uint8_t crc, uint8_t *data, uint8_t len);
uint16_t SD_Card_Crc16(uint16_t crc, uint8_t *data, uint16_t len);
uint16_t SD_Card_Crc16Byte(uint16_t crc, uint8_t c);
void SD_Card_Init(void);
void SD_Card_ReadReg16(uint8_t reg, uint8_t *dst);
uint32_t SD_Card_GetSize(void);
uint16_t SD_Card_ProcessCRC(void);
uint8_t SD_Card_IsActive(void);
void SD_Card_WaitIfBusy(void);
void SD_Card_WaitStartToken(void);

uint8_t SD_Card_RWInit(uint32_t addr, uint8_t readOrWrite, uint8_t singleOrMultiBlock);
uint16_t SD_Card_RWEnd(void);
uint8_t SD_Card_ReadBlock(uint32_t addr, uint8_t *dst);
uint8_t SD_Card_WriteBlock(uint32_t addr, uint8_t *src);
void SD_Card_RWStartMulti(void);
uint16_t SD_Card_RWStopMulti(void);

#endif