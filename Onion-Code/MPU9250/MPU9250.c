#include "MPU9250_header.h"
#include "mpu9250_methoden.c"

float sum = 0;
uint32_t sumCount = 0;
char buffer[14];

int main(){
    printf("main startet...\n\r");
    int data;
    
    // Read the WHO_AM_I register, this is a good test of communication
    int status = i2c_readByte(0, MPU9250_ADDRESS, WHO_AM_I_MPU9250, &data);  // Read WHO_AM_I register for MPU-9250
    if (data == 0x71){ // WHO_AM_I should always be 0x75
        //printf("in if-schleife\n\r");
        resetMPU9250(); // Reset registers to default in preparation for device calibration
        //printf("MPU9250 reseted...\n\r");
        
        // führt zu plausiblen Ausgaben
        //MPU9250SelfTest(SelfTest); // Start by performing self test and reporting values
        //printf("x-axis self test: acceleration trim within : %f of factory value\n\r", SelfTest[0]);  
        //printf("y-axis self test: acceleration trim within : %f of factory value\n\r", SelfTest[1]);  
        //printf("z-axis self test: acceleration trim within : %f of factory value\n\r", SelfTest[2]);  
        //printf("x-axis self test: gyration trim within : %f of factory value\n\r", SelfTest[3]);  
        //printf("y-axis self test: gyration trim within : %f of factory value\n\r", SelfTest[4]);  
        //printf("z-axis self test: gyration trim within : %f of factory value\n\r", SelfTest[5]);  
        
        //calibrateMPU9250(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers  
        //printf("x gyro bias = %f\n\r", gyroBias[0]);
        //printf("y gyro bias = %f\n\r", gyroBias[1]);
        //printf("z gyro bias = %f\n\r", gyroBias[2]);
        //printf("x accel bias = %f\n\r", accelBias[0]);
        //printf("y accel bias = %f\n\r", accelBias[1]);
        //printf("z accel bias = %f\n\r", accelBias[2]);
        //printf("nach calibrate\n\r");  
        
        usleep(2000000);
        // Initialize device for active mode read of acclerometer, gyroscope, and temperature
        initMPU9250(); 
        printf("MPU9250 initialized for active data mode...\n\r"); 
        
        // Initialize device for active mode read of magnetometer
        initAK8963(magCalibration);
        printf("AK8963 initialized for active data mode...\n\r"); 
        printf("Accelerometer full-scale range = %f  g\n\r", 2.0f*(float)(1<<Ascale));
        printf("Gyroscope full-scale range = %f  deg/s\n\r", 250.0f*(float)(1<<Gscale));
        if(Mscale == 0) printf("Magnetometer resolution = 14  bits\n\r");
        if(Mscale == 1) printf("Magnetometer resolution = 16  bits\n\r");
        if(Mmode == 2) printf("Magnetometer ODR = 8 Hz\n\r");
        if(Mmode == 6) printf("Magnetometer ODR = 100 Hz\n\r");
        usleep(1000000);
    } else {
        printf("Could not connect to MPU9250: \n\r");
        printf("%#x \n",  data);
        while(1) ; // Loop forever if communication doesn't happen
    }
 
    getAres(); // Get accelerometer sensitivity
    getGres(); // Get gyro sensitivity
    getMres(); // Get magnetometer sensitivity
    printf("Accelerometer sensitivity is %f LSB/g \n\r", 1.0f/aRes);
    printf("Gyroscope sensitivity is %f LSB/deg/s \n\r", 1.0f/gRes);
    printf("Magnetometer sensitivity is %f LSB/G \n\r", 1.0f/mRes);
    magbias[0] = +470.;  // User environmental x-axis correction in milliGauss, should be automatically calculated
    magbias[1] = +120.;  // User environmental x-axis correction in milliGauss
    magbias[2] = +125.;  // User environmental x-axis correction in milliGauss
 
    while(1) {
  
        // If intPin goes high, all data registers have new data
        // On interrupt, check if data ready interrupt
        //if (i2c_readByte(0, MPU9250_ADDRESS, INT_STATUS, &rdByte) & 0x01) {  
        i2c_readByte(0, MPU9250_ADDRESS, INT_STATUS, &rdByte);
        if (rdByte & 0x01) {  
            
          readAccelData(accelCount);  // Read the x/y/z adc values   
          // Now we'll calculate the accleration value into actual g's
          ax = (float)accelCount[0]*aRes - accelBias[0];  // get actual g value, this depends on scale being set
          ay = (float)accelCount[1]*aRes - accelBias[1];   
          az = (float)accelCount[2]*aRes - accelBias[2];  

          readGyroData(gyroCount);  // Read the x/y/z adc values
          // Calculate the gyro value into actual degrees per second
          gx = (float)gyroCount[0]*gRes - gyroBias[0];  // get actual gyro value, this depends on scale being set
          gy = (float)gyroCount[1]*gRes - gyroBias[1];  
          gz = (float)gyroCount[2]*gRes - gyroBias[2];   

          readMagData(magCount);  // Read the x/y/z adc values   
          // Calculate the magnetometer values in milliGauss
          // Include factory calibration per data sheet and user environmental corrections
          mx = (float)magCount[0]*mRes*magCalibration[0] - magbias[0];  // get actual magnetometer value, this depends on scale being set
          my = (float)magCount[1]*mRes*magCalibration[1] - magbias[1];  
          mz = (float)magCount[2]*mRes*magCalibration[2] - magbias[2];   
        }

        //Now = t.read_us();
        deltat = (float)((Now - lastUpdate)/1000000.0f) ; // set integration time by time elapsed since last filter update
        lastUpdate = Now;

        sum += deltat;
        sumCount++;

        //    if(lastUpdate - firstUpdate > 10000000.0f) {
        //     beta = 0.04;  // decrease filter gain after stabilized
        //     zeta = 0.015; // increasey bias drift gain after stabilized
        //   }

        // Pass gyro rate as rad/s
        //  mpu9250.MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  my,  mx, mz);
        //MahonyQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f, my, mx, mz);

        // Serial print and/or display at 0.5 s rate independent of data rates
        //delt_t = t.read_ms() - count;
        //if (delt_t > 50) { // update LCD once per half-second (500) independent of read rate

            // ouput as csv
            long timeinusec = getMicrotime();
            printf("%lu", timeinusec);
            
            printf(";%f", 1000*ax); 
            printf(";%f", 1000*ay); 
            printf(";%f\n\r", 1000*az); 
        
            //printf("ax = %f", 1000*ax); 
            //printf(" ay = %f", 1000*ay); 
            //printf(" az = %f  mg\n\r", 1000*az); 

            //printf("gx = %f", gx); 
            //printf(" gy = %f", gy); 
            //printf(" gz = %f  deg/s\n\r", gz); 

            //printf("mx = %f", mx); 
            //printf(" my = %f", my); 
            //printf(" mz = %f  mG\n\r", mz); 

            // tempCount = mpu9250.readTempData();  // Read the adc values
            //temperature = ((float) tempCount) / 333.87f + 21.0f; // Temperature in degrees Centigrade
            //printf(" temperature = %f  C\n\r", temperature); 

            //printf("q0 = %f\n\r", q[0]);
            //printf("q1 = %f\n\r", q[1]);
            //printf("q2 = %f\n\r", q[2]);
            //printf("q3 = %f\n\r", q[3]);      

            /*    lcd.clear();
                lcd.printString("MPU9250", 0, 0);
                lcd.printString("x   y   z", 0, 1);
                sprintf(buffer, "%d %d %d mg", (int)(1000.0f*ax), (int)(1000.0f*ay), (int)(1000.0f*az));
                lcd.printString(buffer, 0, 2);
                sprintf(buffer, "%d %d %d deg/s", (int)gx, (int)gy, (int)gz);
                lcd.printString(buffer, 0, 3);
                sprintf(buffer, "%d %d %d mG", (int)mx, (int)my, (int)mz);
                lcd.printString(buffer, 0, 4); 
             */ 

             // Define output variables from updated quaternion---these are Tait-Bryan 
             // angles, commonly used in aircraft orientation.
             // In this coordinate system, the positive z-axis is down toward Earth. 
             // Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
             // Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
             // Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
             // These arise from the definition of the homogeneous rotation matrix constructed from quaternions.
             // Tait-Bryan angles as well as Euler angles are non-commutative; that is, the get the correct orientation the rotations must be
             // applied in the correct order which for this configuration is yaw, pitch, and then roll.
             // For more see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles which has additional links.
             //yaw   = atan2(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);   
             //pitch = -asin(2.0f * (q[1] * q[3] - q[0] * q[2]));
             //roll  = atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);
             //pitch *= 180.0f / PI;
             //yaw   *= 180.0f / PI; 
             //yaw   -= 13.8f; // Declination at Danville, California is 13 degrees 48 minutes and 47 seconds on 2014-04-04
             //roll  *= 180.0f / PI;

             // printf("Yaw, Pitch, Roll: %f %f %f\n\r", yaw, pitch, roll);
             // printf("average rate = %f\n\r", (float) sumCount/sum);

             //    sprintf(buffer, "YPR: %f %f %f", yaw, pitch, roll);
             //    lcd.printString(buffer, 0, 4);
             //    sprintf(buffer, "rate = %f", (float) sumCount/sum);
             //    lcd.printString(buffer, 0, 5);

             //myled= !myled;
//           count = t.read_ms(); 

//           if(count > 1<<21) {
//                t.start(); // start the timer over again if ~30 minutes has passed
//                count = 0;
//                deltat= 0;
//                lastUpdate = t.read_us();
//            }
//            sum = 0;
//            sumCount = 0; 
        //}
    }
}
