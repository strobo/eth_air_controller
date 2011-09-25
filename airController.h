#ifndef _AIR_CONDITINOR
#define _AIR_CONDITINOR

#include <avr/pgmspace.h>

typedef struct data_daikin_t{
	uint8_t buf[35];
} DATA_DAIKIN;



void init_airController(DATA_DAIKIN  *daikin);
void airController_on(DATA_DAIKIN *daikin);
void airController_off(DATA_DAIKIN *daikin);
void airController_setTemp(DATA_DAIKIN *daikin, uint8_t temp);
uint8_t airController_getTemp(DATA_DAIKIN *daikin);
uint8_t airController_checksum(DATA_DAIKIN *daikin);
//uint8_t* getControllData();
#endif
