/***************** 吧****************************************************************************************************
* MM32F327X-G8P Opensourec Library 即（MM32F327X-G8P 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
* 
* 本文件是 MM32F327X-G8P 开源库的一部分
* 
* MM32F327X-G8P 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
* 
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
* 
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
* 
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
* 
* 文件名称          main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 8.32.4 or MDK 5.37
* 适用平台          MM32F327X_G8P
* 店铺链接          https://seekfree.taobao.com/
* 
* 修改记录
* 日期              作者                备注
* 2022-08-10        Teternal            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "menu.h"
#include "motor.h"
#include "control.h"
#include "image.h"
#include "660.h"
#include "obstacle.h"

#include "isr.h"
// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

// 本例程是开源库移植用空工程
/*
要去记录
修改代码
修改代码2  





*/
// **************************** 代码区域 ****************************
uint32 a=20,b=0;
uint8 image_copy[MT9V03X_H][MT9V03X_W];
uint8 image_copy_two_value[MT9V03X_H][MT9V03X_W];//二值化后的原数组
int Threshold=0;
//int count_2s=0;
int count_10s=0;
int32 encoder1;
int32 encoder2;
float speed_mps1;
float speed_mps2; 
#define FLASH_SECTOR    127     // 使用最后一个扇区
#define FLASH_PAGE      0       // 使用第0页
#define UART_INDEX              (DEBUG_UART_INDEX   )                           // 默认 UART_1
#define UART_BAUDRATE           (DEBUG_UART_BAUDRATE)                           // 默认 115200
#define UART_TX_PIN             (DEBUG_UART_TX_PIN  )                           // 默认 UART1_TX_A9
#define UART_RX_PIN             (DEBUG_UART_RX_PIN  )                           // 默认 UART1_RX_A10

#define UART_PRIORITY           (UART1_IRQn)                                    // 对应串口中断的中断编号 在 mm32f3277gx.h 头文件中查看 IRQn_Type 枚举体
uint8 uart_get_data[64];                                                        // 串口接收数据缓冲区
uint8 fifo_get_data[64];                                                        // fifo 输出读出缓冲区

uint8 get_data = 0;                                                             // 接收数据变量
uint32 fifo_data_count = 0;                                                     // fifo 数据个数

fifo_struct uart_data_fifo;
void all_init(void)
{
    //============================ 时钟与调试初始化 ============================//
    clock_init(SYSTEM_CLOCK_120M);      // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                       // 初始化默认 Debug UART
    
    //=============================== GPIO初始化 ===============================//
    // 电机方向控制
    gpio_init(DIR_L, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // GPIO 初始化为输出 默认上拉输出高
    gpio_init(DIR_R, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // GPIO 初始化为输出 默认上拉输出高
    gpio_init(BEEP, GPO, GPIO_LOW, GPO_PUSH_PULL);
    // 按键输入
    gpio_init(E2, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_enter
    gpio_init(E3, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_return
    gpio_init(E4, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_down
    gpio_init(E5, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_up

    //============================== PWM初始化 ================================//
    pwm_init(PWM_L, 17000, 0);         // PWM 通道初始化频率 17KHz 占空比初始为 0    
    pwm_init(PWM_R, 17000, 0);         // PWM 通道初始化频率 17KHz 占空比初始为 0
    imu660ra_init();
    //============================= 编码器初始化 ==============================//
    /* 编码器接口 */
    encoder_quad_init(TIM3_ENCODER, TIM3_ENCODER_CH1_B4, TIM3_ENCODER_CH2_B5);
    encoder_quad_init(TIM4_ENCODER, TIM4_ENCODER_CH1_B6, TIM4_ENCODER_CH2_B7);
    //  timer_init(TIM_3, TIMER_US);
    //  timer_start(TIM_3); 

    //============================ 外设模块初始化 =============================//
    /* 显示屏初始化 */
    ips200_init(IPS200_TYPE_SPI);
    ips200_show_string(0, 16, "init success.");
    ips200_clear();
    
    /* 摄像头初始化 */
    mt9v03x_init();
   
    /* 定时器初始化（注释状态） */
    //  timer_init(TIM_1, TIMER_US);
  //    pit_ms_init(TIM5_PIT, 10);//
	    /*中断*/			
	    pit_ms_init(TIM6_PIT, 1    );//陀螺仪
			pit_ms_init(TIM7_PIT, 10  );//标志位判断   //4
	 		pit_ms_init(TIM2_PIT, 5  			);//速度方向
      interrupt_set_priority(TIM6_IRQn, 3);
			interrupt_set_priority(TIM7_IRQn, 2);
			interrupt_set_priority(TIM2_IRQn, 1);
    //  pit_ms_init(TIM2_PIT, 100);
    
    /* Flash操作（注释状态） */
    // flash_read_page(FLASH_SECTOR, FLASH_PAGE, &b, 1);
     key_init(5);
    //============================= 延时与完成 ===============================//
    system_delay_ms(300);             // 初始化延时
   // system_delay_ms(1000);            // 显示延时
}

/* 全局变量定义 */
float steer_output;         // 方向环输出（用于速度修正的控制量，可能用于差速控制）
#define MAX_DUTY            (50)   // PWM占空比最大值（限制电机最大输出）
int8 duty = 0;              // 当前PWM占空比（控制电机转速）
bool dir1 = true;           // 电机方向标志位（true=正转，false=反转）

int main(void)  
{
    /* 系统初始化（硬件外设、摄像头、电机、传感器等） */
    all_init();
    
    // 主循环（控制核心）
    while(1)
    {
        /* 图像处理与显示模块（调试时可启用） */
     //      island_show();                    // 调试用：显示环岛检测信息（当前被注释）
		//		  ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0); 

         //     Camera_show();                    // 调试用：显示摄像头原始图像（当前被注释）
        // menu_sub();                       // 调试用：显示子菜单（当前被注释）
        // menu_main();                     // 调试用：显示主菜单（当前被注释）
        // show_test();                      // 调试用：测试显示（当前被注释）
        /* 图像采  集与处理 */
        if(mt9v03x_finish_flag)             // 检查摄像头是否完成一帧图像采集
        {
//            // 计算自适应阈值（动态调整二值化阈值）
            Threshold = My_Adapt_Threshold((uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H);
            // 图像二值化处理（将灰度图转为黑白图）
            Image_Binarization(Threshold);//加大阈值来增加对应的图象值更让偏向黑色
            image_filter(my_image.image_two_value);
     //          Multi_Peak_Threshold_MT9V03X((uint8 *)mt9v03x_image, &my_image);
            // 清除图像采集完成标志（准备下一帧采集）
            mt9v03x_finish_flag = 0;
        }
	//		  ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0); 
				 
      //   island_show();   
        /* 赛道中线提取 */
        Longest_White_Column();              // 通过检测最长白列提取赛道中线
		//		 Find_Down_Point( MT9V03X_H-1, 0 );
			//	   Find_Up_Point( MT9V03X_H-20, 20 );	
//				if(my_island.island_state==5)
//				{
//				      my_island.right_down_line[0]= my_image.Right_Down_Find;//40
//              my_island.right_down_line[1]= my_image.Right_Line[my_island.right_down_line[0]];//120
//							my_island.k = ((float)(my_island.right_down_line[1] - 80)) / 
//														((float)(my_island.right_down_line[0] - 20));

//							// 2. 从右下角点向上补线
//							for(int i = my_island.right_down_line[0]; i > 5; i--) {
//									// 正确的线性插值公式
//									my_image.Right_Line[i] = my_island.right_down_line[1] + 
//																					my_island.k * (i - my_island.right_down_line[0]);
//									
//									// 边界保护
//									my_image.Right_Line[i] = (my_image.Right_Line[i] <= 1) ? 1 : 
//																					((my_image.Right_Line[i] >= MT9V03X_W - 2) ? MT9V03X_W - 2 : my_image.Right_Line[i]);
//							}
////					          xieji_right(my_island.right_down_line[1], 0, my_island.right_down_line[0],   0);
////					Right_Add_Line(my_island.right_down_line[1], 0, my_island.right_down_line[0],   0);
////					draw_boundary_lines_wide();
//	//			    xieji_right(90, my_island.right_down_line[1], 10,my_island.right_down_line[0]);

//				}
//										if(my_island.island_state == 3)
//				{
//						// 1. 获取左上拐点坐标（图像坐标系）
//						my_island.Left_Up_Guai[0] = my_image.Left_Up_Find; // 行号y（向下增大）
//						my_island.Left_Up_Guai[1] = my_image.Left_Line[my_island.Left_Up_Guai[0]]; // 列号x（向右增大）
//						
//						// 2. 设置目标点（右下角）
//						int target_col = 179;   // 列号x（更靠右）
//						int target_row = 79;    // 行号y（更靠下）
//						
//						// 3. 计算正确的斜率（Δx/Δy）
//						if(my_island.Left_Up_Guai[0] != target_row) {
//								// 注意：行号差是target_row - Left_Up_Guai[0]（向下为正）
//								my_island.k = ((float)(target_col - my_island.Left_Up_Guai[1])) / 
//														 ((float)(target_row - my_island.Left_Up_Guai[0]));
//						} else {
//								my_island.k = 0; // 水平线处理
//						}

//						// 4. 从拐点向下补线到目标行（注意循环方向改变）
//						for(int i = my_island.Left_Up_Guai[0]; i <= target_row; i++) 
//						{
//								// 线性插值公式：x = x1 + k*(y-y1)
//								int x = my_island.Left_Up_Guai[1] + 
//											 (int)(my_island.k * (i - my_island.Left_Up_Guai[0]));
//								
//								// 边界保护
//								if(x <= 1) {
//										x = 1;
//								} 
//								else if(x >= MT9V03X_W - 2) {
//										x = MT9V03X_W - 2;
//								}
//								
//								my_image.Left_Line[i] = x;
//						}
//				}
				Cross_Detect();
			//	island_detect_left();   
			  island_detect();
        if(my_island.detect==1)
				{
				  island_detect_left();
				}
				else if (my_island.detect==2)
				{
				  island_detect_right();
				}
				// 
		  	Ramp_Detect(); 
			  obstacle_detect();  

				if(my_order.cross==1 )
	   			{
				//   Lengthen_Left_Boundry(0,int end);
					 Lengthen_Left_Boundry(my_image.Left_Up_Find-1,MT9V03X_H-10);
           Lengthen_Right_Boundry(my_image.Right_Up_Find-1,MT9V03X_H-10);				
				}
  //        /* 环岛特殊处理（状态5：出环阶段） */
//        if(my_island.island_state == 5)      // 检查是否处于环岛状态5
//        {
//            // 绘制出环辅助线（基于左边界下降点坐标）
//            xieji(my_island.left_down_line[1], 70, my_island.left_down_line[0], 20);
//        }
////       if(my_control.last_err>=5)
////			 {  
////			    my_control.front=27;
////			 
////			 }
////			 else  
////			 {
////				 my_control.front=30;

////			 }
//        /* 控制误差计算 */
//        // 计算30-36行图像的平均误差（用于方向控制）

							// 直道条件判断
//							if ((my_control.err <= 8 || my_control.err >= -8)&& my_image.Search_Stop_Line>=75) 
//							{
//								 my_control.front=25;
//							} 
//							else // 弯道条件
//							{
//								 my_control.front=20  ;
//							}
          my_control.err = err_sum_average(my_control.front,my_control.front+7); 
//          if(my_order.cross == 1)
//					{
//					   if(my_control.err>10)//左转
//						 {
//						    my_control.err=-30;
//						 }
//						 else if(my_control.err<=-10)//右转
//						 {
//						    my_control.err=30;
//						  
//						 }
//									
//					}						
		//			         Camera_pdd_show();
	      //	  printf("\r\n %d,%d",my_control.Speed_Right_Set,(my_control.encoderl+my_control.encoderr)/2);
                      pdd_sub_menu_main();
								//			obstacle_show();									
			  //	             island_show();  
		// show_test();
			//	 Camera_pdd_show();
////					lcd_showstr(0,190,"ERR");
////          lcd_showint(100,190, my_control.err , 5);

	//	  my_control.err = err_sum_average(my_control.front,my_control.front+3 );   //33 36  39 42
//       //    lcd_showint(100,130, my_order.go , 5);
//   
  //        my_control.err =Err_Sum();
//   //    my_control.last_err=my_control.err;
//        /* 环岛强制纠偏（状态4/5时覆盖误差） */
//        if(my_island.island_state == 5 || my_island.island_state == 4)  
//        {
//            my_control.err = 30;  // 强制设定固定偏差，使车辆保持环岛运动
//        }
   
        /* 系统延时（控制循环频率） */
        system_delay_ms(1);  // 保持1ms控制周期（确保实时性）
    }
}