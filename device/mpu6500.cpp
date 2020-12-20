#include "Arduino.h"
#include "Wire.h"
#include "mpu6500.h"

void WriteByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  Wire.beginTransmission(address);  // Initialize the Tx buffer
  Wire.write(subAddress);           // Put slave register address in Tx buffer
  Wire.write(data);                 // Put data in Tx buffer
  Wire.endTransmission();           // Send the Tx buffer
}

uint8_t ReadByte(uint8_t address, uint8_t subAddress)
{
  uint8_t data; // `data` will store the register data   
  Wire.beginTransmission(address);         // Initialize the Tx buffer
  Wire.write(subAddress);                  // Put slave register address in Tx buffer
  Wire.endTransmission(false);             // Send the Tx buffer, but send a restart to keep connection alive
  Wire.requestFrom(address, (uint8_t) 1);  // Read one byte from slave register address 
  data = Wire.read();                      // Fill Rx buffer with result
  return data;                             // Return data read from slave register
}

void ReadBytes(uint8_t address, uint8_t subAddress, uint8_t count,
                        uint8_t * dest)
{  
  Wire.beginTransmission(address);   // Initialize the Tx buffer
  Wire.write(subAddress);            // Put slave register address in Tx buffer
  Wire.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
  uint8_t i = 0;
  Wire.requestFrom(address, count);  // Read bytes from slave register address 
  while (Wire.available()) {
    dest[i++] = Wire.read(); }         // Put read results in the Rx buffer
}


void Mpu6500Config() {
  uint8_t fifo_mode = 0; /* FIFO_MODE: 1 = stop on full, 0 = overwrite */
  WriteByte(MPU_IIC_ADDR, MPU_PWR_MGMT_1, MPU_PWR_MGMT_1_DEVICE_RESET);
  delay(100);

  WriteByte(MPU_IIC_ADDR, MPU_PWR_MGMT_1, 0
      // | MPU_PWR_MGMT_1_GYRO_STANDBY // disable gyro
      | MPU_PWR_MGMT_1_TEMP_DIS // disable temp sensor
      | MPU_PWR_MGMT_1_CLKSEL_INTERNAL // use internal osc (pll unavailable if gyro is on standby)
  );
  WriteByte(MPU_IIC_ADDR, MPU_PWR_MGMT_2, 0
      // disable gyro
      | MPU_PWR_MGMT_2_DISABLE_XG
      | MPU_PWR_MGMT_2_DISABLE_YG
      | MPU_PWR_MGMT_2_DISABLE_ZG
  );
  WriteByte(MPU_IIC_ADDR, MPU_CONFIG, 0
      | (fifo_mode ? MPU_CONFIG_FIFO_MODE : 0)
      | MPU_CONFIG_EXT_SYNC_SET_NONE
      | MPU_CONFIG_DLPF_CFG_0
  );
  WriteByte(MPU_IIC_ADDR, MPU_ACCEL_CONFIG, 0
      | MPU_ACCEL_CONFIG_ACCEL_FS_SEL_8_G
  );
  WriteByte(MPU_IIC_ADDR, MPU_ACCEL_CONFIG_2, 0
      | MPU_ACCEL_CONFIG_2_FIFO_SIZE_4096_B
      | MPU_ACCEL_CONFIG_2_ACCEL_FCHOICE_0
      | MPU_ACCEL_CONFIG_2_A_DLPF_CFG_0
  );
  WriteByte(MPU_IIC_ADDR, MPU_SMPLRT_DIV, 0); // divide sample frequency by (SMPLRT_DIV+1)
  WriteByte(MPU_IIC_ADDR, MPU_FIFO_EN, 0
      // | MPU_FIFO_EN_GYROX
      // | MPU_FIFO_EN_GYROY
      // | MPU_FIFO_EN_GYROZ
      | MPU_FIFO_EN_ACCEL
  );
  WriteByte(MPU_IIC_ADDR, MPU_I2C_MST_CTRL, 0
  );
  WriteByte(MPU_IIC_ADDR, MPU_INT_ENABLE, 0
  );
  WriteByte(MPU_IIC_ADDR, MPU_USER_CTRL, 0
      // | MPU_USER_CTRL_DMP_EN
      | MPU_USER_CTRL_FIFO_EN
      // | MPU_USER_CTRL_I2C_MST_EN
      | MPU_USER_CTRL_FIFO_RST
  );
  delay(100);
}

uint8_t Mpu6500GetData(uint8_t* buffer, uint16_t size) {
  //size usually 24000
  uint8_t max_iic_size = 252;/* maximum size of one I2C read */
  uint16_t fifo_avail_size = 0;
  uint16_t buffer_pos = 0;
  uint8_t fifo_avail_size_temp[2];
	uint8_t* p_fifo_avail_size = (uint8_t*)&fifo_avail_size;
  while (size > buffer_pos)
  {
    ReadBytes(MPU_IIC_ADDR, MPU_FIFO_COUNTH, 2, fifo_avail_size_temp);
	  p_fifo_avail_size[0] = fifo_avail_size_temp[1];
	  p_fifo_avail_size[1] = fifo_avail_size_temp[0];
    fifo_avail_size = (fifo_avail_size / 6) * 6;
    if(fifo_avail_size > 3500) {
      /* Probable FIFO overflow */
      WriteByte(MPU_IIC_ADDR, MPU_USER_CTRL, MPU_USER_CTRL_FIFO_EN | MPU_USER_CTRL_FIFO_RST);
      delay(10);
      continue;
    }
    while(fifo_avail_size) {
      uint16_t current_readable = min(min(max_iic_size, fifo_avail_size), size - buffer_pos);
      if(current_readable == 0) {
        break;
      }
      ReadBytes(MPU_IIC_ADDR, MPU_FIFO_R_W, current_readable, buffer + buffer_pos);
      buffer_pos += current_readable;
      fifo_avail_size -= current_readable;
    }
  }
  return 0;
}
