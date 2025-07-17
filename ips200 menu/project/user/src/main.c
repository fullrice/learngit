/*********************************************************************************************************************
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
#include "isr.h"
// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

// 本例程是开源库移植用空工程

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
void all_init(void)
{
    //============================ 时钟与调试初始化 ============================//
    clock_init(SYSTEM_CLOCK_120M);      // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                       // 初始化默认 Debug UART
    
    //=============================== GPIO初始化 ===============================//
    // 电机方向控制
    gpio_init(DIR_L, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // GPIO 初始化为输出 默认上拉输出高
    gpio_init(DIR_R, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // GPIO 初始化为输出 默认上拉输出高
    
    // 按键输入
    gpio_init(E2, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_enter
    gpio_init(E3, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_return
    gpio_init(E4, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_down
    gpio_init(E5, GPI, GPIO_HIGH, GPI_PULL_UP);       // key_up

    //============================== PWM初始化 ================================//
    pwm_init(PWM_L, 17000, 0);         // PWM 通道初始化频率 17KHz 占空比初始为 0    
    pwm_init(PWM_R, 17000, 0);         // PWM 通道初始化频率 17KHz 占空比初始为 0

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
	    pit_ms_init(TIM6_PIT, 10);//
			pit_ms_init(TIM7_PIT, 5);//图像
	 		pit_ms_init(TIM2_PIT, 5);//速度方向，可以适当增加·
      interrupt_set_priority(TIM6_IRQn, 1);
			interrupt_set_priority(TIM7_IRQn, 2);
			interrupt_set_priority(TIM2_IRQn, 3);
    //  pit_ms_init(TIM2_PIT, 100);
    
    /* Flash操作（注释状态） */
    // flash_read_page(FLASH_SECTOR, FLASH_PAGE, &b, 1);
     key_init(5);
    //============================= 延时与完成 ===============================//
    system_delay_ms(300);             // 初始化延时
   // system_delay_ms(1000);            // 显示延时
}

float steer_output;         // 方向环输出（速度修正量）
#define MAX_DUTY            (50 )   
int8 duty = 0;
bool dir1 = true;

int main(void)
{
    all_init();
    // 此处编写用户代码 例如外设初始化代码等
    while(1)
    {
       
			//   island_show();
		  	Camera_show();
			//show_test();
        if(mt9v03x_finish_flag)
			 {
				  Threshold=My_Adapt_Threshold((uint8 *)mt9v03x_image,MT9V03X_W, MT9V03X_H);
				  Image_Binarization(Threshold);//图像二值化
			   // Longest_White_Column();
				  mt9v03x_finish_flag=0;//标志位清除，自行准备采集下一帧数据
				 }
			 else{}  
	//			  ips200_show_gray_image(0, 0, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
	  //     draw_mid_line();
   // 	   ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
			   Longest_White_Column();
//         	 if(my_island.island_state==3)  
//				 {
//					    Left_Add_Line(my_image.shortest_White_Column_Left[1],MT9V03X_H-my_image.white_line[my_image.shortest_White_Column_Left[1]]-10,40  ,MT9V03X_H-5);//x1是起点
////				    my_island.k=(float)((float)(MT9V03X_H-my_image.white_line[my_image.shortest_White_Column_Left[1]])/(float)(MT9V03X_W-20-my_image.shortest_White_Column_Left[1]));
////            K_Draw_Line(my_island.k,MT9V03X_W-30,MT9V03X_H-1,0);//记录下第一次上点出现时位置，针对这个环岛拉一条死线，入环
////            Longest_White_Column();//刷新边界数据
//				 }				 
				 my_control.err= err_sum_average(35,40);  //35 40
				 
			//	 	 island_detect();
				 
			//	 		my_control.err= err_sum_average(35,40);  //35 40
						system_delay_ms(1);
        // 此处编写需要循环执行的代码
    }
}
