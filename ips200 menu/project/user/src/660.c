#include "660.h"
#include "motor.h"
#include "menu.h"
#include "image.h"
#include "control.h"
#include <math.h>
#include "isr.h"
pdd my_pdd=
{
	.AD_DIRE=0.7, //+
	.D_DIRE=-1.5,   //-
	.P_DIRE=-170    ,//130  -190
	.steer_output=0,
	.open=1
};
// 150  0.8(ad) -1.5(d) -190(p)  29(fro)  40(p_spe) 1.5(d_spe)   1.5倍
#define FLASH_SECTION_INDEX       (127)                                         // 存储数据用的扇区 倒数第一个扇区
#define FLASH_PAGE_INDEX          (3)                                           // 存储数据用的页码 倒数第一个页码
// PDD???????
int16 sub_page = 1;       // ?????
int16 sub_arrow = 0;      // ???????
int16 sub_page_last;      // ??????
// 1. P_SPEED调整页面（步长±1）
void p_speed_adjust()
{
    lcd_showstr(0, 0, "Adjust P_SPEED");
    lcd_showstr(0, 30, "Value:");
    lcd_showfloat(80, 30, my_control.P_SPEED, 5, 1);  // 显示P_SPEED（保留1位小数）
    lcd_showstr(0, 60, "Up:+1  Down:-1");
    lcd_showstr(0, 80, "Return:Menu");
    
    if(gpio_get_level(key_up) == 0)
    {
        my_control.P_SPEED += 10;  // 步长+1
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_control.P_SPEED -= 10;  // 步长-1
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_return) == 0)
    {
        delay_ms(200);
        lcd_clear();
        sub_page = 1;  // 返回菜单主页面
    }
}

// 2. I_SPEED调整页面（步长±0.1）
void i_speed_adjust()
{
    lcd_showstr(0, 0, "Adjust I_SPEED");
    lcd_showstr(0, 30, "Value:");
    lcd_showfloat(80, 30, my_control.I_SPEED, 5, 3);  // 显示I_SPEED（保留3位小数）
    lcd_showstr(0, 60, "Up:+0.1  Down:-0.1");
    lcd_showstr(0, 80, "Return:Menu");
    
    if(gpio_get_level(key_up) == 0)
    {
        my_control.I_SPEED += 0.5;  // 步长+0.1
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_control.I_SPEED -= 0.5;  // 步长-0.1
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_return) == 0)
    {
        delay_ms(200);
        lcd_clear();
        sub_page = 1;  // 返回菜单主页面
    }
}
/**
 * @brief  保存菜单中的PDD参数到Flash
 * 只存储：AD_DIRE、D_DIRE、P_DIRE（my_pdd）和front（my_control）
 */
void menu_save(void)
{
    // 检查并擦除目标页（避免旧数据干扰）
    if(flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))
    {
        flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }
    flash_buffer_clear(); // 清空缓冲区

    // 按顺序存储菜单中的参数（使用连续索引，避免浪费空间）
    // 索引0：AD_DIRE（float类型）
    flash_union_buffer[0].float_type  = my_pdd.AD_DIRE;
    // 索引1：D_DIRE（float类型）
    flash_union_buffer[1].float_type  = my_pdd.D_DIRE;
    // 索引2：P_DIRE（float类型）
    flash_union_buffer[2].float_type  = my_pdd.P_DIRE;
    // 索引3：front（int16类型，使用int16_type成员）
    flash_union_buffer[3].int16_type  = my_control.front;
    flash_union_buffer[4].float_type  = my_control.P_SPEED;  // 保存P_SPEED
    flash_union_buffer[5].float_type  = my_control.I_SPEED;  // 保存I_SPEED
    // 写入Flash（只保存菜单相关参数）
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
}
void PDD_location(float offset)
{
 //   PPDD->ek =setvalue-actualvalue;
 //   PPDD->location_sum += PPDD->ek;//???????                         
    //out
	  my_pdd.steer_output=0;
	  my_pdd.steer_output=my_pdd.P_DIRE*my_control.err + my_pdd.D_DIRE*(my_control.err  - my_control.last_err) + my_pdd.AD_DIRE*imu660ra_gyro_x;   
 //   PPDD->out=PPDD->kp*PPDD->ek+PPDD->kp2*abs(PPDD->ek)*PPDD->ek+(PPDD->ek-PPDD->ek1)*PPDD->kd+GZ*PPDD->kd2;//?????
    my_control.last_err  = my_control.err ;
	  my_pdd.steer_output=my_pdd.steer_output*offset;
    //PID
	 if(my_pdd.steer_output>DIFF_MAX )
	 {my_pdd.steer_output =DIFF_MAX ;}
	  if(my_pdd.steer_output<DIFF_MIN )
	 {my_pdd.steer_output =DIFF_MIN ;}
  
}
/**
 * @brief  从Flash加载菜单中的PDD参数
 * 对应恢复：AD_DIRE、D_DIRE、P_DIRE（my_pdd）和front（my_control）
 */
void menu_load(void)
{
    // 从Flash读取数据到缓冲区
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);

    // 按存储顺序恢复参数到菜单变量
    // 索引0：恢复AD_DIRE
    my_pdd.AD_DIRE = flash_union_buffer[0].float_type;
    // 索引1：恢复D_DIRE
    my_pdd.D_DIRE = flash_union_buffer[1].float_type;
    // 索引2：恢复P_DIRE
    my_pdd.P_DIRE = flash_union_buffer[2].float_type;
    // 索引3：恢复front（int16类型）
    my_control.front = flash_union_buffer[3].int16_type;
    my_control.P_SPEED = flash_union_buffer[4].float_type;  // 加载P_SPEED
    my_control.I_SPEED = flash_union_buffer[5].float_type;  // 加载I_SPEED
    flash_buffer_clear(); // 清空缓冲区，避免残留数据
}
void Camera_pdd_show()
{    
		if(my_order.show==1)
		{
				ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0); 
				draw_mid_line();
						// 绘制赛道边界线（宽线版本）
				draw_boundary_lines_wide();
		}
    lcd_showstr(0,110,"P_DIRE");
    lcd_showfloat(100,110, my_pdd.P_DIRE, 5, 3);
    
    lcd_showstr(0,130,"AD_DIRE");
    lcd_showfloat(100,130, my_pdd.AD_DIRE, 5, 3);
    
//    lcd_showstr(0,150,"D_DIRE");
//    lcd_showfloat(100,150, my_pdd.D_DIRE, 5, 3);
     lcd_showstr(0,150,"right");
    lcd_showfloat(100,150, my_image.Right_Up_Find, 5, 3);
//    lcd_showstr(0,170,"steer_out");
//    lcd_showfloat(100,170, my_pdd.steer_output, 5, 3);
        lcd_showstr(0,170,"left");
    lcd_showfloat(100,170, my_image.Left_Up_Find, 5, 3);

	  lcd_showstr(0,190,"ERR");
    lcd_showint(100,190, my_control.err , 5);
	
	  lcd_showstr(0,210,"grox");
    lcd_showint(100,210,imu660ra_gyro_x , 5);
    
		lcd_showstr(0,230,"set_speed");  // ??????
    lcd_showint(100,230, my_control.Speed_Right_Set , 5);
		
		lcd_showstr(0,250,"front");  // ??????
    lcd_showint(100,250, my_control.front  , 5);
		 
		lcd_showstr(0,270,"stop");  // ??????
    lcd_showint(100,270, my_image.Search_Stop_Line  , 5);
		
				lcd_showstr(0, 290, "zebra");
    lcd_showint(100, 290       , my_order.zebra  , 5);
     // 新增显示P_SPEED和I_SPEED
//    lcd_showstr(0,290,"P_SPEED");  // 新增行（根据屏幕尺寸调整位置）
//    lcd_showfloat(100,290, my_control.P_SPEED, 5, 1);
//    lcd_showstr(0,310,"I_SPEED");
//    lcd_showfloat(100,310, my_control.I_SPEED, 5, 3);

    if(gpio_get_level(key_up)== 0)
    {
      //  my_pdd.AD_DIRE += 0.1;
		  	 my_pdd.P_DIRE += 10;
		//	my_pdd.D_DIRE+=0.1;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down)== 0)
    {
//        my_pdd.AD_DIRE -= 1;
//        delay_ms(200);
//        lcd_clear();
			   menu_load();
			   my_order.encorder_time=0;  // ???????
        my_order.count_2s=0;       // 2??????
			 my_order.count_1s=0;
        my_order.go=1;             // ????
        my_order.show=0;           // ??????
			my_order.zebra=0;
      //  my_menu.menu_open=0;       // ????
        delay_ms(200);             // ????
        lcd_clear();               // ????

    }
		  if(gpio_get_level(key_enter)== 0)
    {
			 // my_pdd.AD_DIRE += 0.1;
     //  my_pdd.P_DIRE += 10;
			my_control.front-=1;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_return  )== 0)
    {
		   //	 my_pdd.AD_DIRE -= 0.1;
   			//			my_pdd.D_DIRE-=0.1;
        			my_control.front+=1;

        delay_ms(200);
        lcd_clear();
    }
}
void sub_key_action()
{
    if(gpio_get_level(key_up) == 0)
    {
        delay_ms(300);
        lcd_clear();
        sub_arrow -= 20;
    }
    if(gpio_get_level(key_down) == 0)
    {
        delay_ms(300);
        lcd_clear();
        sub_arrow += 20;
    }
    
    // 新增选项后，箭头范围调整为0~160（共9个选项，步长20）
    switch(sub_page)
    {
        case 1:  
            if(sub_arrow < 0) sub_arrow = 160;    // 向上循环到最后一个选项（Back）
            else if(sub_arrow > 160) sub_arrow = 0; // 向下循环到第一个选项
            break;
        default:  
            sub_arrow = 0;
            break;
    }
}
void pdd_sub_menu_main_page()
{
    sub_page_last = sub_page;
    sub_page = 1;
    if(sub_page != sub_page_last)
    {
        sub_arrow = 0;
    }
    
    sub_key_action();
    
    // 显示菜单选项（新增P_SPEED和I_SPEED选项）
    lcd_showstr(0, sub_arrow, "->");
    lcd_showstr(20, 0, "AD_DIRE");
    lcd_showfloat(120, 0, my_pdd.AD_DIRE, 5, 3);
    lcd_showstr(20, 20, "D_DIRE");
    lcd_showfloat(120, 20, my_pdd.D_DIRE, 5, 3);
    lcd_showstr(20, 40, "P_DIRE");
    lcd_showfloat(120, 40, my_pdd.P_DIRE, 5, 3);
    lcd_showstr(20, 60, "front");
    lcd_showint(120, 60, my_control.front, 5);
    // 新增选项：P_SPEED（位置80）、I_SPEED（位置100）
    lcd_showstr(20, 80, "P_SPEED");
    lcd_showfloat(120, 80, my_control.P_SPEED, 5, 1);  // 显示P_SPEED
    lcd_showstr(20, 100, "I_SPEED");
    lcd_showfloat(120, 100, my_control.I_SPEED, 5, 3); // 显示I_SPEED
    // 原有选项后移（保持Save/Load/Back）
    lcd_showstr(20, 120, "Save Params");
    lcd_showstr(20, 140, "Load Params");
    lcd_showstr(20, 160, "Back");
//		lcd_showstr(20, 180, "front_err");
//    lcd_showint(120,180       , my_control.front_err  , 5);
    		lcd_showstr(20, 180, "zebra");
    lcd_showint(120,180       , my_order.zebra  , 5);

    // 回车键逻辑（新增P_SPEED和I_SPEED的调整入口）
    if(gpio_get_level(key_enter) == 0)
    {
        delay_ms(300);
        lcd_clear();
        switch(sub_arrow + 20)
        {
            case 20:  sub_page = 2; break;   // AD_DIRE
            case 40:  sub_page = 3; break;   // D_DIRE
            case 60:  sub_page = 4; break;   // P_DIRE
            case 80:  sub_page = 5; break;   // front
            case 100: sub_page = 6; break;   // 新增：P_SPEED调整
            case 120: sub_page = 7; break;   // 新增：I_SPEED调整
            case 140:                        // Save
                menu_save();
                lcd_showstr(50, 80, "Params Saved!");
                delay_ms(1000);
                lcd_clear();
                sub_page = 1;
                break;
            case 160:                        // Load
                menu_load();
                lcd_showstr(50, 80, "Params Loaded!");
                delay_ms(1000);
                lcd_clear();
                sub_page = 1;
                break;
            case 180: sub_page = 0; break;   // Back
        }
        sub_arrow = 0;
    }
}
// AD_DIRE adjustment page
void pdd_ad_dire_adjust()
{
    lcd_showstr(0, 0, "Adjust AD_DIRE");
    lcd_showstr(0, 30, "Value:");
    lcd_showfloat(80, 30, my_pdd.AD_DIRE, 5, 3);
    lcd_showstr(0, 60, "Up:+  Down:-");
    lcd_showstr(0, 80, "Return:Menu");
    
    // Up key increase
    if(gpio_get_level(key_up) == 0)
    {
        my_pdd.AD_DIRE += 0.1;
        delay_ms(200);
        lcd_clear();
    }
    
    // Down key decrease
    if(gpio_get_level(key_down) == 0)
    {
        my_pdd.AD_DIRE -= 0.1;
        delay_ms(200);
        lcd_clear();
    }
    
    // Return key
    if(gpio_get_level(key_return) == 0)
    {
        delay_ms(200);
        lcd_clear();
        sub_page = 1;  // Return to submenu main page
    }
}

// D_DIRE adjustment page
void pdd_d_dire_adjust()
{
    lcd_showstr(0, 0, "Adjust D_DIRE");
    lcd_showstr(0, 30, "Value:");
    lcd_showfloat(80, 30, my_pdd.D_DIRE, 5, 3);
    lcd_showstr(0, 60, "Up:+  Down:-");
    lcd_showstr(0, 80, "Return:Menu");
    
    if(gpio_get_level(key_up) == 0)
    {
        my_pdd.D_DIRE += 0.5;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_pdd.D_DIRE -= 0.5   ;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_return) == 0)
    {
        delay_ms(200);
        lcd_clear();
        sub_page = 1;
    }
}

// P_DIRE adjustment page
void pdd_p_dire_adjust()
{
    lcd_showstr(0, 0, "Adjust P_DIRE");
    lcd_showstr(0, 30, "Value:");
    lcd_showfloat(80, 30, my_pdd.P_DIRE, 5, 3);
    lcd_showstr(0, 60, "Up:+  Down:-");
    lcd_showstr(0, 80, "Return:Menu");
    
    if(gpio_get_level(key_up) == 0)
    {
        my_pdd.P_DIRE += 5;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_pdd.P_DIRE -= 5;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_return) == 0)
    {
        delay_ms(200);
        lcd_clear();
        sub_page = 1;
    }
}

// front adjustment page
void front_adjust()
{
    lcd_showstr(0, 0, "Adjust front");
    lcd_showstr(0, 30, "Value:");
    lcd_showint(80, 30,my_control.front, 5);
    lcd_showstr(0, 60, "Up:+  Down:-");
    lcd_showstr(0, 80, "Return:Menu");
    
    if(gpio_get_level(key_up) == 0)
    {
			//  my_order.add+=1;
        my_control.front += 1;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_control.front -= 1;
		// my_order.add-=1;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_return) == 0)
    {
        delay_ms(200);
        lcd_clear();
        sub_page = 1;
    }
}

void pdd_sub_menu_main()
{
    if(my_pdd.open==1)
    {
        menu_load();
        my_pdd.open++;
    }
    
    if(sub_page != 0)
    {
        switch(sub_page)
        {
              case 1: pdd_sub_menu_main_page(); break;
            case 2: pdd_ad_dire_adjust(); break;
            case 3: pdd_d_dire_adjust(); break;
            case 4: pdd_p_dire_adjust(); break;
            case 5: front_adjust(); break;
            case 6: p_speed_adjust(); break;   // 新增：P_SPEED调整页面
            case 7: i_speed_adjust(); break;   // 新增：I_SPEED调整页面
            case 8: Camera_pdd_show();break;   // 保持原有Camera页面（若需要）
        }
    }
    else
    {
        Camera_pdd_show();
    }
}

// Function to start PDD submenu
void start_pdd_sub_menu()
{
    sub_page = 1;    // Reset submenu page
    sub_arrow = 0;   // Reset arrow position
    lcd_clear();     // Clear screen
}
