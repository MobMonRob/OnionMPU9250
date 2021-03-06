#include "MPU9250_header.h"

 //void writeByte(uint8_t address, uint8_t subAddress, uint8_t data){
 //       char data_write[2];
 //       data_write[0] = subAddress;
 //       data_write[1] = data;
 //       i2c_writeBytes(0, address, data_write[0], data_write[1], 4);
 //   }
 
// int readByte(uint8_t address, uint8_t subAddress){
//        int data[1]; // `data` will store the register data     
//        char data_write[1];
//        data_write[0] = subAddress;
//        i2c_writeBytes(0, address, data_write[0], data_write[1], 4);
//        i2c_readByte(0, address, subAddress, data); 
//        return data; 
//    }
 
 //void readBytes(uint8_t address, uint8_t subAddress, 
 //                  uint8_t count, uint8_t * dest){     
 //       char data[14];
 //      char data_write[1];
 //       data_write[0] = subAddress;
 //       i2c_writeBytes(0, address, data_write[0], data_write[1], 4);
 //       i2c_read(0, address, subAddress, data, count); 
 //       for(int ii = 0; ii < count; ii++) {
 //        dest[ii] = data[ii];
 //       }
 //} 
 
 void getMres() {
      switch (Mscale){
        // Possible magnetometer scales (and their register bit settings) are:
        // 14 bit resolution (0) and 16 bit resolution (1)
        case MFS_14BITS:
              mRes = 10.0*4219.0/8190.0; // Proper scale to return milliGauss
              break;
        case MFS_16BITS:
              mRes = 10.0*4219.0/32760.0; // Proper scale to return milliGauss
              break;
      }
 }
 
 void getGres() {
      switch (Gscale){
        // Possible gyro scales (and their register bit settings) are:
        // 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11). 
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on 
        // that 2-bit value:
        case GFS_250DPS:
              gRes = 250.0/32768.0;
              break;
        case GFS_500DPS:
              gRes = 500.0/32768.0;
              break;
        case GFS_1000DPS:
              gRes = 1000.0/32768.0;
              break;
        case GFS_2000DPS:
              gRes = 2000.0/32768.0;
              break;
      }
  }

  void getAres() {
      switch (Ascale){
        // Possible accelerometer scales (and their register bit settings) are:
        // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11). 
            // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
        case AFS_2G:
              aRes = 2.0/32768.0;
              break;
        case AFS_4G:
              aRes = 4.0/32768.0;
              break;
        case AFS_8G:
              aRes = 8.0/32768.0;
              break;
        case AFS_16G:
              aRes = 16.0/32768.0;
              break;
      }
 }
  
 void readAccelData(int16_t * destination){
      uint8_t rawData[6];  // x/y/z accel register data stored here
      i2c_read(0, MPU9250_ADDRESS, ACCEL_XOUT_H,  &rawData[0],6);  // Read the six raw data registers into data array
      destination[0] = (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
      destination[1] = (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;  
      destination[2] = (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ; 
 }
 
 void readGyroData(int16_t * destination){
      uint8_t rawData[6];  // x/y/z gyro register data stored here
      i2c_read(0, MPU9250_ADDRESS, GYRO_XOUT_H,  &rawData[0],6);  // Read the six raw data registers sequentially into data array
      destination[0] = (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
      destination[1] = (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;  
      destination[2] = (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ; 
 }
 
 void readMagData(int16_t * destination){
      int data;
      uint8_t rawData[7];  // x/y/z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition
      i2c_readByte(0, AK8963_ADDRESS, AK8963_ST1,&data);
      if(data & 0x01) { // wait for magnetometer data ready bit to be set
      i2c_read(0, AK8963_ADDRESS, AK8963_XOUT_L,  &rawData[0],7);  // Read the six raw data and ST2 registers sequentially into data array
      uint8_t c = rawData[6]; // End data read by reading ST2 register
        if(!(c & 0x08)) { // Check if magnetic sensor overflow set, if not then report data
        destination[0] = (int16_t)(((int16_t)rawData[1] << 8) | rawData[0]);  // Turn the MSB and LSB into a signed 16-bit value
        destination[1] = (int16_t)(((int16_t)rawData[3] << 8) | rawData[2]) ;  // Data stored as little Endian
        destination[2] = (int16_t)(((int16_t)rawData[5] << 8) | rawData[4]) ; 
       }
      }
 }
 
 int16_t readTempData(){
      uint8_t rawData[2];  // x/y/z gyro register data stored here
      i2c_read(0, MPU9250_ADDRESS, TEMP_OUT_H,  &rawData[0],2);  // Read the two raw data registers sequentially into data array 
      return (int16_t)(((int16_t)rawData[0]) << 8 | rawData[1]) ;  // Turn the MSB and LSB into a 16-bit value
 }

 void resetMPU9250() {
      // reset device
      i2c_writeBytes(0, MPU9250_ADDRESS, PWR_MGMT_1, 0x80, 1); // Write a one to bit 7 reset bit; toggle reset device
      usleep(100000);
 }
 
 void initAK8963(float * destination){
      // First extract the factory calibration for each magnetometer axis
      uint8_t rawData[3];  // x/y/z gyro calibration data stored here
      i2c_writeBytes(0, AK8963_ADDRESS, AK8963_CNTL, 0x00, 1); // Power down magnetometer  
      usleep(100000);
      i2c_writeBytes(0, AK8963_ADDRESS, AK8963_CNTL, 0x0F, 1); // Enter Fuse ROM access mode
      usleep(100000);
      i2c_read(0, AK8963_ADDRESS, AK8963_ASAX,  &rawData[0],3);  // Read the x-, y-, and z-axis calibration values
      destination[0] =  (float)(rawData[0] - 128)/256.0f + 1.0f;   // Return x-axis sensitivity adjustment values, etc.
      destination[1] =  (float)(rawData[1] - 128)/256.0f + 1.0f;  
      destination[2] =  (float)(rawData[2] - 128)/256.0f + 1.0f; 
      i2c_writeBytes(0, AK8963_ADDRESS, AK8963_CNTL, 0x00, 1); // Power down magnetometer  
      usleep(100000);
      // Configure the magnetometer for continuous read and highest resolution
      // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
      // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
      i2c_writeBytes(0, AK8963_ADDRESS, AK8963_CNTL, Mscale << 4 | Mmode, 4); // Set magnetometer data resolution and sample ODR
      usleep(100000);
 }
 
 // Initialize MPU9250 device
 void initMPU9250(){  
      
      // wake up device
      i2c_writeBytes(0, MPU9250_ADDRESS, PWR_MGMT_1, 0x00,1); // Clear sleep mode bit (6), enable all sensors 
      usleep(100000); // sleep 100 ms for PLL to get established on x-axis gyro; should check for PLL ready interrupt  

      // get stable time source
      // 001 = Auto selects the best available clock source – PLL if ready, else use the Internal oscillator
      // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
      i2c_writeBytes(0, MPU9250_ADDRESS, PWR_MGMT_1, 0x01,1);  

      // Configure Gyro and Accelerometer
      // Disable FSYNC and set accelerometer and gyro bandwidth to 44 and 42 Hz, respectively; 
      // DLPF_CFG = bits 2:0 = 010; this sets the sample rate at 1 kHz for both
      // Maximum delay is 4.9 ms which is just over a 200 Hz maximum rate
      i2c_writeBytes(0, MPU9250_ADDRESS, CONFIG, 0x03,1);  

      // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
      i2c_writeBytes(0,MPU9250_ADDRESS, SMPLRT_DIV, 0x04,1);  // Use a 200 Hz rate; the same rate set in CONFIG above

      // Set gyroscope full scale range
      // Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
      uint8_t c;
      i2c_readByte(0,MPU9250_ADDRESS, GYRO_CONFIG, &c);
      i2c_writeBytes(0, MPU9250_ADDRESS, GYRO_CONFIG, c & ~0xE0,4); // Clear self-test bits [7:5] 
      i2c_writeBytes(0, MPU9250_ADDRESS, GYRO_CONFIG, c & ~0x18,4 ); // Clear AFS bits [4:3]
      i2c_writeBytes(0, MPU9250_ADDRESS, GYRO_CONFIG, c | Gscale << 3,4); // Set full scale range for the gyro

      // Set accelerometer configuration
      i2c_readByte(0,MPU9250_ADDRESS, ACCEL_CONFIG,&c);
      i2c_writeBytes(0, MPU9250_ADDRESS, ACCEL_CONFIG, c & ~0xE0,1); // Clear self-test bits [7:5] 
      i2c_writeBytes(0, MPU9250_ADDRESS, ACCEL_CONFIG, c & ~0x18,1); // Clear AFS bits [4:3]
      i2c_writeBytes(0, MPU9250_ADDRESS, ACCEL_CONFIG, c | Ascale << 3,4); // Set full scale range for the accelerometer 

      // Set accelerometer sample rate configuration
      // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
      // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
      i2c_readByte(0,MPU9250_ADDRESS, ACCEL_CONFIG2,&c);
      i2c_writeBytes(0, MPU9250_ADDRESS, ACCEL_CONFIG2, c & ~0x0F,1); // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])  
      i2c_writeBytes(0, MPU9250_ADDRESS, ACCEL_CONFIG2, c | 0x03,1); // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz

      // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates, 
      // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

      // Configure Interrupts and Bypass Enable
      // Set interrupt pin active high, push-pull, and clear on read of INT_STATUS, 
      // enable I2C_BYPASS_EN so additional chips 
      // can join the I2C bus and all can be controlled by the Arduino as master
      i2c_writeBytes(0,MPU9250_ADDRESS, INT_PIN_CFG, 0x22,1);    
      i2c_writeBytes(0,MPU9250_ADDRESS, INT_ENABLE, 0x01,1);  // Enable data ready (bit 0) interrupt
 }
 
    // Function which accumulates gyro and accelerometer data after device 
    // initialization. It calculates the average
    // of the at-rest readings and then loads the resulting offsets into 
    // accelerometer and gyro bias registers.
    void calibrateMPU9250(float * dest1, float * dest2){  
        uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
        uint16_t ii, packet_count, fifo_count;
        int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

        // reset device, reset all registers, clear gyro and accelerometer bias registers
        i2c_writeBytes(0, MPU9250_ADDRESS, PWR_MGMT_1, 0x80,1 ); // Write a one to bit 7 reset bit; toggle reset device
        usleep(100000);

        // get stable time source
        // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
        i2c_writeBytes(0, MPU9250_ADDRESS, PWR_MGMT_1, 0x01,1);  
        i2c_writeBytes(0,MPU9250_ADDRESS, PWR_MGMT_2, 0x00,1); 
        usleep(200000);
  
        // Configure device for bias calculation
        i2c_writeBytes(0,MPU9250_ADDRESS, INT_ENABLE, 0x00,1);   // Disable all interrupts
        i2c_writeBytes(0,MPU9250_ADDRESS, FIFO_EN, 0x00,1);      // Disable FIFO
        i2c_writeBytes(0,MPU9250_ADDRESS, PWR_MGMT_1, 0x00,1);   // Turn on internal clock source
        i2c_writeBytes(0,MPU9250_ADDRESS, I2C_MST_CTRL, 0x00,1); // Disable I2C master
        i2c_writeBytes(0,MPU9250_ADDRESS, USER_CTRL, 0x00,1);    // Disable FIFO and I2C master modes
        i2c_writeBytes(0,MPU9250_ADDRESS, USER_CTRL, 0x0C,1);    // Reset FIFO and DMP
        usleep(15000);

        // Configure MPU9250 gyro and accelerometer for bias calculation
        i2c_writeBytes(0,MPU9250_ADDRESS, CONFIG, 0x01,1);      // Set low-pass filter to 188 Hz
        i2c_writeBytes(0,MPU9250_ADDRESS, SMPLRT_DIV, 0x00,1);  // Set sample rate to 1 kHz
        i2c_writeBytes(0,MPU9250_ADDRESS, GYRO_CONFIG, 0x00,1);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
        i2c_writeBytes(0,MPU9250_ADDRESS, ACCEL_CONFIG, 0x00,1); // Set accelerometer full-scale to 2 g, maximum sensitivity

        uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
        uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

        // Configure FIFO to capture accelerometer and gyro data for bias calculation
        i2c_writeBytes(0,MPU9250_ADDRESS, USER_CTRL, 0x40,1);   // Enable FIFO  
        i2c_writeBytes(0,MPU9250_ADDRESS, FIFO_EN, 0x78,1);     // Enable gyro and accelerometer sensors for FIFO (max size 512 bytes in MPU-9250)
        usleep(40000); // accumulate 40 samples in 80 milliseconds = 480 bytes

        // At end of sample accumulation, turn off FIFO sensor read
        i2c_writeBytes(0, MPU9250_ADDRESS, FIFO_EN, 0x00,1);        // Disable gyro and accelerometer sensors for FIFO
        i2c_read(0, MPU9250_ADDRESS, FIFO_COUNTH,  &data[0],2); // read FIFO sample count
        fifo_count = ((uint16_t)data[0] << 8) | data[1];
        packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging

        for (ii = 0; ii < packet_count; ii++) {
          int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
          i2c_read(0, MPU9250_ADDRESS, FIFO_R_W,  &data[0],12); // read data for averaging
          accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
          accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
          accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;    
          gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
          gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
          gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;

          accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
          accel_bias[1] += (int32_t) accel_temp[1];
          accel_bias[2] += (int32_t) accel_temp[2];
          gyro_bias[0]  += (int32_t) gyro_temp[0];
          gyro_bias[1]  += (int32_t) gyro_temp[1];
          gyro_bias[2]  += (int32_t) gyro_temp[2];

        }
        accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
        accel_bias[1] /= (int32_t) packet_count;
        accel_bias[2] /= (int32_t) packet_count;
        gyro_bias[0]  /= (int32_t) packet_count;
        gyro_bias[1]  /= (int32_t) packet_count;
        gyro_bias[2]  /= (int32_t) packet_count;
    
        if(accel_bias[2] > 0L) {
            accel_bias[2] -= (int32_t) accelsensitivity;
        // Remove gravity from the z-axis accelerometer bias calculation
        } else {
            accel_bias[2] += (int32_t) accelsensitivity;
        }
 
        // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
        data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
        data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
        data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
        data[3] = (-gyro_bias[1]/4)       & 0xFF;
        data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
        data[5] = (-gyro_bias[2]/4)       & 0xFF;

        // Push gyro biases to hardware registers
        /*  writeByte(MPU9250_ADDRESS, XG_OFFSET_H, data[0]);
          writeByte(MPU9250_ADDRESS, XG_OFFSET_L, data[1]);
          writeByte(MPU9250_ADDRESS, YG_OFFSET_H, data[2]);
          writeByte(MPU9250_ADDRESS, YG_OFFSET_L, data[3]);
          writeByte(MPU9250_ADDRESS, ZG_OFFSET_H, data[4]);
          writeByte(MPU9250_ADDRESS, ZG_OFFSET_L, data[5]);
        */
        dest1[0] = (float) gyro_bias[0]/(float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
        dest1[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
        dest1[2] = (float) gyro_bias[2]/(float) gyrosensitivity;

        // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
        // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
        // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
        // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
        // the accelerometer biases calculated above must be divided by 8.

        int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
        i2c_read(0,MPU9250_ADDRESS, XA_OFFSET_H,  &data[0],2); // Read factory accelerometer trim values
        accel_bias_reg[0] = (int16_t) ((int16_t)data[0] << 8) | data[1];
        i2c_read(0,MPU9250_ADDRESS, YA_OFFSET_H,  &data[0],2);
        accel_bias_reg[1] = (int16_t) ((int16_t)data[0] << 8) | data[1];
        i2c_read(0,MPU9250_ADDRESS, ZA_OFFSET_H,  &data[0],2);
        accel_bias_reg[2] = (int16_t) ((int16_t)data[0] << 8) | data[1];

        uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
        uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

        for(ii = 0; ii < 3; ii++) {
          if(accel_bias_reg[ii] & mask) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
        }

        // Construct total accelerometer bias, including calculated average accelerometer bias from above
        accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
        accel_bias_reg[1] -= (accel_bias[1]/8);
        accel_bias_reg[2] -= (accel_bias[2]/8);

        data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
        data[1] = (accel_bias_reg[0])      & 0xFF;
        data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
        data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
        data[3] = (accel_bias_reg[1])      & 0xFF;
        data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
        data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
        data[5] = (accel_bias_reg[2])      & 0xFF;
        data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

        // Apparently this is not working for the acceleration biases in the MPU-9250
        // Are we handling the temperature correction bit properly?
        // Push accelerometer biases to hardware registers
        /*  writeByte(MPU9250_ADDRESS, XA_OFFSET_H, data[0]);
          writeByte(MPU9250_ADDRESS, XA_OFFSET_L, data[1]);
          writeByte(MPU9250_ADDRESS, YA_OFFSET_H, data[2]);
          writeByte(MPU9250_ADDRESS, YA_OFFSET_L, data[3]);
          writeByte(MPU9250_ADDRESS, ZA_OFFSET_H, data[4]);
          writeByte(MPU9250_ADDRESS, ZA_OFFSET_L, data[5]);
        */
        // Output scaled accelerometer biases for manual subtraction in the main program
        dest2[0] = (float)accel_bias[0]/(float)accelsensitivity; 
        dest2[1] = (float)accel_bias[1]/(float)accelsensitivity;
        dest2[2] = (float)accel_bias[2]/(float)accelsensitivity;
    }

    
    void MPU9250SelfTest(float * destination){ // Should return percent deviation from factory trim values, +/- 14 or less deviation is a pass
	  uint8_t rawData[6] = {0, 0, 0, 0, 0, 0};
	  uint8_t selfTest[6];
	  int16_t gAvg[3], aAvg[3], aSTAvg[3], gSTAvg[3];
	  float factoryTrim[6];
	  uint8_t FS = 0;
   
	  i2c_writeBytes(0,MPU9250_ADDRESS, SMPLRT_DIV, 0x00,1);    // Set gyro sample rate to 1 kHz
	  i2c_writeBytes(0,MPU9250_ADDRESS, CONFIG, 0x02,1);        // Set gyro sample rate to 1 kHz and DLPF to 92 Hz
	  i2c_writeBytes(0,MPU9250_ADDRESS, GYRO_CONFIG, 1<<FS,1);  // Set full scale range for the gyro to 250 dps
	  i2c_writeBytes(0,MPU9250_ADDRESS, ACCEL_CONFIG2, 0x02,1); // Set accelerometer rate to 1 kHz and bandwidth to 92 Hz
	  i2c_writeBytes(0,MPU9250_ADDRESS, ACCEL_CONFIG, 1<<FS,1); // Set full scale range for the accelerometer to 2 g

	  for( int ii = 0; ii < 200; ii++) { // get average current values of gyro and acclerometer

	    // Read the six raw data registers into data array
	    i2c_read(0,MPU9250_ADDRESS, ACCEL_XOUT_H,  &rawData[0],6); 
	    aAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ; // Turn the MSB and LSB into a signed 16-bit value
	    aAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;
	    aAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ;

	    // Read the six raw data registers sequentially into data array
	    i2c_read(0,MPU9250_ADDRESS, GYRO_XOUT_H,  &rawData[0],6); 
	    gAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ; // Turn the MSB and LSB into a signed 16-bit value
	    gAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;
	    gAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ;
 	  }
  
	  for (int ii =0; ii < 3; ii++) { // Get average of 200 values and store as average current readings
	    aAvg[ii] /= 200;
	    gAvg[ii] /= 200;
	  }
  
	 // Configure the accelerometer for self-test
	 i2c_writeBytes(0,MPU9250_ADDRESS, ACCEL_CONFIG, 0xE0,1); // Enable self test on all three axes and set accelerometer range to +/- 2 g
	 i2c_writeBytes(0,MPU9250_ADDRESS, GYRO_CONFIG, 0xE0,1); // Enable self test on all three axes and set gyro range to +/- 250 degrees/s
	 usleep(25000); // Delay a while to let the device stabilize

	 for( int ii = 0; ii < 200; ii++) { // get average self-test values of gyro and acclerometer

	   i2c_read(0,MPU9250_ADDRESS, ACCEL_XOUT_H,  &rawData[0],6); // Read the six raw data registers into data array
	   aSTAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ; // Turn the MSB and LSB into a signed 16-bit value
	   aSTAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;
	   aSTAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ;

	   i2c_read(0, MPU9250_ADDRESS, GYRO_XOUT_H,  &rawData[0],6); // Read the six raw data registers sequentially into data array
	   gSTAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ; // Turn the MSB and LSB into a signed 16-bit value
	   gSTAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;
	   gSTAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ;
	 }

	 for (int ii =0; ii < 3; ii++) { // Get average of 200 values and store as average self-test readings
	    aSTAvg[ii] /= 200;
	    gSTAvg[ii] /= 200;
	 }
  
 // Configure the gyro and accelerometer for normal operation
   i2c_writeBytes(0,MPU9250_ADDRESS, ACCEL_CONFIG, 0x00,1);
   i2c_writeBytes(0,MPU9250_ADDRESS, GYRO_CONFIG, 0x00,1);
   usleep(25000); // Delay a while to let the device stabilize
   
   // Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
   selfTest[0] = i2c_readByte(0, MPU9250_ADDRESS, SELF_TEST_X_ACCEL, &selfTest[0]); // X-axis accel self-test results
   selfTest[1] = i2c_readByte(0,MPU9250_ADDRESS, SELF_TEST_Y_ACCEL, &selfTest[1]); // Y-axis accel self-test results
   selfTest[2] = i2c_readByte(0,MPU9250_ADDRESS, SELF_TEST_Z_ACCEL, &selfTest[2]); // Z-axis accel self-test results
   selfTest[3] = i2c_readByte(0,MPU9250_ADDRESS, SELF_TEST_X_GYRO, &selfTest [3]); // X-axis gyro self-test results
   selfTest[4] = i2c_readByte(0,MPU9250_ADDRESS, SELF_TEST_Y_GYRO, &selfTest[4]); // Y-axis gyro self-test results
   selfTest[5] = i2c_readByte(0,MPU9250_ADDRESS, SELF_TEST_Z_GYRO, &selfTest [5]); // Z-axis gyro self-test results

  // Retrieve factory self-test value from self-test code reads
   factoryTrim[0] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[0] - 1.0) )); // FT[Xa] factory trim calculation
   factoryTrim[1] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[1] - 1.0) )); // FT[Ya] factory trim calculation
   factoryTrim[2] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[2] - 1.0) )); // FT[Za] factory trim calculation
   factoryTrim[3] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[3] - 1.0) )); // FT[Xg] factory trim calculation
   factoryTrim[4] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[4] - 1.0) )); // FT[Yg] factory trim calculation
   factoryTrim[5] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[5] - 1.0) )); // FT[Zg] factory trim calculation
 
 // Report results as a ratio of (STR - FT)/FT; the change from Factory Trim of the Self-Test Response
 // To get percent, must multiply by 100
   for (int i = 0; i < 3; i++) {
     destination[i] = 100.0*((float)(aSTAvg[i] - aAvg[i]))/factoryTrim[i]; // Report percent differences
     destination[i+3] = 100.0*((float)(gSTAvg[i] - gAvg[i]))/factoryTrim[i+3]; // Report percent differences
   }
   
}



// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration, rotation rate, and magnetic moments to produce a quaternion-based estimate of absolute
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
        void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)
        {
            float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
            float norm;
            float hx, hy, _2bx, _2bz;
            float s1, s2, s3, s4;
            float qDot1, qDot2, qDot3, qDot4;

            // Auxiliary variables to avoid repeated arithmetic
            float _2q1mx;
            float _2q1my;
            float _2q1mz;
            float _2q2mx;
            float _4bx;
            float _4bz;
            float _2q1 = 2.0f * q1;
            float _2q2 = 2.0f * q2;
            float _2q3 = 2.0f * q3;
            float _2q4 = 2.0f * q4;
            float _2q1q3 = 2.0f * q1 * q3;
            float _2q3q4 = 2.0f * q3 * q4;
            float q1q1 = q1 * q1;
            float q1q2 = q1 * q2;
            float q1q3 = q1 * q3;
            float q1q4 = q1 * q4;
            float q2q2 = q2 * q2;
            float q2q3 = q2 * q3;
            float q2q4 = q2 * q4;
            float q3q3 = q3 * q3;
            float q3q4 = q3 * q4;
            float q4q4 = q4 * q4;

            // Normalise accelerometer measurement
            norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0.0f) return; // handle NaN
            norm = 1.0f/norm;
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0.0f) return; // handle NaN
            norm = 1.0f/norm;
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            _2q1mx = 2.0f * q1 * mx;
            _2q1my = 2.0f * q1 * my;
            _2q1mz = 2.0f * q1 * mz;
            _2q2mx = 2.0f * q2 * mx;
            hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
            hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
            _2bx = sqrt(hx * hx + hy * hy);
            _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
            _4bx = 2.0f * _2bx;
            _4bz = 2.0f * _2bz;

            // Gradient decent algorithm corrective step
            s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
            s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
            s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
            s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
            norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
            norm = 1.0f/norm;
            s1 *= norm;
            s2 *= norm;
            s3 *= norm;
            s4 *= norm;
int beta = 0;

            // Compute rate of change of quaternion
            qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
            qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
            qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
            qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

            // Integrate to yield quaternion
            q1 += qDot1 * deltat;
            q2 += qDot2 * deltat;
            q3 += qDot3 * deltat;
            q4 += qDot4 * deltat;
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
            norm = 1.0f/norm;
            q[0] = q1 * norm;
            q[1] = q2 * norm;
            q[2] = q3 * norm;
            q[3] = q4 * norm;

        }
  
  
  
 // Similar to Madgwick scheme but uses proportional and integral filtering on 
 // the error between estimated reference vectors and
 // measured ones. 
 void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy, 
                             float gz, float mx, float my, float mz){
     
            float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
            float norm;
            float hx, hy, bx, bz;
            float vx, vy, vz, wx, wy, wz;
            float ex, ey, ez;
            float pa, pb, pc;

            // Auxiliary variables to avoid repeated arithmetic
            float q1q1 = q1 * q1;
            float q1q2 = q1 * q2;
            float q1q3 = q1 * q3;
            float q1q4 = q1 * q4;
            float q2q2 = q2 * q2;
            float q2q3 = q2 * q3;
            float q2q4 = q2 * q4;
            float q3q3 = q3 * q3;
            float q3q4 = q3 * q4;
            float q4q4 = q4 * q4;   

            // Normalise accelerometer measurement
            norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0.0f) return; // handle NaN
            norm = 1.0f / norm;        // use reciprocal for division
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0.0f) return; // handle NaN
            norm = 1.0f / norm;        // use reciprocal for division
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            hx = 2.0f * mx * (0.5f - q3q3 - q4q4) + 2.0f * my * (q2q3 - q1q4) + 2.0f * mz * (q2q4 + q1q3);
            hy = 2.0f * mx * (q2q3 + q1q4) + 2.0f * my * (0.5f - q2q2 - q4q4) + 2.0f * mz * (q3q4 - q1q2);
            bx = sqrt((hx * hx) + (hy * hy));
            bz = 2.0f * mx * (q2q4 - q1q3) + 2.0f * my * (q3q4 + q1q2) + 2.0f * mz * (0.5f - q2q2 - q3q3);

            // Estimated direction of gravity and magnetic field
            vx = 2.0f * (q2q4 - q1q3);
            vy = 2.0f * (q1q2 + q3q4);
            vz = q1q1 - q2q2 - q3q3 + q4q4;
            wx = 2.0f * bx * (0.5f - q3q3 - q4q4) + 2.0f * bz * (q2q4 - q1q3);
            wy = 2.0f * bx * (q2q3 - q1q4) + 2.0f * bz * (q1q2 + q3q4);
            wz = 2.0f * bx * (q1q3 + q2q4) + 2.0f * bz * (0.5f - q2q2 - q3q3);  

            // Error is cross product between estimated direction and measured direction of gravity
            ex = (ay * vz - az * vy) + (my * wz - mz * wy);
            ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
            ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
            if (Ki > 0.0f)
            {
                eInt[0] += ex;      // accumulate integral error
                eInt[1] += ey;
                eInt[2] += ez;
            }
            else
            {
                eInt[0] = 0.0f;     // prevent integral wind up
                eInt[1] = 0.0f;
                eInt[2] = 0.0f;
            }

            // Apply feedback terms
            gx = gx + Kp * ex + Ki * eInt[0];
            gy = gy + Kp * ey + Ki * eInt[1];
            gz = gz + Kp * ez + Ki * eInt[2];

            // Integrate rate of change of quaternion
            pa = q2;
            pb = q3;
            pc = q4;
            q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * deltat);
            q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * deltat);
            q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * deltat);
            q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * deltat);

            // Normalise quaternion
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
            norm = 1.0f / norm;
            q[0] = q1 * norm;
            q[1] = q2 * norm;
            q[2] = q3 * norm;
            q[3] = q4 * norm;
 
        }

int _i2c_getFd(int adapterNum, int *devHandle){
	int 	status;
	char 	pathname[255];

	// define the path to open
	status = snprintf(pathname, sizeof(pathname), I2C_DEV_PATH, adapterNum);

	// check the filename
	if (status < 0 || status >= sizeof(pathname)) {
		// add errno
        return EXIT_FAILURE;
    }

	// create a file descriptor for the I2C bus
#ifdef I2C_ENABLED
  	*devHandle = open(pathname, O_RDWR);
#else
  	*devHandle = 0;
#endif

  	// check the defvice handle
  	if (*devHandle < 0) {
  		// add errno
  		return EXIT_FAILURE;
  	}

  	return EXIT_SUCCESS;
}

// release the device file handle
int _i2c_releaseFd(int devHandle)
{
#ifdef I2C_ENABLED
	if ( close(devHandle) < 0 ) {
		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}

// set the device address
int _i2c_setDevice(int devHandle, int addr)
{
#ifdef I2C_ENABLED
	// set to 7-bit addr
	if ( ioctl(devHandle, I2C_TENBIT, 0) < 0 ) {
		return EXIT_FAILURE;
	}

	// set the address
	if ( ioctl(devHandle, I2C_SLAVE, addr) < 0 ) {
		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}

// set the 10bit device address
int _i2c_setDevice10bit(int devHandle, int addr){
#ifdef I2C_ENABLED
	// set to 10-bit addr
	if ( ioctl(devHandle, I2C_TENBIT, 1) < 0 ) {
		return EXIT_FAILURE;
	}

	// set the address
	if ( _i2c_setDevice(devHandle, addr) != EXIT_SUCCESS ) {
		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}

// generic function to write a buffer to the i2c bus
int _i2c_writeBuffer(int devNum, int devAddr, uint8_t *buffer, int size)
{
	int 	status;
	int 	fd, index;

	// open the file handle
	status 	= _i2c_getFd(devNum, &fd);

	// set the device address
	if ( status == EXIT_SUCCESS ) {
		status 	= _i2c_setDevice(fd, devAddr);
	}

	//onionPrint(ONION_SEVERITY_DEBUG_EXTRA, "%s writing buffer:\n", I2C_PRINT_BANNER);
	for (index = 0; index < size; index++) {
		//onionPrint(ONION_SEVERITY_DEBUG_EXTRA, "\tbuffer[%d]: 0x%02x\n", index, buffer[index]);
	}

	// perform the write
	if ( status == EXIT_SUCCESS ) {
#ifdef I2C_ENABLED
		// write to the i2c device
		status = write(fd, buffer, size);
		if (status != size) {
			//onionPrint(ONION_SEVERITY_FATAL, "%s write issue for register 0x%02x, errno is %d: %s\n", I2C_PRINT_BANNER, buffer[0], errno, strerror(errno) );
			status 	= EXIT_FAILURE;
		}
		else {
			status	= EXIT_SUCCESS;
		}
#endif
 	}

 	// release the device file handle
 	status 	|= _i2c_releaseFd(fd);

	return (status);
}

// generic function to write a buffer to the i2c bus
int i2c_writeBuffer(int devNum, int devAddr, int addr, uint8_t *buffer, int size)
{
	int 	status;
	uint8_t *bufferNew;

	// allocate the new buffer
	size++;		// adding addr to buffer
	bufferNew 	= malloc( size * sizeof *bufferNew );

	// add the address to the data buffer
	bufferNew[0]	= addr;
	memcpy( &bufferNew[1], &buffer[0], size * sizeof *buffer );

 	// perform the write
 	status 	= _i2c_writeBuffer(devNum, devAddr, bufferNew, size);

 	// free the allocated memory
 	free(bufferNew);

	return (status);
}

// generic function to write a buffer to the i2c bus (no in-device address
int i2c_writeBufferRaw(int devNum, int devAddr, uint8_t *buffer, int size){
	return _i2c_writeBuffer(devNum, devAddr, buffer, size);
}

// write n bytes to the i2c bus
int i2c_write(int devNum, int devAddr, int addr, int val){
	int 	status;
	int 	size, tmp, index;
	uint8_t	buffer[I2C_BUFFER_SIZE]; 

	//// buffer setup
	// clear the buffer
	memset( buffer, 0, I2C_BUFFER_SIZE );
	// push the address and data values into the buffer
	buffer[0]	= (addr & 0xff);
	buffer[1]	= (val & 0xff);
	size 		= 2;

	// if value is more than 1-byte, add to the buffer
	tmp 	= (val >> 8);	// start with byte 1
	index	= 2;
	while (tmp > 0x00) {
		buffer[index] = (uint8_t)(tmp & 0xff);

		tmp	= tmp >> 8; // advance the tmp data by a byte
		index++; 		// increment the index

		size++;			// increase the size
	}

	//onionPrint(ONION_SEVERITY_DEBUG, "%s Writing to device 0x%02x: addr = 0x%02x, data = 0x%02x (data size: %d)\n", I2C_PRINT_BANNER, devAddr, addr, val, (size-1) );

	// write the buffer
 	status = _i2c_writeBuffer(devNum, devAddr, buffer, size);

	return (status);
}

// write a specified number of bytes to the i2c bus
int i2c_writeBytes(int devNum, int devAddr, int addr, int val, int numBytes){
	int 	status;
	int 	size, index;
	uint8_t	buffer[I2C_BUFFER_SIZE];

	//// buffer setup
	// clear the buffer
	memset( buffer, 0, sizeof(buffer) );
	// push the address and data values into the buffer
	buffer[0]	= (addr & 0xff);
	size 		= 1;

	// add all data bytes to buffer
	index	= 1;
	for (index = 0; index < numBytes; index++) {
		buffer[index+1] = (uint8_t)( (val >> (8*index)) & 0xff );

		size++;			// increase the size
	}

	//onionPrint(ONION_SEVERITY_DEBUG, "%s Writing to device 0x%02x: addr = 0x%02x, data = 0x%02x (data size: %d)\n", I2C_PRINT_BANNER, devAddr, addr, val, (size-1) );

	// write the buffer
	status 	= _i2c_writeBuffer(devNum, devAddr, buffer, size);

	return (status);
}

// read a byte from the i2c bus
int i2c_read(int devNum, int devAddr, int addr, uint8_t *buffer, int numBytes){
	int 	status, size, index;
	int 	fd;

	//onionPrint(ONION_SEVERITY_DEBUG, "%s Reading %d byte%s from device 0x%02x: addr = 0x%02x", I2C_PRINT_BANNER, numBytes, (numBytes > 1 ? "s": ""), devAddr, addr);

	// open the device file handle
	status 	= _i2c_getFd(devNum, &fd);

	// set the device address
	if ( status == EXIT_SUCCESS ) {
		status 	= _i2c_setDevice(fd, devAddr);
	}

	// perform the read 	
	if ( status == EXIT_SUCCESS ) {
		//// set addr
		// clear the buffer
		memset( buffer, 0, numBytes );
		// push the address and data values into the buffer
		buffer[0]	= (addr & 0xff);
		size 		= 1;

#ifdef I2C_ENABLED
		// write to the i2c device
		status = write(fd, buffer, size);
		if (status != size) {
			//onionPrint(ONION_SEVERITY_FATAL, "%s write issue for register 0x%02x, errno is %d: %s\n", I2C_PRINT_BANNER, addr, errno, strerror(errno) );
		}
#endif

		//// read data
		// clear the buffer
		memset( buffer, 0, numBytes );

#ifdef I2C_ENABLED
		// read from the i2c device
		size 	= numBytes;
		status 	= read(fd, buffer, size);
		if (status != size) {
			//onionPrint(ONION_SEVERITY_FATAL, "%s read issue for register 0x%02x, errno is %d: %s\n", I2C_PRINT_BANNER, addr, errno, strerror(errno) );
			status 	= EXIT_FAILURE;
		}
		else {
			status 	= EXIT_SUCCESS;
		}
#else
		buffer[0]	= 0x0;
		size 		= 1;
		/*
		// for debug
		printf("Setting buffer... it has length of %d\n", strlen(buffer) );
		buffer[0] 	= 0x34;
		buffer[1] 	= 0x12;
		size = 2;
		printf("Done setting buffer... it has length of %d\n", strlen(buffer) );
		printf("size is %d\n", size);*/
#endif		

		//// print the data
		//onionPrint(ONION_SEVERITY_DEBUG, "\tread %d byte%s, value: 0x", size, (size > 1 ? "s" : "") );

		for (index = (size-1); index >= 0; index--) {
			//onionPrint(ONION_SEVERITY_DEBUG, "%02x", (buffer[index] & 0xff) );
		}
		//onionPrint(ONION_SEVERITY_DEBUG, "\n");
 	}

 	// release the device file handle
 	status 	|= _i2c_releaseFd(fd);

	return (status);
}

// read a byte from the i2c bus
int i2c_readRaw(int devNum, int devAddr, uint8_t *buffer, int numBytes){
	int 	status, size, index;
	int 	fd;

	//onionPrint(ONION_SEVERITY_DEBUG, "%s Reading %d byte%s from device 0x%02x", I2C_PRINT_BANNER, numBytes, (numBytes > 1 ? "s": ""), devAddr);

	// open the device file handle
	status 	= _i2c_getFd(devNum, &fd);

	// set the device address
	if ( status == EXIT_SUCCESS ) {
		status 	= _i2c_setDevice(fd, devAddr);
	}

	// perform the read
	if ( status == EXIT_SUCCESS ) {
		//// read data
		// clear the buffer
		memset( buffer, 0, I2C_BUFFER_SIZE );

#ifdef I2C_ENABLED
		// read from the i2c device
		size 	= numBytes;
		status 	= read(fd, buffer, size);
		if (status != size) {
			//onionPrint(ONION_SEVERITY_FATAL, "%s read issue, errno is %d: %s\n", I2C_PRINT_BANNER, errno, strerror(errno) );
			status 	= EXIT_FAILURE;
		} else {
			status 	= EXIT_SUCCESS;
		}
#else
		buffer[0]	= 0x0;
		size 		= 1;
		/*
		// for debug
		printf("Setting buffer... it has length of %d\n", strlen(buffer) );
		buffer[0] 	= 0x34;
		buffer[1] 	= 0x12;
		size = 2;
		printf("Done setting buffer... it has length of %d\n", strlen(buffer) );
		printf("size is %d\n", size);*/
#endif

		//// print the data
		//onionPrint(ONION_SEVERITY_DEBUG, "\tread %d byte%s, value: 0x", size, (size > 1 ? "s" : "") );

		for (index = (size-1); index >= 0; index--) {
			//onionPrint(ONION_SEVERITY_DEBUG, "%02x", (buffer[index] & 0xff) );
		}
		//onionPrint(ONION_SEVERITY_DEBUG, "\n");
 	}

 	// release the device file handle
 	status 	|= _i2c_releaseFd(fd);

	return (status);
}

	// read a single byte from the i2c bus
	int i2c_readByte(int devNum, int devAddr, int addr, int *val){
		int 	status;
		uint8_t	buffer[I2C_BUFFER_SIZE];
		status	= i2c_read(devNum, devAddr, addr, buffer, 1);
		*val 	= (int)(buffer[0]);
		return (status);
	}
	// Timestamp in microseconds
	long getMicrotime(){
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
	}
