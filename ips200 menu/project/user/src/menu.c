#include "menu.h"
#include "isr.h"
#include "image.h"
#include "motor.h"
#include "control.h"
void Menu_show_1();
void test1();
void test2();
void test3();
void dir();  //24
void dir_p_show();//242
void dir_d_show();//241
void speed_p_show();//212
void speed_i_show();//212
void speed_show();//213
void Camera_show();
void flash_read_speed();
int16 page=1,arrow=0;//控制菜单的俩变量
int16 page_last;//
int count=0;


typedef struct
	{
		uint8 menu_open;
	}menu;
menu my_menu={1};
//fan.menu_open=1;
//按键上下移动
void motor();
void control_show();
void go_show();//231
void go_show_ready();
void key_action()
{          
//	     if(gpio_get_level(key_up)==0)
//	{
//		    key3_flag=1;
//		    key3_count=0;
//		       lcd_clear();
//		   arrow=arrow-20;
//	}      //60 4   40  3        
    if(gpio_get_level(key_up)==0)
    {
		   delay_ms(300);
       lcd_clear();
		arrow=arrow-20;
    }
    if(gpio_get_level(key_down)==0)
    {
//       key4_flag=1;
//		    key4_count=0;
			delay_ms(300);
		       lcd_clear();
		   arrow=arrow+20;
    }
	switch(page)
	{
		case 1:if((arrow<0)) arrow=60;else if((arrow>60))arrow=0; break;
		case 21:if((arrow<0)) arrow=40;else if((arrow>40))arrow=0; break;
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
					  case 22:Camera_show();      break;//第一页
					 case 23:control_show();     break;//第一页
					case 24 :dir();break;
					  case 211:speed_p_show();break;
					  case 212:speed_i_show();break;
					case 213:speed_show();break;
					case 231:go_show() ; break;
					case 241:dir_d_show(); ; break;
					case 242:dir_p_show() ; break;
  				case 2311:go_show_ready();break;
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
//	 flash_read_speed();
	  page_last=page;
    page=1;
    if(page!=page_last)
    {
        arrow=0;
    }
    key_action();
    lcd_showstr(0,arrow,"->");
		lcd_showstr(20,0,"speed");
    lcd_showstr(20,20,"image_show");
		lcd_showstr(20,40,"control");
		lcd_showstr(20,60,"DIR");
    if(gpio_get_level(key_enter)==0)
    {
        delay_ms(300);
        lcd_clear();
					switch((arrow)+20)//进入第二页，前面一页的箭头选中选项
			{
				case 20:page=21;arrow=0;break;
				case 40:page=22;arrow=0;break;
				case 60:page=23;arrow=0;break;
				case 80 :page=24;arrow=0;break;
				case 5:page=25;arrow=0;break;
				case 6:page=26;arrow=0;break;
				case 7:page=27;arrow=0;break;
				case 8:page=28;arrow=0;break;
			}
	 
    }
}
void flash_read_speed()
{
   flash_read_page_to_buffer(10,1);  
   my_control.P_SPEED= flash_union_buffer[0].float_type;
   my_control.I_SPEED= flash_union_buffer[1].float_type;
}

void flash_write_speed()
{
    flash_buffer_clear();
		flash_union_buffer[0].float_type  =my_control.P_SPEED;  
	  flash_union_buffer[1].float_type  =my_control.I_SPEED;  
		flash_erase_page(10,1);
	  flash_write_page_from_buffer(10,1);        // 向指定 Flash 扇区的页码写入缓冲区数据

}
void control_show()//23
{
	   key_action();
     lcd_showstr(0,arrow,"->");
	   lcd_showstr(20,0,"go");
	   lcd_showstr(20,20,"stop");
     if(gpio_get_level(key_enter)== 0)
    {
        delay_ms(300);
        lcd_clear();
			
			switch((arrow)+20)//换页
			{
				case 20:page=231;arrow=0;break;
				case 40:page=232;arrow=0;break;
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
void go_show()//231
{
	  
	   lcd_showstr(0,0,"set_duty");
	  lcd_showfloat(0,20, my_control.Speed_Left_Set, 5, 2);
	   if(gpio_get_level(key_enter)== 0)
    {
        delay_ms(300);
        lcd_clear();
			  page=2311;
    }
	  if(gpio_get_level(key_up)== 0)
    {
			my_control.Speed_Left_Set=my_control.Speed_Left_Set+100;
			my_control.Speed_Right_Set=my_control.Speed_Right_Set+100;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.Speed_Left_Set=my_control.Speed_Left_Set-100;
			my_control.Speed_Right_Set=my_control.Speed_Right_Set-100;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=23;
    }
		flash_write_speed();
}
void go_show_ready()
{
    lcd_showstr(0,0,"Are you sure to go?");
	 if(gpio_get_level(key_enter)== 0)
    {
			int i=0;
			while(i<10)
      {
			   delay_ms(300);
				i++;
			}
			my_order.count_2s=0;//重新计时 
			my_order.go=1;
			 while(count<1000)
			 {
				 count++;
			 }
			 count=0;
			  page=213;
        delay_ms(100);
        lcd_clear();
			 
    }
    if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=231;
    }
}
void motor()//21
{ 
   key_action();
//	 flash_write_speed();
//	 flash_read_speed();
	// flash_read_page_to_buffer(10, 1);  
//	 my_control.P_SPEED= flash_union_buffer[0].float_type;
//	 flash_buffer_clear();
//	 flash_read_page_to_buffer(11, 1);  
//	 my_control.I_SPEED= flash_union_buffer[0].float_type;
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
void dir()//24
{ 
   key_action();
//	 flash_write_speed();
//	 flash_read_speed();
	// flash_read_page_to_buffer(10, 1);  
//	 my_control.P_SPEED= flash_union_buffer[0].float_type;
//	 flash_buffer_clear();
//	 flash_read_page_to_buffer(11, 1);  
//	 my_control.I_SPEED= flash_union_buffer[0].float_type;
	// kd= flash_union_buffer[2].float_type;
   lcd_showstr(0,arrow,"->");
	 lcd_showstr(20,0,"dir_p_show:");
   lcd_showfloat(140,0, my_control.P_DIRE , 5, 2);
	 lcd_showstr(20,20,"dir_D_show:");
	 lcd_showfloat(140,20, my_control.D_DIRE, 5, 2);
	 lcd_showstr(20,40,"dir_show");
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
				case 20:page=242;arrow=0;break;
				case 40:page=241;arrow=0;break;
		//		case 60:page=243;arrow=0;break;
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
			 my_control.P_SPEED=my_control.P_SPEED+0.3;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.P_SPEED=my_control.P_SPEED-0.3;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=21;
    }
		flash_write_speed();
    
}
void dir_d_show()//241
{
  //  key_action();
 //   lcd_showstr(20,arrow,"->");
	  lcd_showstr(0,0,"dir_d");
	  lcd_showfloat(0,20, my_control.D_DIRE, 5, 2);
	if(gpio_get_level(key_up)== 0)
    {
			 my_control.D_DIRE=my_control.D_DIRE+10;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.D_DIRE=my_control.D_DIRE-10 ;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=24;
    }
		flash_write_speed();
}
void dir_p_show()//242
{
  //  key_action();
  //  lcd_showstr(20,arrow,"->");
	  lcd_showstr(0,0,"dir_p");
	  lcd_showfloat(0,20, my_control.P_DIRE, 5, 2);
	if(gpio_get_level(key_up)== 0)
    {
			 my_control.P_DIRE=my_control.P_DIRE+1 ;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.P_DIRE=my_control.P_DIRE-1 ;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=24;
    }
		flash_write_speed();
    
}
void speed_i_show()//212
{
  //  key_action();
 //   lcd_showstr(20,arrow,"->");
	  lcd_showstr(0,0,"speed_i");
	  lcd_showfloat(0,20, my_control.I_SPEED, 5, 2);
	if(gpio_get_level(key_up)== 0)
    {
			 my_control.I_SPEED=my_control.I_SPEED+0.01;
        delay_ms(300);
        lcd_clear();
		}
		if(gpio_get_level(key_down)== 0)
    {
			 my_control.I_SPEED=my_control.I_SPEED-0.01;
        delay_ms(300);
        lcd_clear();
		}
	 if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=21;
    }
		flash_write_speed();
}
void speed_show()//213
{  
//   lcd_showstr(0,arrow,"->");
	 delay_ms(200);
	  lcd_showstr(0,0,"set_l");
	 lcd_showstr(0,20,"set_r");
	 lcd_showstr(0,40,"speed_l");
	 lcd_showstr(0,60,"speed_r");
	 lcd_showstr(0,80,"steer_out");
	lcd_showstr(0,100,"err");
	lcd_showstr(0,120,"pwml");
	lcd_showstr(0,140,"pwmr");
	lcd_showstr(0,160,"lost");
	 lcd_showint(100,0, my_control.Speed_Left_Set, 5);
	 lcd_showint(100,20, my_control.Speed_Right_Set, 5);
	 lcd_showint(100,40, my_control.encoderl, 5);
	 lcd_showint(100,60, my_control.encoderr, 5);
	lcd_showint(100,80, my_control.steer_output , 5);
	lcd_showint(100,100, my_control.err , 5);
	lcd_showint(100,120, my_control.pwm_l , 5);
	lcd_showint(100,140, my_control.pwm_r , 5);
		lcd_showint(100,160, my_image.Longest_White_Column_Right[0] , 5);

//	if(my_order.go==0)
//	{
//	   delay_ms(200);
//        lcd_clear();
//        page=211;
//	}
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
	 lcd_showstr(0,80,"steer_out");
	 lcd_showint(100,0, my_control.Speed_Left_Set, 5);
	 lcd_showint(100,20, my_control.Speed_Right_Set, 5);
	 lcd_showint(100,40, my_control.encoderl, 5);
	 lcd_showint(100,60, my_control.encoderr, 5);
	lcd_showint(100,60, my_control.encoderr, 5);
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

void Camera_show()//22
{
 // ips200_show_gray_image(0, 0, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
	ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);  
	lcd_showstr(0,110,"speed_err");
	lcd_showint(100,110, my_control.speed_lasterrL , 5);
	lcd_showstr(0,130,"add");
	lcd_showint(100,130, my_order.add , 5);
	lcd_showstr(0,90,"pwm2");
	lcd_showint(100,90, my_control.pwm_r  , 5);
	lcd_showstr(0,150,"err");
	lcd_showint(100,150, my_control.err , 5);
	lcd_showstr(0,170,"p");
	lcd_showfloat(100,170, my_control.P_DIRE , 5,3);
		lcd_showstr(0,190,"d");
	lcd_showfloat(100,190, my_control.D_DIRE , 5,3);
		lcd_showstr(0,210,"set_speed");
	lcd_showint(100,210, my_control.Speed_Left_Set , 5);
		lcd_showstr(0,230,"speed_L");
	lcd_showint(100,230, my_control.encoderl , 5);
		lcd_showstr(0,250,"speed_R");
	lcd_showint(100,250, my_control.encoderr , 5);
	lcd_showstr(0,270,"steer_out");
	lcd_showint(100,270, my_control.steer_output , 5);
		lcd_showstr(0,290,"black");
	lcd_showint(100,290, my_order.black , 5);
	draw_mid_line();
//	Cross_Detect(); 
//	Find_Up_Point( MT9V03X_H-1, 0 );
//	Find_Down_Point( MT9V03X_H-1, 0 );
//	ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
//	Draw_Track_Boundary();
 if(gpio_get_level(key_up)== 0)
    {
        my_control.D_DIRE+=1;
			//	my_order.add++;
			//my_control.P_DIRE+=10;
			  delay_ms(200);
        lcd_clear();
    }
		 if(gpio_get_level(key_down)== 0)
    {
        my_control.D_DIRE-=1;
			//	my_order.add--;
		//	my_control.P_DIRE-=10;
			  delay_ms(200);
        lcd_clear();
    }
//	 if(gpio_get_level(key_return)== 0)
//    {
//        delay_ms(200);
//        lcd_clear();
//        page=1;
//    }
	 if(gpio_get_level(key_return)== 0)
    {   
			
				my_order.count_2s=0;//重新计时 
			 delay_ms(400);
			  my_order.go=1;
			  delay_ms(200);
        lcd_clear();
    }




}
void Cross_show()//22
{
 // ips200_show_gray_image(0, 0, (const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
	ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);  
	draw_mid_line();
//	Cross_Detect(); 
	Find_Up_Point( MT9V03X_H-1, 0 );
	Find_Down_Point( MT9V03X_H-1, 0 );
//	ips200_show_gray_image(0, 0, (const uint8 *)my_image.image_two_value, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
//	Draw_Track_Boundary();
 if(gpio_get_level(key_up)== 0)
    {
     //   my_control.D_DIRE+=1;
		//	my_control.P_DIRE+=10;
		//	my_order.add++;
			  delay_ms(200);
        lcd_clear();
    }
		 if(gpio_get_level(key_down)== 0)
    {//    my_control.D_DIRE-=1;
		//	my_control.P_DIRE-=10;
		//	my_order.add--;
			  delay_ms(200);
        lcd_clear();
    }
//	 if(gpio_get_level(key_return)== 0)
//    {
//        delay_ms(200);
//        lcd_clear();
//        page=1;
//    }
	 if(gpio_get_level(key_return)== 0)
    {   
			
				my_order.count_2s=0;//重新计时 
			  delay_ms(400);
			  my_order.go=1;
			  delay_ms(200);
        lcd_clear();
    }




}