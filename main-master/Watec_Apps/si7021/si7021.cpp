// Copyright (c) Konstantin Belyalov. All rights reserved.
// Licensed under the MIT license.

#include "hw.h"


#define SI7021_ADDRESS_READ          (0x40 << 1) | 0x01
#define SI7021_ADDRESS_WRITE         (0x40 << 1)

#define SI7021_MEASURE_HOLD           0xE5 // Measure Relative Humidity, Hold Master Mode
#define SI7021_MEASURE_NOHOLD         0xF5 // Measure Relative Humidity, No Hold Master Mode
#define SI7021_MEASURE_TEMP_HOLD      0xE3 // Measure Temperature, Hold Master Mode
#define SI7021_MEASURE_TEMP_NOHOLD    0xF3 // Measure Temperature, No Hold Master Mode
#define SI7021_READ_PREV_TEMP         0xE0 // Read Temperature Value from Previous RH Measurement
#define SI7021_RESET                  0xFE
#define SI7021_WRITE_USER_REG1        0xE6 // Write RH/T User Register 1
#define SI7021_READ_USER_REG1         0xE7 // Read RH/T User Register 1
#define SI7021_WRITE_HEATER_REG       0x51 // Write Heater Control Register
#define SI7021_READ_HEATER_REG        0x11 // Read Heater Control Register
#define SI7021_READ_ID1               0xFA0F // Read Electronic ID 1st Byte
#define SI7021_READ_ID2               0xFCC9 // Read Electronic ID 2nd Byte
#define SI7021_FIRMWARE_VERSION       0x84B8 // Read Firmware Revision

#ifdef __cplusplus
extern "C" {
#endif


si7021_t::si7021_t(I2C_HandleTypeDef *_hi2cx){
	hi2cx = &*_hi2cx ;
}



// Helper to read 2 bytes of device ID
 uint32_t si7021_t::_readID(uint32_t reg){
	uint32_t id = 0;
	uint8_t si7021_buf[4];

	si7021_buf[0] = reg >> 8;
	si7021_buf[1] = reg & 0xff;

	int res = HAL_I2C_Master_Transmit(hi2cx, SI7021_ADDRESS_WRITE, &si7021_buf[0], 2, 100);
	if (res != HAL_OK) {
		goto out;
	}

	res = HAL_I2C_Master_Receive(hi2cx, SI7021_ADDRESS_READ, &si7021_buf[0], 4, 100);
	if (res != HAL_OK) {
		goto out;
	}

	id = si7021_buf[0] << 24;
	id |= si7021_buf[1] << 16;
	id |= si7021_buf[2] << 8;
	id |= si7021_buf[3];

	out:
	return id;
}


// Helper to read and convert temperature into uint format
uint32_t si7021_t::read_convertTemperature(){
	uint8_t si7021_buf[4];
	int res = HAL_I2C_Master_Receive(hi2cx, SI7021_ADDRESS_READ, si7021_buf, 2, 100);

	if (res != HAL_OK) {
		return SI7021_MEASURE_FAILED;
	}

	double raw = (double)(si7021_buf[0] << 8 | si7021_buf[1]) * 175.72 / 65536 - 46.85;
	uint32_t t1 = raw;
	uint32_t t2 = (raw - t1) * 100;

	return (t1 * 100) + t2;
}


uint64_t si7021_t::readID(){
	uint32_t id1 = _readID(SI7021_READ_ID1);
	uint32_t id2 = _readID(SI7021_READ_ID2);

	if (id1 == 0 || id2 == 0) {
		return 0;
	}

	return (uint64_t)id1 << 32 | id2;
}


uint32_t si7021_t::setConfig( uint8_t heater, uint8_t resolution ){
	uint8_t si7021_buf[4];
	si7021_buf[0] = SI7021_WRITE_USER_REG1;
	si7021_buf[1] = heater | resolution;

	return HAL_I2C_Master_Transmit(hi2cx, SI7021_ADDRESS_WRITE, &si7021_buf[0], 2, 100);
}


uint32_t si7021_t::setHeaterPower( uint8_t power ){
	uint8_t si7021_buf[4];
	si7021_buf[0] = SI7021_WRITE_HEATER_REG;
	si7021_buf[1] = power;

	return HAL_I2C_Master_Transmit(hi2cx, SI7021_ADDRESS_WRITE, si7021_buf, 2, 100);
}


uint32_t si7021_t::measureHumidity()
{
  uint8_t si7021_buf[4];
  si7021_buf[0] = SI7021_MEASURE_NOHOLD;

  // Start measure
  int res = HAL_I2C_Master_Transmit(hi2cx, SI7021_ADDRESS_WRITE, si7021_buf, 1, 100);
  if (res != HAL_OK) {
    return SI7021_MEASURE_FAILED;
  }
  HAL_Delay(30);

  // Read result
  res = HAL_I2C_Master_Receive(hi2cx, SI7021_ADDRESS_READ, si7021_buf, 2, 100);
  if (res != HAL_OK) {
    return SI7021_MEASURE_FAILED;
  }

  return (si7021_buf[0] << 8 | si7021_buf[1]) * 125 / 65536 - 6;
}


int32_t si7021_t::measureTemperature(){
  uint8_t si7021_buf[4];
  si7021_buf[0] = SI7021_MEASURE_TEMP_NOHOLD;

  int res = HAL_I2C_Master_Transmit(hi2cx, SI7021_ADDRESS_WRITE, &si7021_buf[0], 1, 100);
  if (res != HAL_OK) {
    return SI7021_MEASURE_FAILED;
  }
  HAL_Delay(30);

  return read_convertTemperature();
}


int32_t si7021_t::readPreviousTemperature(){
  uint8_t si7021_buf[4];
  si7021_buf[0] = SI7021_READ_PREV_TEMP;

  int res = HAL_I2C_Master_Transmit(hi2cx, SI7021_ADDRESS_WRITE, &si7021_buf[0], 1, 100);
  if (res != HAL_OK) {
    return SI7021_MEASURE_FAILED;
  }

  return read_convertTemperature();
}



void si7021_t::init(){
	MX_I2C1_Init();

	setConfig(SI7021_HEATER_ON, SI7021_RESOLUTION_RH12_TEMP14) ;
	setHeaterPower(SI7021_HEATER_POWER_3MA) ;

}


#ifdef __cplusplus
}
#endif
