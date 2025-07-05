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
int32 encoder1;
int32 encoder2;
float speed_mps1;
float speed_mps2; 
#define FLASH_SECTOR    127     // 使用最后一个扇区
#define FLASH_PAGE      0       // 使用第0页
void all_init(void)
{
  //  timer_init(TIM_1, TIMER_US);
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();     
	//  timer_init(TIM_3, TIMER_US);
	//  timer_start(TIM_3); 
	  gpio_init(DIR_L, GPO, GPIO_HIGH, GPO_PUSH_PULL);                            // GPIO 初始化为输出 默认上拉输出高
    pwm_init(PWM_L, 17000, 0);                                                  // PWM 通道初始化频率 17KHz 占空比初始为 0    
    gpio_init(DIR_R, GPO, GPIO_HIGH, GPO_PUSH_PULL);                            // GPIO 初始化为输出 默认上拉输出高
    pwm_init(PWM_R, 17000, 0);         
	 encoder_quad_init(TIM3_ENCODER, TIM3_ENCODER_CH1_B4, TIM3_ENCODER_CH2_B5);
	 encoder_quad_init(TIM4_ENCODER, TIM4_ENCODER_CH1_B6, TIM4_ENCODER_CH2_B7);	// PWM 通道初始化频率 17KHz 占空比初始为 0
	// flash_read_page(FLASH_SECTOR, FLASH_PAGE, &b, 1);
		system_delay_ms(300);	// 初始化默认 Debug UART
    gpio_init(E2, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_enter
    gpio_init(E3, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_return
    gpio_init(E4, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_down
    gpio_init(E5, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_up
    // 此处编写用户代码 例如外设初始化代码等
    	ips200_init(IPS200_TYPE_SPI);
  //	 pit_ms_init(TIM5_PIT, 10);//
//	 interrupt_set_priority(TIM5_IRQn, 1);
   	 pit_ms_init(TIM2_PIT, 100);
/*
			 while(1)
				{
						if(mt9v03x_init())
						{
								ips200_show_string(0, 16, "mt9v03x reinit.");
						}
						else
						{
								break;
						}
						system_delay_ms(500);                                                   // 短延时快速闪灯表示异常
				}
				*/
    ips200_show_string(0, 16, "init success.");
  	system_delay_ms(1000);  
    ips200_clear();
}



 //uint32   otsu_time = 0;
// uint32   otsu_time2 = 0;
int main(void)
{
	 
    all_init();  
	 
    // 此处编写用户代码 例如外设初始化代码等
	  //修改字体
    //ips200_set_font(IPS200_8X16_FONT);
    while(1)
    {
		     Motor_Right(1000);
					Motor_Left(1000);
			 menu_main();
        // 此处编写需要循环执行的代码
        
        // 此处编写需要循环执行的代码
    }
}
