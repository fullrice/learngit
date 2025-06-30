#include "menu.h"
void Menu_show_1();
void test1();
void test2();
void test3();
int16 page=1,arrow=0;//控制菜单的俩变量
int16 page_last;//
int16 menu_open=1;//开启标志
float kp=1.0,ki=2.0,kd=3.0;
float I_count1=1.0,I_count2=1,I_count3=1;

typedef struct
	{
		uint8 menu_open;
	}my;
my fan={1};
//fan.menu_open=1;
//按键上下移动

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
	if(page==1)
	{
	    if((arrow<0)) arrow=40;
	   else if((arrow>40))arrow=0;
	}else
	{
		 if((arrow<0)) arrow=60;
	else if((arrow>60))arrow=0;
	}
}


//菜单主程序
void menu_main()
{
    if(fan.menu_open==1)
    {
        switch(page)
        {
            case 1 :Menu_show_1();      break;//第一页
            case 21:test1();      break;//第一页
					  case 22:test2();      break;//第一页
					  case 214:test3();      break;//第一页
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
void test1()//21
{ 
   key_action();
   lcd_showstr(0,arrow,"->");
   lcd_showfloat(20,0, kp, 5, 2);
	 lcd_showfloat(20,20, ki, 5, 2);
	 lcd_showfloat(20,40, kd, 5, 2);
	 //lcd_showfloat(20,80, I_count1, 5, 2);
   lcd_showstr(20,60,"test3");
		if(gpio_get_level(key_enter)== 0)
    {
        delay_ms(300);
        lcd_clear();
		
			switch(arrow)
			{
				case 0:kp++;break;
				case 20:ki++;break;			
				case 40:kd++;break;
				break;
			}
			switch((arrow)+20)//换页
			{
				case 80:page=214;arrow=0;break;
			}	 
    }
		if(gpio_get_level(key_return)== 0)
    {
        delay_ms(200);
        lcd_clear();
        page=1;
    }
		
		kp = (kp >= 5) ?  1.0:kp;
		ki = (ki >= 5) ? 2.0: ki;
		kd = (kd >= 5) ? 3.0: kd;
		/*
		I_count1 = (I_count1 >= 5) ? 1 : I_count1;
		I_count2 = (I_count2 >= 5) ? 1 : I_count2;
		I_count3 = (I_count3 >= 5) ? 1 : I_count3;
	  */

}

void test3()//214
{
  key_action();
   lcd_showstr(0,arrow,"->");
	 lcd_showstr(20,0,"Hello");
	 lcd_showstr(20,20,"Bye");
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