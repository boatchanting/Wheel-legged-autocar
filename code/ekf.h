#include "zf_common_headfile.h"
#ifndef CODE_EKF_H_
#define CODE_EKF_H_


#define DEG_TO_RAD      (57.295779513082320876798154814105f)
#define dt              (0.005f)
#define K               (0.9f)


typedef struct
{
        float gyro_x;
        float gyro_y;
        float gyro_z;
        float acc_x;
        float acc_y;
        float acc_z;
}imu_t;





void EKF_Init(void);
void EKF_UpData(void);


#endif /* CODE_EKF_H_ */
