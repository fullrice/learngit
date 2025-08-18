#include "motor.h"
#include "menu.h"
#include "image.h"
#include "control.h"
#include "obstacle.h"
#include <math.h>
Obstacle my_obstacle={0};/**
* @brief 路障识别函数
*/
void obstacle_show()//22
{
 // ips200_show_gray_image(0, 0, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
	if(my_order.show==1)
	{
	    ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0); 
			draw_mid_line();
      draw_boundary_lines_wide();
	}		


	   lcd_showstr(0,90,"current_width");
  	lcd_showint(150,90,my_image.Road_Wide[30], 5);//列号
		//lcd_showstr(0,110,"left_start");
	//lcd_showint(100,110, my_image.Boundry_Start_Left, 5);
//	lcd_showstr(0,110,"point");
//	lcd_showint(100,110, my_island.right_down_line[0], 5);//
			lcd_showstr(0,110,"standard_width");  
	  lcd_showint(150,110,Standard_Road_Wide[30], 5);//角点的行数

   			lcd_showstr(0,130,"narrrow_count");
	lcd_showint(150,130,my_obstacle.narrow_count,5);
 
	 lcd_showstr(0,150,"err");
	lcd_showint(150,150,my_control.err , 5);
  
////	lcd_showstr(0,190,"right_lost");
////	lcd_showint(100,190, my_image.Right_Lost_Counter, 5);
//	lcd_showstr(0,190,"right_down");
//	lcd_showint(100,190, my_image.Right_Down_Find, 5);
//	lcd_showstr(0,210,"left_lost");
//	lcd_showint(100,210, my_image.Left_Lost_Counter, 5);
//		lcd_showstr(0,230,"sear_lost");
//	lcd_showint(100,230, my_image.Search_Stop_Line, 5);
////	lcd_showstr(0,230,"short_col");
////	lcd_showint(100,230, my_image.white_line[my_image.shortest_White_Column_Left[1]], 5);
//	lcd_showstr(0,250,"island");
//	lcd_showint(100,250,my_island.island_state, 5);
//	lcd_showstr(0,270,"mon_line");
//	lcd_showint(100,270,my_island.monotonicity_change_line_left[0], 5);
////	lcd_showstr(0,270,"angle");
////	lcd_showint(100,270,my_island.gyro_x_angle, 5);

	lcd_showstr(0,170,"state");
	lcd_showint(150,170,my_obstacle.state, 5);
//  lcd_showstr(0,290,"left_start");
//	lcd_showint(100,290,my_image.Boundry_Start_Left, 5);

 if(gpio_get_level(key_up)== 0)
    {
			  delay_ms(200);
        lcd_clear();
    }
		 if(gpio_get_level(key_down)== 0)
    {

			  delay_ms(200);
        lcd_clear();
    }
	 if(gpio_get_level(key_enter)== 0)
    {
   		
        delay_ms(200);
        lcd_clear();
        
    }
	}


void obstacle_detect(void)
{
	  my_obstacle.narrow_count=0;
	  my_obstacle.current_width=0;
	  my_obstacle.standard_width=0;
    if(my_image.continuity_change_flag_right)
		{
     my_obstacle.direction = 2;
		}
    else if(my_image.continuity_change_flag_left)
		{
     my_obstacle.direction = 1;
		}
    // 只在直道且无其他元素时检测路障
    if(my_island.island_state == 0 && my_order.cross == 0 && my_order.ramp == 0)
    {
			 for(int i=40;i>=10;i--)
			{
			   my_obstacle.current_width= my_image.Road_Wide[i];
				 my_obstacle.standard_width=Standard_Road_Wide[i];
				 if(my_obstacle.current_width <= my_obstacle.standard_width*0.8)
				 {
					 my_obstacle.narrow_count++;
				 }
					 
			}
			if(my_obstacle.narrow_count>=5)
			{
			  my_obstacle.state=1;
			}
			else
			{
			  my_obstacle.state=0;
			}
		}
		if(my_obstacle.state==1)
		{
		  obstacle_avoid_process();
		}
}

/**
* @brief 路障处理核心函数
*/
void obstacle_detect_process(void)
{
    for(int i = MT9V03X_H - 3; i > MT9V03X_H - my_image.Search_Stop_Line + 10; i--)
    {
        float current_width = my_image.Road_Wide[i];
        float standard_width = Standard_Road_Wide[i];
        
        switch(my_obstacle.state)
        {
            case 0: // 初始检测
                if(current_width >= standard_width * 0.85f)
                {
                    my_obstacle.flag1 = 1;
                    if(++my_obstacle.counter1 >= 15)
                    {
                        my_obstacle.state = 1;
                        my_obstacle.counter1 = 0;
                    }
                }
                else
                {
                    my_obstacle.flag1 = 0;
                    my_obstacle.counter1 = 0;
                }
                break;
                
            case 1: // 发现路障
                if(current_width <= standard_width * 0.9f)
                {
                    if(++my_obstacle.narrow_count >= 10)
                    {
                        my_obstacle.state = 2;
                        my_obstacle.step = 1;
                        my_obstacle.narrow_count = 0;
                        my_obstacle.encoder_sum = 0;
                        
                        // 确定避障方向
                        if(my_image.continuity_change_flag_right)
                            my_obstacle.direction = 1;
                        else if(my_image.continuity_change_flag_left)
                            my_obstacle.direction = 2;
                    }
                }
                break;
                
            case 2: // 避障处理
                obstacle_avoid_process();
                break;
                
            case 3: // 回归处理
                if(current_width >= standard_width * 0.95f)
                {
                    if(++my_obstacle.counter3 >= 8)
                        reset_obstacle_detect();
                }
                else
                {
                    my_obstacle.counter3 = 0;
                }
                break;
        }
        
        if(my_obstacle.state > 0) break;
    }
    
    my_obstacle.flag = (my_obstacle.state > 0) ? 1 : 0;
}

/**
* @brief 避障动作处理
*/
void obstacle_avoid_process(void)
{
    // 边界偏移避障
    for(int i = MT9V03X_H - 1; i >= MT9V03X_H - my_image.Search_Stop_Line; i--)
    {
        if(my_obstacle.direction == 1) // 左避障
        {
            my_image.Left_Line[i] = (my_image.Left_Line[i] > 20) ? 
                                  (my_image.Left_Line[i] + 15) : 5;
        }
        else if(my_obstacle.direction == 2)// 右避障
        {
            my_image.Right_Line[i] = (my_image.Right_Line[i] < MT9V03X_W - 20) ? 
                                   (my_image.Right_Line[i] - 15) : (MT9V03X_W - 5);
        }
    }
    
//    // 编码器累计判断
//    my_obstacle.encoder_sum += my_abs(my_control.encoderl) + my_abs(my_control.encoderr);
//    
//    if(my_obstacle.encoder_sum > 14000 || my_image.Search_Stop_Line > 110)
//    {
//        my_obstacle.state = 2;
//        my_obstacle.encoder_sum = 0;
//    }
}

/**
* @brief 重置路障状态
*/
void reset_obstacle_detect(void)
{
    memset(&my_obstacle, 0, sizeof(my_obstacle));
}