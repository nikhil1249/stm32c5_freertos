#ifndef TMC6460_H
#define TMC6460_H

#include <stdint.h>


#define TMC6460_CHIP_ID_REG          0x000U

#define TMC6460_EXPECTED_CHIP_ID     0x36343630UL


typedef enum
{
    TMC6460_OK = 0,
    TMC6460_ERROR,
    TMC6460_BUSY,
    TMC6460_TIMEOUT,
    TMC6460_INVALID_PARAM,
    TMC6460_INVALID_RESPONSE,
    TMC6460_INVALID_CHIP_ID

} tmc6460_status_t;


/* Generic register functions */

tmc6460_status_t TMC6460_ReadRegister(uint16_t address,
                                      uint32_t *value);

tmc6460_status_t TMC6460_WriteRegister(uint16_t address,
                                       uint32_t value);


/* Register field functions */

tmc6460_status_t TMC6460_ReadField(uint16_t address,
                                   uint32_t mask,
                                   uint8_t shift,
                                   uint32_t *value);

tmc6460_status_t TMC6460_WriteField(uint16_t address,
                                    uint32_t mask,
                                    uint8_t shift,
                                    uint32_t value);


/* Device APIs */

tmc6460_status_t TMC6460_ReadChipID(uint32_t *chip_id);

tmc6460_status_t TMC6460_CheckCommunication(void);

void TMC_SendData(void);

#endif