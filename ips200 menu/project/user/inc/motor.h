#ifndef _MOTOR_H
#define _MOTOR_H
/*头文件*/
#include "zf_common_headfile.h"
/*宏定义*/
#define SPEED_MAX            (3000)                                               // 最大 MAX_DUTY% 占空比
#define SPEED_MIN            (-3000 )                                               // 最大 MAX_DUTY% 占空比
#define DIFF_MAX            (7000 )        //4000                                       // 最大 MAX_DUTY% 占空比
#define DIFF_MIN            (-7000  )        //4000                                       // 最大 MAX_DUTY% 占空比
#define DIR_L                (A2 )
#define PWM_L               (TIM5_PWM_CH4_A3)

#define DIR_R               (A0 )
#define PWM_R               (TIM5_PWM_CH2_A1)
typedef struct {
    // 速度参数
    int Base_Speed;         // 基准速度
    int Speed_Left_Set;     // 左轮设定速度
    int Speed_Right_Set;    // 右轮设定速度
    int Straight_Speed;     // 直道高速   
	  volatile int encoderr;
	  volatile int encoderl;
    // 控制参数
    volatile float err;              // 当前误差
	  volatile float last_err;
		volatile float speed_lasterrL;
	  volatile float speed_err;
		volatile float speed_lasterrR;
		volatile float P_DIRE;
		volatile float D_DIRE;
		volatile float P_SPEED;
		volatile float I_SPEED;
	//设置电机
	  float pwm_l;
	  float pwm_r;
		volatile float steer_output;
} control;

extern control my_control;
void Motor_Right(int pwm_R);
void Motor_Left(int pwm_L);
/*误差*/
float Err_Sum(void);
float err_sum_average(uint8 start_point,uint8 end_point);
float Optimized_Err_Sum(void);
float my_abs(float x);
/*速度*/
void CascadeControl(float speed_l, float speed_r,int DesireSpeed);
void PID_DIR(int offset);
void PID_SPEED(float speed_l, float speed_r,int DesireSpeed);
void PID2_SPEED(float speed, int DesireSpeed);
#endif