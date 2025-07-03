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
#define FLASH_SECTOR    127     // 使用最后一个扇区
#define FLASH_PAGE      0       // 使用第0页
void all_init(void)
{
  //  timer_init(TIM_1, TIMER_US);
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();     
	timer_init(TIM_3, TIMER_US);
	timer_start(TIM_3); 
	 flash_read_page(FLASH_SECTOR, FLASH_PAGE, &b, 1);
		system_delay_ms(300);	// 初始化默认 Debug UART
    gpio_init(E2, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_enter
    gpio_init(E3, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_return
    gpio_init(E4, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_down
    gpio_init(E5, GPI, GPIO_HIGH, GPI_PULL_UP);  // key_up
    // 此处编写用户代码 例如外设初始化代码等
    	ips200_init(IPS200_TYPE_SPI);
//	 pit_ms_init(TIM2_PIT, 100);
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
			memcpy(image_copy, mt9v03x_image, MT9V03X_H*MT9V03X_W);
		 //	ips200_show_gray_image(0, 0, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
		//	 ips200_show_gray_image(0, 200, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
		//	 mt9v03x_finish_flag=0;		
			
			 if(mt9v03x_finish_flag)
			 {
				  uint32 start_time =timer_get (TIM_3);
			    Threshold=My_Adapt_Threshold((uint8 *)mt9v03x_image,MT9V03X_W, MT9V03X_H);
				  Image_Binarization(Threshold);//图像二值化
				  uint32 otsu_time = timer_get (TIM_3) - start_time;
				  uint32 start_time2 =timer_get (TIM_3);
				  lq_sobelAutoThreshold(image_copy,image_copy_two_value);
					uint32 otsu_time2 = timer_get (TIM_3) - start_time;
				 ips200_show_uint (0, 300, otsu_time, 5);
			   ips200_show_uint (60, 300, otsu_time2, 5);
          mt9v03x_finish_flag=0;//标志位清除，自行准备采集下一帧数据
			 }
			 else{}
			/*
				 if(Threshold>=160)//guopu
				 {
					  uint16 temp;
					  temp=512+(160-Threshold)*20;
				     mt9v03x_set_exposure_time(temp);
				 }
				*/ 
		 //  if(mt9v03x_finish_flag)
			// {
			//	 Threshold=Distributed_Otsu_Threshold((uint8 *)image_copy, MT9V03X_W, MT9V03X_H);
			   // Threshold=My_Adapt_Threshold((uint8 *)image_copy,MT9V03X_W, MT9V03X_H);
				 // Image_Binarization2(image_copy,image_copy_two_value, MT9V03X_W,MT9V03X_H, Threshold);
				// lq_sobelAutoThreshold(image_copy,image_copy_two_value);
				//  Partitioned_Otsu_Binarization(image_copy,image_copy_two_value, MT9V03X_W,MT9V03X_H);
      //    mt9v03x_finish_flag=0;//标志位清除，自行准备采集下一帧数据
		//	 }
		//	 else{}

        
    
			  delay_ms(20); 
			// ips200_show_uint (0, 300, otsu_time, 5);
		//	 ips200_show_uint (30, 300, otsu_time2, 5);
		 	  ips200_show_gray_image(0, 0, (const uint8 *)image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
				 ips200_show_gray_image(0, 180, (const uint8 *)image_copy_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
				//  mt9v03x_finish_flag=0;
		//	 menu_main();
        // 此处编写需要循环执行的代码
        
        // 此处编写需要循环执行的代码
    }
}
