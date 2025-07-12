#include "motor_test.h"
#include "motor.h"
/*-------------------------------------------------------------------------------------------------------------------
  @brief     差速的计算
  @param     价值，误差，两个参数
  @return    获取到的diff
  @note      计算
-------------------------------------------------------------------------------------------------------------------*/
//函数本体 //简单的位置pid
int PD1_DIFF(float expect_val,float err,float P,float D)//舵机PD调节
{
   float  u;
   volatile static float error_current,error_last;
   float ek,ek1;
   error_current=err-expect_val;
   ek=error_current;
   ek1=error_current-error_last;
   u=P*ek+D*ek1;
   error_last=error_current;
 
   if(u>=DIFF_MAX )//限幅处理
       u=DIFF_MAX;
   else if(u<=DIFF_MIN)//限幅处理
       u=DIFF_MIN;
   return (int)u;
}
//函数本体 //简单的位置pid
int PD_DIFF(float expect_val,float err,float P,float D)//舵机PD调节
{
   float  u;
   volatile static float error_current,error_last;
   float ek,ek1;
   error_current=err-expect_val;
   ek=error_current;
   ek1=error_current-error_last;
   u=P*ek+D*ek1;
   error_last=error_current;
 
   if(u>=DIFF_MAX )//限幅处理
       u=DIFF_MAX;
   else if(u<=DIFF_MIN)//限幅处理
       u=DIFF_MIN;
   return (int)u;
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     PID控制
  @param     int set_speed ,int speed,期望值，实际值,参数值
  @return    电机占空比SPEED_MIN~SPEED_MAX
  Sample     pwm_R= PID_R(set_speed_right,right_wheel);//pid控制电机转速
             pwm_L= PID_L(set_speed_left,left_wheel ); //pid控制电机转速
  @note      调参是门玄学
-------------------------------------------------------------------------------------------------------------------*/
int PID_L(int set_speed ,int speed,float kp, float ki)//pid控制电机转速//使用的pid可以多套
{
    volatile static int out;
    volatile static int out_increment;
    volatile static int ek,ek1;
  //  float kp=1.46,ki=2.3;
 
   
    ek1 = ek;
    ek = set_speed - speed;
    out_increment= (int)(kp*(ek-ek1) + ki*ek);
    out+= out_increment;
 
    if(out>=SPEED_MAX)//限幅处理
        out=SPEED_MAX;
    else if(out<=SPEED_MIN)
        out=SPEED_MIN;
    return (int) out;
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     PID控制
  @param     int set_speed ,int speed,期望值，实际值,参数值
  @return    电机占空比SPEED_MIN~SPEED_MAX
  Sample     pwm_R= PID_R(set_speed_right,right_wheel);//pid控制电机转速
             pwm_L= PID_L(set_speed_left,left_wheel ); //pid控制电机转速
  @note      调参是门玄学
-------------------------------------------------------------------------------------------------------------------*/
int PID_R(int set_speed ,int speed,float kp, float ki)//pid控制电机转速//使用的pid可以多套
{
    volatile static int out;
    volatile static int out_increment;
    volatile static int ek,ek1;
  //  float kp=1.46,ki=2.3;
   float kp_out,ki_out;
   
    ek1 = ek;
    ek = set_speed - speed;
	  kp_out=kp*(ek-ek1);
	  ki_out=ki*ek;
	  ki_out=(ki>=100)?0:ki_out;
	  ki_out=(ki<=-100)?0:ki_out;
    out_increment= (int)(kp_out + ki_out);
    out+= out_increment;
 
    if(out>=SPEED_MAX)//限幅处理
        out=SPEED_MAX;
    else if(out<=SPEED_MIN)
        out=SPEED_MIN;
    return (int) out;
}

 