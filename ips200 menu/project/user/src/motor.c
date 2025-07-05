#include "motor.h"
#include "image.h"
#include <math.h>

#define DIFF_MAX     20
#define DIFF_MIN      0
//加权控制
const uint8 Weight[MT9V03X_H]=
{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端20 ——29 行权重
        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端40 ——49 行权重
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端60 ——69 行权重
};
control my_control = {
    .Base_Speed = 0,
    .Speed_Left_Set = 5000,
    .Speed_Right_Set = 5000,
    .Straight_Speed = 0,
    .err = 0.0f,
    .last_err = 0.0f,
    .speed_lasterrL = 0.0f,
    .speed_lasterrR = 0.0f,
    .P_DIRE = 0.0f,
    .D_DIRE = 0.0f,
    .P_SPEED = 0.0f,
    .I_SPEED = 0.0f,
	  .pwm_l=0.0f,
	  .pwm_r=0.0f,
	  .encoder1=0,//1300
	  .encoder2=0 //1400
};
//0否定
flag  my_flag={
    .Go=0





};
/*-------------------------------------------------------------------------------------------------------------------
@brief     右电机
  @param     pwm_R
  @return    null
  @note     注意范围
-------------------------------------------------------------------------------------------------------------------*/
void Motor_Right(int pwm_R)
{
    if(pwm_R>=SPEED_MAX)//限幅处理
        pwm_R=SPEED_MAX;
    else if(pwm_R<=SPEED_MIN)
        pwm_R=SPEED_MIN;
    if(pwm_R>=0)
   {
            gpio_set_level(DIR_R, GPIO_HIGH);                                   // DIR输出高电平
            pwm_set_duty(PWM_R, pwm_R );                   // 计算占空比
   }
  else
   {
            gpio_set_level(DIR_R, GPIO_LOW);                                    // DIR输出低电平
            pwm_set_duty(PWM_R, -pwm_R);                // 计算占空比
   }
}
/*-------------------------------------------------------------------------------------------------------------------
@brief     左电机
  @param     pwm_L
  @return    null
  @note     注意范围
-------------------------------------------------------------------------------------------------------------------*/
void Motor_Left(int pwm_L)
{
    if(pwm_L>=SPEED_MAX)//限幅处理
        pwm_L=SPEED_MAX;
    else if(pwm_L<=SPEED_MIN)
        pwm_L=SPEED_MIN;
    if(pwm_L>=0)
   {
            gpio_set_level(DIR_L, GPIO_HIGH);                                   // DIR输出高电平
            pwm_set_duty(PWM_L, pwm_L );                   // 计算占空比  映射
   }
  else
   {
            gpio_set_level(DIR_L, GPIO_LOW);                                    // DIR输出低电平
            pwm_set_duty(PWM_L, -pwm_L);                // 计算占空比
   }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     摄像头误差获取
  @param     null
  @return    获取到的误差
  Sample     err=Err_Sum();
  @note      加权取平均
-------------------------------------------------------------------------------------------------------------------*/
float Err_Sum(void)
{
    int i;
    float err=0;
    float weight_count=0;
    //常规误差
    for(i=MT9V03X_H-1;i>=MT9V03X_H-my_image.Search_Stop_Line-1;i--)//常规误差计算
    {
        err+=(MT9V03X_W/2-((my_image.Left_Line[i]+my_image.Right_Line[i])>>1))*Weight[i];//右移1位，等效除2
        weight_count+=Weight[i];
    }
    err=err/weight_count;
    
    return err;
}

/**
 * @brief 优化后的误差计算函数，考虑前瞻距离和赛道斜率
 * @return 加权平均误差值（有正负）
 */
float Optimized_Err_Sum(void)
{
    int i;
    float err = 0.0f;
    float weight_sum = 0.0f;
    float dynamic_weight;
    
    // 计算赛道斜率（基于中线的前3行和最后1行）
    float slope = 0.0f;
    int valid_slope_samples = 0;
    const int slope_window = 3; // 斜率计算窗口大小
    
    // 确保有足够行数计算斜率
    if(my_image.Search_Stop_Line > slope_window) 
    {
        int start_row = MT9V03X_H - my_image.Search_Stop_Line;
        int end_row = MT9V03X_H - 1;
        
        // 计算起始行和结束行的中线位置
        float start_center = (my_image.Left_Line[start_row] + my_image.Right_Line[start_row]) * 0.5f;
        float end_center = (my_image.Left_Line[end_row] + my_image.Right_Line[end_row]) * 0.5f;
        
        // 计算斜率（单位：像素/行）
        slope = (end_center - start_center) / (float)(end_row - start_row);
    }
    
    // 动态权重因子：斜率越大，权重调整越大
    float slope_factor = 1.0f + 0.5f * tanh(slope * 2.0f); // 使用tanh限制调整幅度
    
    // 距离因子：距离越远，权重越大
    float distance_factor = (float)my_image.Search_Stop_Line / MT9V03X_H;
    
    // 计算加权误差
    for(i = MT9V03X_H - 1; i >= MT9V03X_H - my_image.Search_Stop_Line; i--)
    {
        // 动态权重 = 基础权重 × 斜率因子 × 距离因子
        dynamic_weight = Weight[i] * slope_factor * (1.0f + distance_factor);
        
        // 计算中线偏差（使用浮点运算提高精度）
        float center = (my_image.Left_Line[i] + my_image.Right_Line[i]) * 0.5f;
        err += (MT9V03X_W * 0.5f - center) * dynamic_weight;
        weight_sum += dynamic_weight;
    }
    
    // 返回加权平均误差（防止除零）
    return (weight_sum > 0.001f) ? (err / weight_sum) : 0.0f;
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     差速的计算
  @param     价值，误差，两个参数
  @return    获取到的diff
  @note      计算
-------------------------------------------------------------------------------------------------------------------*/
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

 
void Velocity_Control(int speed_left_real,int speed_right_real)//赛道类型判别，来选定速度
{
    int pwm_R=0,pwm_L=0;
  
        //速度决策
 
 
 
 
        //速度决策
      //  pwm_L= PID_L(Speed_Left_Set ,speed_left_real );//pid控制电机转速
    //    pwm_R= PID_R(Speed_Right_Set,speed_right_real);//pid控制电机转速
 
        //棒棒作为[PID的辅助
        if(my_control.Speed_Left_Set - speed_left_real>150)
        {
            pwm_L=SPEED_MAX;
        }
        else if(my_control.Speed_Left_Set - speed_left_real<-150)
        {
            pwm_L=SPEED_MIN;
        }
        if(my_control.Speed_Right_Set- speed_right_real>150)
        {
            pwm_L=SPEED_MAX;
        }
        else if(my_control.Speed_Right_Set- speed_right_real<-150)
        {
            pwm_L=SPEED_MIN;
        }
    
    Motor_Left (pwm_L);
    Motor_Right(pwm_R);
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     PID串行控制
  @param     int set_speed ,int speed,期望值，实际值,参数值
  @return    电机占空比SPEED_MIN~SPEED_MAX
  Sample     pwm_R= PID_R(set_speed_right,right_wheel);//pid控制电机转速
             pwm_L= PID_L(set_speed_left,left_wheel ); //pid控制电机转速
  @note      调参是门玄学
-------------------------------------------------------------------------------------------------------------------*/
void CascadeControl(float speed_l, float speed_r,int DesireSpeed) {
    /******************** 1. 方向环（内环）PD计算 ********************/
   // float steer_error = offset; // 当前横向偏差
    float steer_output;         // 方向环输出（速度修正量）
    
    // PD计算（位置式）
    steer_output = my_control.P_DIRE * my_control.err 
                 + my_control.D_DIRE * (my_control.err  - my_control.last_err);
    my_control.err  = my_control.last_err ; // 更新误差

    // 方向环输出限幅（与期望速度同量级，如±0.3m/s）
    if (steer_output > 0.3f)  steer_output = 0.3f;
    if (steer_output < -0.3f) steer_output = -0.3f;

    /******************** 2. 速度环（外环）输入合成 ********************/
    float target_speed_l = DesireSpeed + steer_output; // 左轮目标速度
    float target_speed_r = DesireSpeed - steer_output; // 右轮目标速度

    /******************** 3. 速度环PI计算（增量式） ********************/
    // 左轮速度环
    float speed_err_l = target_speed_l - speed_l;
    int pwm_l = (int)(my_control.speed_lasterrL+ 
              (my_control.I_SPEED * speed_err_l) + 
              (my_control.P_SPEED * (speed_err_l - my_control.speed_lasterrL)));
    
    // 右轮速度环（独立计算误差）
    float speed_err_r = target_speed_r - speed_r;
    int pwm_r = (int)(my_control.speed_lasterrR + 
              (my_control.I_SPEED* speed_err_r) + 
              (my_control.P_SPEED * (speed_err_r - my_control.speed_lasterrR)));

    // 更新速度环历史误差（左轮）
   my_control.speed_lasterrL = speed_err_l;
    
    // 更新右轮误差（若独立计算）
    my_control.speed_lasterrR = speed_err_r;

    /******************** 4. PWM限幅及电机输出 ********************/
    // 左轮PWM限幅
    if (pwm_l > SPEED_MAX) pwm_l = SPEED_MAX;
    if (pwm_l < SPEED_MIN) pwm_l = SPEED_MIN;
    
    // 右轮PWM限幅
    if (pwm_r > SPEED_MAX) pwm_r = SPEED_MAX;
    if (pwm_r < SPEED_MIN) pwm_r = SPEED_MIN;
		
		my_control.pwm_l=pwm_l;
		my_control.pwm_r=pwm_r;
		
	//	Motor_Left(pwm_l);
//		Motor_Right(pwm_r);
    // 实际电机输出（需根据硬件接口调整）
  
}