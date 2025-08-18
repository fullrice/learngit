#include "motor.h"
#include "image.h"
#include "control.h"
#include "660.h"

#include <math.h>
#define PB  6
#define PM  5
#define PS  4
#define ZO  3
#define NS  2
#define NM  1
#define NB  0
 
float U=0;                            /*偏差,偏差微分以及输出值的精确量*/
float PF[2]={0},DF[2]={0},UF[4]={0};  /*偏差,偏差微分以及输出值的隶属度*/
int Pn=0,Dn=0,Un[4]={0};
float t1=0,t2=0,t3=0,t4=0,temp1=0,temp2=0;
int pwm_l,pwm_r=0;
//加权控制
//const uint8 Weight[MT9V03X_H]=
//{
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端20 ——29 行权重
//        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
//       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端40 ——49 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端60 ——69 行权重
//};
//const uint8 Weight[MT9V03X_H]=
//{
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
//        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
//       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端20 ——29 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端40 ——49 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端60 ——69 行权重
//};
//const uint8 Weight[MT9V03X_H]=
//{
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端20 ——29 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端30 ——39 行权重
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
//       1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端40 ——49 行权重
//       1, 1, 1, 1, 1, 1, 1, 1, 1, 1  ,              //图像最远端50 ——59 行权重
//       1, 1, 1, 1  ,1,15  ,91,15,1,               //图像最远端60 ——69 行权重
//};
int Weight[80] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 10-19 行（低权重）
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 20-29 行（低权重）
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 30-39 行（过渡）
    15, 30, 50, 80,                // 33-36 行（核心高权重）
    30, 15, 1, 1, 1, 1,           // 37-42 行（过渡下降）
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 43-52 行（低权重）
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 53-62 行（低权重）
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 63-72 行（低权重）
    1, 1, 1, 1, 1, 1, 1         // 73-79 行（补全最后 8 个值）
};
//const uint8 Weight[MT9V03X_H]=
//{
//        19,17,15,13,11,19,17,15,13,11 ,               //图像最远端00 ——09 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端20 ——29 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端30 ——39 行权重
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
//       1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端40 ——49 行权重
//       1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                 //图像最远端60 ——69 行权重
//};
//const uint8 Weight[MT9V03X_H]=
//{
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,  
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//	       19,17,15,13,11, 9, 7, 5, 3, 1, 
//        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
//       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端40 ——49 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重v             //图像最远端60 ——69 行权重
//};
//const uint8 Weight[MT9V03X_H]=
//{
//	      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重v             //图像最远端60 ——69 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,  
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
//       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端40 ——49 行权重
//};
//const uint8 Weight[MT9V03X_H]=
//{
//       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端40 ——49 行权重
//        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
//        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,  
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  //        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重v      
//        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重v             //图像最远端60 ——69 行权重
//};
control my_control = {
    .Base_Speed = 0,
    .Speed_Left_Set = 150 ,  //250
    .Speed_Right_Set = 150,
    .Straight_Speed = 0,
    .err = 0.0f,//左正右负，并且保证向左转的时候左轮小于右轮，那么
	  .front_err=0.0f,
    .last_err = 0.0f,
    .speed_lasterrL =0,
    .speed_lasterrR = 0.0f,
    .P_DIRE = -30              ,//-13 -25 -38  -36  -39  尽量偏左  调高并且换位置 -29 -30   -36   -39 （340）-45（360）  -32(260 260)
    .D_DIRE = 0         , //-0.2  -0.3  -0.2 微调
    .P_SPEED=50 , //5.69    5.99(0.002)  6.39 10.39  12.39 14.39
    .I_SPEED =2          , //0.1   0.001  5.69（0.0015）0.003 0.004
	  .pwm_l=0.0f   ,
	  .Shift_Ratio=0.0f,
	  .pwm_r=0.0f,
	  .encoderl=0,//1300
	  .encoderr=0,//1400
    .steer_output=0,
	  .speed_err=0,
	  .front   
	=27        ,
	  .max_encoderr=0,
		.max_encoderl=0,
		.right_offset=0,
		.left_offset=0
};
//0否定

/*-------------------------------------------------------------------------------------------------------------------
@brief     右电机
  @param     pwm_R
  @return    null
  @note     注意范围
-------------------------------------------------------------------------------------------------------------------*/
void Motor_Right(int pwm_R)
{
    if(pwm_R>=10000)//限幅处理
        pwm_R=10000;
    else if(pwm_R<=-10000)
        pwm_R=-10000;
    if(pwm_R<=0)
   {
		    //  gpio_set_level(DIR_L, GPIO_LOW);  //凡
              gpio_set_level(DIR_R, GPIO_HIGH);                                      // DIR输出高电平
		    //   gpio_set_level(DIR_R,GPIO_LOW);    
            pwm_set_duty(PWM_R, -pwm_R );                   // 计算占空比
   }
  else
   {
         // gpio_set_level(DIR_R, GPIO_HIGH);   //凡
		       gpio_set_level(DIR_R, GPIO_LOW);                                   // DIR输出低电平
            pwm_set_duty(PWM_R, pwm_R);                // 计算占空比
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
     if(pwm_L>=10000)//限幅处理
        pwm_L=10000;
    else if(pwm_L<=-10000)
        pwm_L=-10000;
    if(pwm_L>=0)
   {
		         gpio_set_level(DIR_L, GPIO_HIGH);    //凡                               // DIR输出高电平
          //   gpio_set_level(DIR_L, GPIO_LOW);                                  // DIR输出高电平
            pwm_set_duty(PWM_L, pwm_L );                   // 计算占空比  映射
   }
  else
   {
		  gpio_set_level(DIR_L, GPIO_LOW);  //凡
	//	 gpio_set_level(DIR_L, GPIO_HIGH);  
                                        // DIR输出低电平
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
float err_sum_average(uint8 start_point,uint8 end_point)
{
    //防止参数输入错误
    if(end_point<start_point)
    {
        uint8 t=end_point;
        end_point=start_point;
        start_point=t;
    }  

    if(start_point<MT9V03X_H-my_image.Search_Stop_Line)start_point=MT9V03X_H-my_image.Search_Stop_Line-1;//防止起点越界
    if(end_point<MT9V03X_H-my_image.Search_Stop_Line)end_point=MT9V03X_H-my_image.Search_Stop_Line-2;//防止终点越界
     
    float err=0;
//		   				for(int i=start_point;i<end_point;i++)
//				{ 
//						err+=(MT9V03X_W/2-((my_image.Left_Line[i]+my_image.Right_Line[i])>>1));//位操作等效除以2
//				}
		//双边
		//左圆环
						if(my_island.island_state==0 || my_island.island_state==5 )
				{
					//双边
						for(int i=start_point;i<end_point;i++)
						{   

								err+=(MT9V03X_W/2 - ((my_image.Left_Line[i]+my_image.Right_Line[i])/2  ));//位操作等效除以2 1.9  
						}
				}
		     if(my_island.detect==1)
				{
				    		if( my_island.island_state==3 )
								{
										//单边

												for(int i=start_point;i<end_point;i++)
												{
													err+=(MT9V03X_W/2 -Standard_Road_Wide[i]/2-my_image.Left_Line[i]-20  );//左  -20
											 //     err += (my_image.Right_Line[i] - (MT9V03X_W /2 + Standard_Road_Wide[i] / 2)); // 右巡线
												}

								 }
								if(my_island.island_state==1  || my_island.island_state==2  || my_island.island_state==4)
								{
												for(int i=start_point;i<end_point;i++)
												{
														err += ( (MT9V03X_W/2 + Standard_Road_Wide[i]/2) - my_image.Right_Line[i] );
												}
								}
				}
				else if (my_island.detect==2)
				{
						if( my_island.island_state==3 )
						{
								//单边

										for(int i=start_point;i<end_point;i++)
										{
											err += ( (MT9V03X_W/2 + Standard_Road_Wide[i]/2) - my_image.Right_Line[i]+20);
									 //     err += (my_image.Right_Line[i] - (MT9V03X_W /2 + Standard_Road_Wide[i] / 2)); // 右巡线
										}

						 }
						if(my_island.island_state==1  || my_island.island_state==2  || my_island.island_state==4)
						{
										for(int i=start_point;i<end_point;i++)
										{
												err+=(MT9V03X_W/2 -Standard_Road_Wide[i]/2-my_image.Left_Line[i]);//左  -20

										}
						}
				}
//			if(my_island.island_state==0 || my_island.island_state==5 )
//		{
//			//双边
//				for(int i=start_point;i<end_point;i++)
//				{   

//						err+=(MT9V03X_W/2 - ((my_image.Left_Line[i]+my_image.Right_Line[i])/2  ));//位操作等效除以2 1.9  
//				}
//		}
//		if( my_island.island_state==3 )
//		{
//				//单边

//						for(int i=start_point;i<end_point;i++)
//						{
//							err+=(MT9V03X_W/2 -Standard_Road_Wide[i]/2-my_image.Left_Line[i]-20  );//左  -20
//           //     err += (my_image.Right_Line[i] - (MT9V03X_W /2 + Standard_Road_Wide[i] / 2)); // 右巡线
//						}

//	   }
//		if(my_island.island_state==1  || my_island.island_state==2  || my_island.island_state==4)
//		{
//		        for(int i=start_point;i<end_point;i++)
//						{
//								err += ( (MT9V03X_W/2 + Standard_Road_Wide[i]/2) - my_image.Right_Line[i] );
//						}
//		}
		//右圆环
//		
//				if(my_island.island_state==0 || my_island.island_state==5 )
//				{
//					//双边
//						for(int i=start_point;i<end_point;i++)
//						{   

//								err+=(MT9V03X_W/2 - ((my_image.Left_Line[i]+my_image.Right_Line[i])/2  ));//位操作等效除以2 1.9  
//						}
//				}
//					if( my_island.island_state==3 )
//					{
//							//单边

//									for(int i=start_point;i<end_point;i++)
//									{
//										err += ( (MT9V03X_W/2 + Standard_Road_Wide[i]/2) - my_image.Right_Line[i]+20);
//								 //     err += (my_image.Right_Line[i] - (MT9V03X_W /2 + Standard_Road_Wide[i] / 2)); // 右巡线
//									}

//					 }
//					if(my_island.island_state==1  || my_island.island_state==2  || my_island.island_state==4)
//					{
//									for(int i=start_point;i<end_point;i++)
//									{
//									    err+=(MT9V03X_W/2 -Standard_Road_Wide[i]/2-my_image.Left_Line[i]);//左  -20

//									}
//					}
		
		
		
		
		
		
		
		
    my_control.front_err=(MT9V03X_W/2-((my_image.Left_Line[my_pdd.front_front]+my_image.Right_Line[my_pdd.front_front])/2));		
    err=err/(end_point-start_point);
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
float Fuzzy_P(int E,int EC)
{
 
 
//只要改下面这几行参数
    //这玩意没什么规律，p越大，转弯越好，直道会有抖动，p小转不过来，凭感觉调
    //建议先用单套pd，看看车子正常的p大概在什么范围，下面的p就会有方向
	float EFF[7]={-14,-7,-5,0,5,7,14};//摄像头误差分区
    /*输入量D语言值特征点*/
    float DFF[7]={-28,-21,-12,0,12,21,28};//误差变化率分区
    /*输出量U语言值特征点(根据赛道类型选择不同的输出值)*/
    float UFF[7]={-40,-38,-36,-34,-32,-30,0};//限幅分区
//只要改上面这几行参数
 
 
	int rule[7][7]={
    //    0   1   2   3   4   5   6
        { 6 , 5 , 4 , 3 , 2 , 1 , 0},//0
        { 5 , 4 , 3 , 2 , 1 , 0 , 1},//1
        { 4 , 3 , 2 , 1 , 0 , 1 , 2},//2
        { 3 , 2 , 1 , 0 , 1 , 2 , 3},//3
        { 2 , 1 , 0 , 1 , 2 , 3 , 4},//4
        { 1 , 0 , 1 , 2 , 3 , 4 , 5},//5
        { 0 , 1 , 2 , 3 , 4 , 5 , 6},//6
    };
 
    /*隶属度的确定*/
    /*根据PD的指定语言值获得有效隶属度*/
    if((E>(*(EFF+0))) && (E<(*(EFF+6))))
    {
        if(E<=((*(EFF+1))))
        {
            Pn=-2;
            *(PF+0)=((*(EFF+1))-E)/((*(EFF+1))-((*(EFF+0))));
        }
        else if(E<=((*(EFF+2))))
        {
            Pn=-1;
            *(PF+0)=((*(EFF+2))-E)/((*(EFF+2))-(*(EFF+1)));
        }
        else if(E<=((*(EFF+3))))
        {
            Pn=0;
            *(PF+0)=((*(EFF+3))-E)/((*(EFF+3))-(*(EFF+2)));
        }
        else if(E<=((*(EFF+4))))
        {
            Pn=1;
            *(PF+0)=((*(EFF+4))-E)/((*(EFF+4))-(*(EFF+3)));
        }
        else if(E<=((*(EFF+5))))
        {
            Pn=2;
            *(PF+0)=((*(EFF+5))-E)/((*(EFF+5))-(*(EFF+4)));
        }
        else if(E<=((*(EFF+6))))
        {
            Pn=3;
            *(PF+0)=((*(EFF+6))-E)/((*(EFF+6))-(*(EFF+5)));
        }
    }
 
    else if(E<=((*(EFF+0))))
    {
        Pn=-2;
        *(PF+0)=1;
    }
    else if(E>=((*(EFF+6))))
    {
        Pn=3;
        *(PF+0)=0;
    }
 
   *(PF+1)=1-(*(PF+0));
 
 
    //判断D的隶属度
    if(EC>(*(DFF+0))&&EC<(*(DFF+6)))
    {
        if(EC<=(*(DFF+1)))
        {
            Dn=-2;
            (*(DF+0))=((*(DFF+1))-EC)/((*(DFF+1))-(*(DFF+0)));
        }
        else if(EC<=(*(DFF+2)))
        {
            Dn=-1;
            (*(DF+0))=((*(DFF+2))-EC)/((*(DFF+2))-(*(DFF+1)));
        }
        else if(EC<=(*(DFF+3)))
        {
            Dn=0;
            (*(DF+0))=((*(DFF+3))-EC)/((*(DFF+3))-(*(DFF+2)));
        }
        else if(EC<=(*(DFF+4)))
        {
            Dn=1;
            (*(DF+0))=((*(DFF+4))-EC)/((*(DFF+4))-(*(DFF+3)));
        }
        else if(EC<=(*(DFF+5)))
        {
            Dn=2;
            (*(DF+0))=((*(DFF+5))-EC)/((*(DFF+5))-(*(DFF+4)));
        }
        else if(EC<=(*(DFF+6)))
        {
            Dn=3;
            (*(DF+0))=((*(DFF+6))-EC)/((*(DFF+6))-(*(DFF+5)));
        }
    }
    //不在给定的区间内
    else if (EC<=(*(DFF+0)))
    {
        Dn=-2;
        (*(DF+0))=1;
    }
    else if(EC>=(*(DFF+6)))
    {
        Dn=3;
        (*(DF+0))=0;
    }
 
    DF[1]=1-(*(DF+0));
 
    /*使用误差范围优化后的规则表rule[7][7]*/
    /*输出值使用13个隶属函数,中心值由UFF[7]指定*/
    /*一般都是四个规则有效*/
    Un[0]=rule[Pn+2][Dn+2];
    Un[1]=rule[Pn+3][Dn+2];
    Un[2]=rule[Pn+2][Dn+3];
    Un[3]=rule[Pn+3][Dn+3];
 
    if((*(PF+0))<=(*(DF+0)))    //求小
        (*(UF+0))=*(PF+0);
    else
        (*(UF+0))=(*(DF+0));
    if((*(PF+1))<=(*(DF+0)))
        (*(UF+1))=*(PF+1);
    else
        (*(UF+1))=(*(DF+0));
    if((*(PF+0))<=DF[1])
        (*(UF+2))=*(PF+0);
    else
        (*(UF+2))=DF[1];
    if((*(PF+1))<=DF[1])
        (*(UF+3))=*(PF+1);
    else
        (*(UF+3))=DF[1];
    /*同隶属函数输出语言值求大*/
    if(Un[0]==Un[1])
    {
        if(((*(UF+0)))>((*(UF+1))))
            (*(UF+1))=0;
        else
            (*(UF+0))=0;
    }
    if(Un[0]==Un[2])
    {
        if(((*(UF+0)))>((*(UF+2))))
            (*(UF+2))=0;
        else
            (*(UF+0))=0;
    }
    if(Un[0]==Un[3])
    {
        if((*(UF+0))>(*(UF+3)))
            (*(UF+3))=0;
        else
            (*(UF+0))=0;
    }
    if(Un[1]==Un[2])
    {
        if((*(UF+1))>(*(UF+2)))
            (*(UF+2))=0;
        else
            (*(UF+1))=0;
    }
    if(Un[1]==Un[3])
    {
        if((*(UF+1))>(*(UF+3)))
            (*(UF+3))=0;
        else
            (*(UF+1))=0;
    }
    if(Un[2]==Un[3])
    {
        if((*(UF+2))>(*(UF+3)))
            (*(UF+3))=0;
        else
            (*(UF+2))=0;
    }
    t1=((*(UF+0)))*(*(UFF+(*(Un+0))));
    t2=((*(UF+1)))*(*(UFF+(*(Un+1))));
    t3=((*(UF+2)))*(*(UFF+(*(Un+2))));
    t4=((*(UF+3)))*(*(UFF+(*(Un+3))));
    temp1=t1+t2+t3+t4;
    temp2=(*(UF+0))+(*(UF+1))+(*(UF+2))+(*(UF+3));//模糊量输出
    U=temp1/temp2;
    return U;
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
  //  mt_control.steer_output;         // 方向环输出（速度修正量）
    
    // PD计算（位置式）
    my_control.steer_output = my_control.P_DIRE * my_control.err 
                 + my_control.D_DIRE * (my_control.err  - my_control.last_err);
    my_control.last_err  = my_control.err ; // 更新误差

    // 方向环输出限幅（与期望速度同量级，如±0.3m/s）
  //  if (steer_output > 0.3f)  steer_output = 0.3f;
 //   if (steer_output < -0.3f) steer_output = -0.3f;

    /******************** 2. 速度环（外环）输入合成 ********************/
    float target_speed_l = DesireSpeed + my_control.steer_output; // 左轮目标速度
    float target_speed_r = DesireSpeed - my_control.steer_output; // 右轮目标速度

    /******************** 3. 速度环PI计算（增量式） ********************/
    // 左轮速度环
    float speed_err_l = target_speed_l - speed_l;
//    int pwm_l = (int)(my_control.speed_lasterrL+ 
//              (my_control.I_SPEED * speed_err_l) + 
//              (my_control.P_SPEED * (speed_err_l - my_control.speed_lasterrL)));
    pwm_l = (int)( (my_control.I_SPEED * speed_err_l) + 
              (my_control.P_SPEED * (speed_err_l - my_control.speed_lasterrL)));
    // 右轮速度环（独立计算误差）
    float speed_err_r = target_speed_r - speed_r;
		//my_control.speed_lasterrR
    pwm_r = (int)( (my_control.I_SPEED* speed_err_r) + 
              (my_control.P_SPEED * (speed_err_r - my_control.speed_lasterrR)));

    // 更新速度环历史误差（左轮）
   my_control.speed_lasterrL = speed_err_l;
    
    // 更新右轮误差（若独立计算）
    my_control.speed_lasterrR = speed_err_r;

    /******************** 4. PWM限幅及电机输出 ********************/
   
		
		my_control.pwm_l=my_control.pwm_l+pwm_l;//
		my_control.pwm_r=my_control.pwm_r+pwm_r;//
		 // 左轮PWM限幅
    if (my_control.pwm_l > SPEED_MAX) my_control.pwm_l = SPEED_MAX;
    if (my_control.pwm_l < SPEED_MIN) my_control.pwm_l = SPEED_MIN;
    
  // 右轮PWM限幅
    if (my_control.pwm_r > SPEED_MAX) my_control.pwm_r = SPEED_MAX;
    if (my_control.pwm_r < SPEED_MIN) my_control.pwm_r = SPEED_MIN;
	//	Motor_Left(pwm_l);
//		Motor_Right(pwm_r);
    // 实际电机输出（需根据硬件接口调整）
  
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     PID并行控制的速度环
  @param     int set_speed ,int speed,期望值，实际值,参数值
  @return    电机占空比SPEED_MIN~SPEED_MAX
  Sample     pwm_R= PID_R(set_speed_right,right_wheel);//pid控制电机转速
             pwm_L= PID_L(set_speed_left,left_wheel ); //pid控制电机转速
  @note      调参是门玄学
-------------------------------------------------------------------------------------------------------------------*/
void PID1_SPEED(float speed_l, float speed_r,int DesireSpeed)  {

    /******************** 2. 速度环（外环）输入合成 ********************/
    float target_speed_l = DesireSpeed ;
    float target_speed_r = DesireSpeed ;
      // 更新速度环历史误差（左轮）
   my_control.speed_lasterrL = my_control.speed_err;
   my_control.speed_lasterrR = my_control.speed_err; 
    // 更新右轮误差（若独立计算）
    /******************** 3. 速度环PI计算（增量式） ********************/
    // 左轮速度环
   my_control.speed_err= target_speed_l - speed_l;
//    int pwm_l = (int)(my_control.speed_lasterrL+ 
//              (my_control.I_SPEED * speed_err_l) + 
//              (my_control.P_SPEED * (speed_err_l - my_control.speed_lasterrL)));
    my_control.pwm_l+= (int)( (my_control.I_SPEED * my_control.speed_err) + 
              (my_control.P_SPEED * (my_control.speed_err - my_control.speed_lasterrL)));
    // 右轮速度环（独立计算误差）
    my_control.speed_err = target_speed_r - speed_r;
		//my_control.speed_lasterrR
    my_control.pwm_r+= (int)( (my_control.I_SPEED* my_control.speed_err) + 
              (my_control.P_SPEED * (my_control.speed_err - my_control.speed_lasterrR)));



    /******************** 4. PWM限幅及电机输出 ********************/
   
		
//		my_control.pwm_l=my_control.pwm_l+pwm_l;//
	//	my_control.pwm_r=my_control.pwm_r+pwm_r;//
		 // 左轮PWM限幅
    if (my_control.pwm_l > SPEED_MAX) my_control.pwm_l = SPEED_MAX;
    if (my_control.pwm_l < SPEED_MIN) my_control.pwm_l = SPEED_MIN;
    
  // 右轮PWM限幅
    if (my_control.pwm_r > SPEED_MAX) my_control.pwm_r = SPEED_MAX;
    if (my_control.pwm_r < SPEED_MIN) my_control.pwm_r = SPEED_MIN;
	//	Motor_Left(pwm_l);
//		Motor_Right(pwm_r);
    // 实际电机输出（需根据硬件接口调整）
  
}

void PID_SPEED(float speed_l, float speed_r, int DesireSpeed) {
    // 目标速度
    float target_speed = DesireSpeed;

    // 计算左右轮独立误差
    float err_l = target_speed - speed_l;
    float err_r = target_speed - speed_r;

    // 增量式 PID 计算（左轮）
    float delta_pwm_l = 
        my_control.P_SPEED * (err_l - my_control.speed_lasterrL) +  // P 项
        my_control.I_SPEED * err_l;                                 // I 项

    // 增量式 PID 计算（右轮）
    float delta_pwm_r = 
        my_control.P_SPEED * (err_r - my_control.speed_lasterrR) +  // P 项
        my_control.I_SPEED * err_r;                                 // I 项

    // 更新 PWM 输出（左轮）
    my_control.pwm_l += (int)delta_pwm_l;
		if (my_control.pwm_l > SPEED_MAX) my_control.pwm_l = SPEED_MAX;
    if (my_control.pwm_l < SPEED_MIN) my_control.pwm_l = SPEED_MIN;
     my_control.pwm_r += (int)delta_pwm_r;
  // 右轮PWM限幅
    if (my_control.pwm_r > SPEED_MAX) my_control.pwm_r = SPEED_MAX;
    if (my_control.pwm_r < SPEED_MIN) my_control.pwm_r = SPEED_MIN;
   
    // 更新 PWM 输出（右轮）



    // 保存历史误差（用于下一次计算）
    my_control.speed_lasterrL = err_l;
    my_control.speed_lasterrR = err_r;
}
void PID2_SPEED(float speed, int DesireSpeed) {
    // 目标速度
    float target_speed = DesireSpeed;

   float err=target_speed - speed;

    // 增量式 PID 计算（左轮）
    float delta_pwm = 
        my_control.P_SPEED * (err - my_control.speed_lasterrL) +  // P 项
        my_control.I_SPEED * err;                                 // I 项
                              // I 项

    // 更新 PWM 输出（左轮）
    my_control.pwm_l += (int)delta_pwm;
		if (my_control.pwm_l > SPEED_MAX) my_control.pwm_l = SPEED_MAX;
    if (my_control.pwm_l < SPEED_MIN) my_control.pwm_l = SPEED_MIN;
     my_control.pwm_r += (int)delta_pwm;
  // 右轮PWM限幅
    if (my_control.pwm_r > SPEED_MAX) my_control.pwm_r = SPEED_MAX;
    if (my_control.pwm_r < SPEED_MIN) my_control.pwm_r = SPEED_MIN;
   
    // 更新 PWM 输出（右轮）



    // 保存历史误差（用于下一次计算）
    my_control.speed_lasterrL = err;
    my_control.speed_lasterrR = err;
}
void pid_single_l(float speed, int DesireSpeed)
{
     float target_speed = DesireSpeed;

   float err=target_speed - speed;

    // 增量式 PID 计算（左轮）
    float delta_pwm = 
        my_control.P_SPEED * (err - my_control.speed_lasterrL) +  // P 项
        my_control.I_SPEED * err;                                 // I 项
                              // I 项

    // 更新 PWM 输出（左轮）
    my_control.pwm_l += (int)delta_pwm;
		if (my_control.pwm_l > SPEED_MAX) my_control.pwm_l = SPEED_MAX;
    if (my_control.pwm_l < SPEED_MIN) my_control.pwm_l = SPEED_MIN;
    // 更新 PWM 输出（右轮）



    // 保存历史误差（用于下一次计算）
    my_control.speed_lasterrL = err;
    my_control.speed_lasterrR = err;


}
void pid_single_r(float speed, int DesireSpeed)
{
     float target_speed = DesireSpeed;

   float err=target_speed - speed;

    // 增量式 PID 计算（左轮）
    float delta_pwm = 
        my_control.P_SPEED * (err - my_control.speed_lasterrL) +  // P 项
        my_control.I_SPEED * err;                                 // I 项
                              // I 项

     my_control.pwm_r += (int)delta_pwm;
  // 右轮PWM限幅
    if (my_control.pwm_r > SPEED_MAX) my_control.pwm_r = SPEED_MAX;
    if (my_control.pwm_r < SPEED_MIN) my_control.pwm_r = SPEED_MIN;
   
    // 更新 PWM 输出（右轮）



    // 保存历史误差（用于下一次计算）
    my_control.speed_lasterrL = err;
    my_control.speed_lasterrR = err;


}
float my_abs_float(float x) {
    return (x < 0) ? -x : x;
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     PID并行控制的方向环
  @param     int set_speed ,int speed,期望值，实际值,参数值
  @return    电机占空比SPEED_MIN~SPEED_MAX
  Sample     pwm_R= PID_R(set_speed_right,right_wheel);//pid控制电机转速
             pwm_L= PID_L(set_speed_left,left_wheel ); //pid控制电机转速
  @note      调参是门玄学
-------------------------------------------------------------------------------------------------------------------*/
void PID_DIR(float offset) {
   // PD计算（位置式）
	
//	else 
//		if (my_abs(my_control.err)>=15)
//	{
//	   my_control.P_DIRE=-35  ;
//		my_control.D_DIRE=0-my_order.add*0.1;
//		my_control.Speed_Right_Set=my_control.Speed_Left_Set-30;
//	}
//	else
//	{
//	   my_control.P_DIRE=-30;
//		my_control.D_DIRE=0;
//		my_control.Speed_Right_Set=my_control.Speed_Left_Set;
//	
//	}
//	if(my_control.err>20 || my_control.err<=-20)
//	{ 
//	     my_control.P_DIRE=-40;
//		
//	}
//	else
//	{
//	    my_control.P_DIRE=-20;
//	
//	}
	  my_control.steer_output =0;  
    my_control.steer_output = my_control.P_DIRE * my_control.err 
                 + my_control.D_DIRE * (my_control.err  - my_control.last_err);
    my_control.last_err  = my_control.err ; // 更新误差
   //输出限幅
    my_control.steer_output=my_control.steer_output*offset;
	 if(my_control.steer_output>DIFF_MAX )
	 {my_control.steer_output =DIFF_MAX ;}
	  if(my_control.steer_output<DIFF_MIN )
	 {my_control.steer_output =DIFF_MIN ;}
  
}






