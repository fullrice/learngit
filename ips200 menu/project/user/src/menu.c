#include "menu.h"
#include "image.h"
#include "motor.h"
void Menu_show_1();
void test1();
void test2();
void test3();
void speed_p_show();//212
void speed_i_show();//212
void speed_show();//213
int16 page=1,arrow=0;//控制菜单的俩变量
int16 page_last;//



typedef struct
	{
		uint8 menu_open;
	}menu;
menu my_menu={1};
//fan.menu_open=1;
//按键上下移动
void motor();
void key_action()
{          
	                  
    if(gpio_get_level(key_up)==0)
    {
		   delay_ms(300);
       lcd_clear();
		arrow=arrow-20;
    }
    if(gpio_get_level(key_down)==0)
    {
        delay_ms(300);
        lcd_clear();
		   arrow=arrow+20;
    }
	switch(page)
	{
		case 1:if((arrow<0)) arrow=40;else if((arrow>40))arrow=0; break;
	}
	
	if((arrow<0)) arrow=60;
	else if((arrow>60))arrow=0;
	
}


//菜单主程序
void menu_main()
{
    if(my_menu.menu_open==1)
    {
        switch(page)
        {
            case 1 :Menu_show_1();      break;//第一页
            case 21:motor();      break;//第一页
					  case 22:test2();      break;//第一页
					  case 211:speed_p_show();break;
					  case 212:speed_i_show();break;
					case 213:speed_show();break;
					 // case 214:test3();      break;//第一页
        }
    }
	 if(gpio_get_level(key_return)==0)
	 {
		 
	   //主菜单下的return可以添加指定效果
	 }
 }
void Menu_show_1()//1
{
	  //更新
	  page_last=page;
    page=1;
    if(page!=page_last)
    {
        arrow=0;
    }
    key_action();
    lcd_showstr(0,arrow,"->");
		lcd_showstr(20,0,"test1");
    lcd_showstr(20,20,"test2");
		lcd_showstr(20,40,"shortage");
    if(gpio_get_level(key_enter)==0)
    {
        delay_ms(300);
        lcd_clear();
					switch((arrow)+20)//进入第二页，前面一页的箭头选中选项
			{
				case 20:page=21;arrow=0;break;
				case 40:page=22;arrow=0;break;
				case 61:page=23;arrow=0;break;
				case 4:page=24;arrow=0;break;
				case 5:page=25;arrow=0;break;
				case 6:page=26;arrow=0;break;
				case 7:page=27;arrow=0;break;
				case 8:page=28;arrow=0;break;
			}
	 
    }
}
void motor()//21
{ 
   key_action();
	 flash_read_page_to_buffer(10, 1);  
	 my_control.P_SPEED= flash_union_buffer[0].float_type;
	// flash_buffer_clear();
	// flash_read_page_to_buffer(11, 1);  
//	 my_control.I_SPEED= flash_union_buffer[1].float_type;
	// kd= flash_union_buffer[2].float_type;
   lcd_showstr(0,arrow,"->");
	 lcd_showstr(20,0,"speed_p_show:");
   lcd_showfloat(140,0, my_control.P_SPEED, 5, 2);
	 lcd_showstr(20,20,"speed_i_show:");
	 lcd_showfloat(140,20, my_control.I_SPEED, 5, 2);
	 lcd_showstr(20,40,"speed_show");
	// lcd_showfloat(20,60, my_control.encoder2, 5, 2);
//	 lcd_showfloat(20,40, kd, 5, 2);
	 //lcd_showfloat(20,80, I_count1, 5, 2);
 //  lcd_showstr(20,60,"test3");
		if(gpio_get_level(key_enter)== 0)
    {
        delay_ms(300);
        lcd_clear();
			
			switch((arrow)+20)//换页
			{
				case 20:page=211;arrow=0;break;
				case 40:page=212;arrow=0;break;
				case 60:page=213;arrow=0;break;
			//	case 80:page=214;arrow=0;break;
			}	 
    }
		if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=1;
    }


}

void speed_p_show()//211
{
  //  key_action();
  //  lcd_showstr(20,arrow,"->");
	  lcd_showstr(0,0,"speed_p");
	  lcd_showfloat(0,20, my_control.P_SPEED, 5, 2);
	if(gpio_get_level(key_up)== 0)
    {
			 my_control.P_SPEED=my_control.P_SPEED+0.1;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.P_SPEED=my_control.P_SPEED-0.1;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=21;
    }
		flash_buffer_clear();
		flash_union_buffer[0].float_type  =my_control.P_SPEED;  
		flash_erase_page(10, 1);
	  flash_write_page_from_buffer(10, 1);        // 向指定 Flash 扇区的页码写入缓冲区数据			

}
void speed_i_show()//212
{
  //  key_action();
 //   lcd_showstr(20,arrow,"->");
	  lcd_showstr(0,0,"speed_i");
	  lcd_showfloat(0,20, my_control.I_SPEED, 5, 2);
	if(gpio_get_level(key_up)== 0)
    {
			 my_control.I_SPEED=my_control.I_SPEED+0.1;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.I_SPEED=my_control.I_SPEED-0.1;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=21;
    }
		flash_buffer_clear();
		flash_union_buffer[0].float_type  =my_control.I_SPEED;  
		flash_erase_page(11, 1);
	  flash_write_page_from_buffer(11, 1);        // 向指定 Flash 扇区的页码写入缓冲区数据			

}
void speed_show()//213
{  
//   lcd_showstr(0,arrow,"->");
	 lcd_showstr(0,0,"set_l");
	 lcd_showstr(0,20,"set_r");
	 lcd_showstr(0,40,"speed_l");
	 lcd_showstr(0,60,"speed_r");
	 lcd_showint(100,0, my_control.Speed_Left_Set, 5);
	 lcd_showint(100,20, my_control.Speed_Right_Set, 5);
	 lcd_showint(100,40, -my_control.encoder1, 5);
	 lcd_showint(100,60, my_control.encoder2, 5);
   if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=21;
    }



}
void test3()//214
{
  key_action();
//   lcd_showstr(0,arrow,"->");
	 lcd_showstr(0,0,"set_l");
	 lcd_showstr(0,20,"set_r");
	 lcd_showstr(0,40,"speed_l");
	 lcd_showstr(0,60,"speed_r");
	 lcd_showint(100,0, my_control.Speed_Left_Set, 5);
	 lcd_showint(100,20, my_control.Speed_Right_Set, 5);
	 lcd_showint(100,40, my_control.encoder1, 5);
	 lcd_showint(100,60, my_control.encoder2, 5);
	if(gpio_get_level(key_enter)== 0)
    {
        delay_ms(300);
        lcd_clear();
			
			switch((arrow)+20)//换页
			{
				case 20:page=211;arrow=0;break;
				case 40:page=212;arrow=0;break;
				case 60:page=213;arrow=0;break;
			//	case 80:page=214;arrow=0;break;
			}	 
    }
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=21;
    }

}

void test2()//22
{
   key_action();
   lcd_showstr(0,arrow,"->");
	 lcd_showstr(20,0,"olleh");
	 lcd_showstr(20,20,"eyb");
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=1;
    }




}