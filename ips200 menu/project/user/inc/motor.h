#ifndef _MOTOR_H
#define _MOTOR_H
/*头文件*/
#include "zf_common_headfile.h"
/*宏定义*/
#define SPEED_MAX            (5000 )                                               // 最大 MAX_DUTY% 占空比
#define SPEED_MIN            (-5000 )                                               // 最大 MAX_DUTY% 占空比
#define DIR_L               (A0 )
#define PWM_L               (TIM5_PWM_CH2_A1)

#define DIR_R               (A2 )
#define PWM_R               (TIM5_PWM_CH4_A3)
typedef struct {
    // 速度参数
    int Base_Speed;         // 基准速度
    int Speed_Left_Set;     // 左轮设定速度
    int Speed_Right_Set;    // 右轮设定速度
    int Straight_Speed;     // 直道高速   
	  int encoder1;
	  int encoder2;
    // 控制参数
    float err;              // 当前误差
	  float last_err;
		float speed_lasterrL;
		float speed_lasterrR;
		float P_DIRE;
		float D_DIRE;
		float P_SPEED;
		float I_SPEED;
	//设置电机
	  float pwm_l;
	  float pwm_r;
		
} control;
typedef struct {

    volatile uint8 Go;             //发车阶段的标志位
	  








}flag;
extern control my_control;
void Motor_Right(int pwm_R);
void Motor_Left(int pwm_L);
/*误差*/
float Err_Sum(void);
float Optimized_Err_Sum(void);
/*速度*/
void CascadeControl(float speed_l, float speed_r,int DesireSpeed);
#endif