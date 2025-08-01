#include "660.h"
#include "motor.h"
#include "menu.h"
#include "image.h"
#include "control.h"
#include <math.h>
#include "isr.h"
pdd my_pdd=
{
	.AD_DIRE=0.8,
	.D_DIRE=-0.5,
	.P_DIRE=-160    ,
	.steer_output=0,
	.open=1
};
#define FLASH_SECTION_INDEX       (127)                                         // 存储数据用的扇区 倒数第一个扇区
#define FLASH_PAGE_INDEX          (3)                                           // 存储数据用的页码 倒数第一个页码
// PDD???????
int16 sub_page = 1;       // ?????
int16 sub_arrow = 0;      // ???????
int16 sub_page_last;      // ??????
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

    flash_buffer_clear(); // 清空缓冲区，避免残留数据
}
void Camera_pdd_show()
{
    ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0); 

    lcd_showstr(0,110,"P_DIRE");
    lcd_showfloat(100,110, my_pdd.P_DIRE, 5, 3);
    
    lcd_showstr(0,130,"AD_DIRE");
    lcd_showfloat(100,130, my_pdd.AD_DIRE, 5, 3);
    
    lcd_showstr(0,150,"D_DIRE");
    lcd_showfloat(100,150, my_pdd.D_DIRE, 5, 3);
    
    lcd_showstr(0,170,"steer_out");
    lcd_showfloat(100,170, my_pdd.steer_output, 5, 3);

	  lcd_showstr(0,190,"ERR");
    lcd_showint(100,190, my_control.err , 5);
	
	  lcd_showstr(0,210,"grox");
    lcd_showint(100,210,imu660ra_gyro_x , 5);
    
		lcd_showstr(0,230,"set_speed");  // ??????
    lcd_showint(100,230, my_control.Speed_Left_Set , 5);
		
		lcd_showstr(0,250,"front");  // ??????
    lcd_showint(100,250, my_control.front  , 5);
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
        my_order.go=1;             // ????
        my_order.show=0;           // ??????
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
// PDD submenu key handling
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
    
    // 主页面共6个选项（0~120，步长20），箭头循环范围调整
    switch(sub_page)
    {
        case 1:  
            if(sub_arrow < 0) sub_arrow = 120;   // 向上循环到最后一个选项
            else if(sub_arrow > 120) sub_arrow = 0; // 向下循环到第一个选项
            break;
        default:  
            sub_arrow = 0; // 调节页面无需箭头移动
            break;
    }
}

// PDD submenu main page
void pdd_sub_menu_main_page()
{
    sub_page_last = sub_page;
    sub_page = 1;
    if(sub_page != sub_page_last)
    {
        sub_arrow = 0;
    }
    
    sub_key_action();
    
    // 显示菜单选项（明确对应存储的参数）
    lcd_showstr(0, sub_arrow, "->");
    lcd_showstr(20, 0, "AD_DIRE");
    lcd_showfloat(120, 0, my_pdd.AD_DIRE, 5, 3);
    lcd_showstr(20, 20, "D_DIRE");
    lcd_showfloat(120, 20, my_pdd.D_DIRE, 5, 3);
    lcd_showstr(20, 40, "P_DIRE");
    lcd_showfloat(120, 40, my_pdd.P_DIRE, 5, 3);
    lcd_showstr(20, 60, "front");
    lcd_showint(120, 60, my_control.front, 5);
    lcd_showstr(20, 80, "Save Params");  // 保存当前菜单参数
    lcd_showstr(20, 100, "Load Params"); // 加载之前保存的参数
    lcd_showstr(20, 120, "Back");        // 返回上级
    
    // 回车键逻辑（针对菜单参数的保存/加载）
    if(gpio_get_level(key_enter) == 0)
    {
        delay_ms(300);
        lcd_clear();
        switch(sub_arrow + 20)
        {
            case 20:  sub_page = 2; break;   // 调节AD_DIRE
            case 40:  sub_page = 3; break;   // 调节D_DIRE
            case 60:  sub_page = 4; break;   // 调节P_DIRE
            case 80:  sub_page = 5; break;   // 调节front
            case 100:                        // 保存参数
                menu_save();
                lcd_showstr(50, 80, "Params Saved!"); // 明确提示保存的是菜单参数
                delay_ms(1000);
                lcd_clear();
                sub_page = 1;
                break;
            case 120:                        // 加载参数
                menu_load();
                lcd_showstr(50, 80, "Params Loaded!");
                delay_ms(1000);
                lcd_clear();
                sub_page = 1;
                break;
            case 140: sub_page = 0; break;   // 返回上级
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
        my_pdd.D_DIRE += 0.1;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_pdd.D_DIRE -= 0.1;
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
        my_pdd.P_DIRE += 10;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_pdd.P_DIRE -= 10;
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
    lcd_showint(80, 30, my_control.front, 5);
    lcd_showstr(0, 60, "Up:+  Down:-");
    lcd_showstr(0, 80, "Return:Menu");
    
    if(gpio_get_level(key_up) == 0)
    {
        my_control.front += 1;
        delay_ms(200);
        lcd_clear();
    }
    
    if(gpio_get_level(key_down) == 0)
    {
        my_control.front -= 1;
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

// PDD submenu main function, similar to menu_main
void pdd_sub_menu_main()
{
    // Only process when submenu is active
    if(my_pdd.open==1)
		{
		   menu_load();
			 my_pdd.open++;
		}
		
		
    if(sub_page != 0)
    {
        switch(sub_page)
        {
            case 1: pdd_sub_menu_main_page(); break;  // Main submenu page
            case 2: pdd_ad_dire_adjust(); break;      // AD_DIRE adjustment
            case 3: pdd_d_dire_adjust(); break;       // D_DIRE adjustment
            case 4: pdd_p_dire_adjust(); break;       // P_DIRE adjustment
            case 5: front_adjust(); break;            // front adjustment
				   	case 6:  Camera_pdd_show();break;            // front adjustment
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
