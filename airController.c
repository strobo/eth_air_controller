#include "airController.h"
#include <avr/pgmspace.h>

void init_airController(DATA_DAIKIN *daikin)
{
	daikin->buf[0]=0x11;daikin->buf[1]=0xDA;daikin->buf[2]=0x27;daikin->buf[3]=0x00;daikin->buf[4]=0xC5;daikin->buf[5]=0x00;daikin->buf[6]=0x00;daikin->buf[7]=0xD7;
	daikin->buf[8]=0x11;daikin->buf[9]=0xDA;daikin->buf[10]=0x27;daikin->buf[11]=0x00;daikin->buf[12]=0x42;daikin->buf[13]=0x00;daikin->buf[14]=0x00;daikin->buf[15]=0x54;
	daikin->buf[16]=0x11;daikin->buf[17]=0xDA;daikin->buf[18]=0x27;daikin->buf[19]=0x00;daikin->buf[20]=0x00;daikin->buf[21]=0x39;daikin->buf[22]=0x28;daikin->buf[23]=0x00;
	daikin->buf[24]=0xA0;daikin->buf[25]=0x00;daikin->buf[26]=0x00;daikin->buf[27]=0x06;daikin->buf[28]=0x60;daikin->buf[29]=0x00;daikin->buf[30]=0x00;daikin->buf[31]=0xC1;
	daikin->buf[32]=0x00;daikin->buf[33]=0x00;daikin->buf[34]=0x3A;

/*	if(state == true){
		buf[21] |= 0x01;
	} else {
		buf[21] ^= 0x01;
	}
*/
	/* temp code here */

	daikin->buf[34] = airController_checksum(daikin);

}

void airController_on(DATA_DAIKIN *daikin){
	//state = ON;
	daikin->buf[21] = 0x39;
	daikin->buf[34] = airController_checksum(daikin);
}

void airController_off(DATA_DAIKIN *daikin){
	//state = OFF;
	daikin->buf[21] = 0x38;
	daikin->buf[34] = airController_checksum(daikin);
}

void airController_setTemp(DATA_DAIKIN *daikin, uint8_t temp)
{
	daikin->buf[22] = (temp-18)*2 + 0x24;
	daikin->buf[34] = airController_checksum(daikin);
}

uint8_t airConroller_getTemp(DATA_DAIKIN *daikin)
{
	return (daikin->buf[22]-0x24)/2 + 18;
}
//unsigned char* AirConditioner::getControllData(void)
//{
/*if(state == ON){
  daikin->buf[21] |= 0x01;
  } else {
  daikin->buf[21] ^= 0x01;
  }*/
//	daikin->buf[34] = checksum();
//	return daikin->buf;
//}

uint8_t airController_checksum(DATA_DAIKIN *daikin)
{
	uint8_t sum = 0;
	uint8_t i;

	for(i = 16; i <= 33; i++){
		sum += daikin->buf[i];
	}
	return sum;
}

