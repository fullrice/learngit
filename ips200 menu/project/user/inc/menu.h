#ifndef _MENU_H
#define _MENU_H
/*头文件*/
#include "zf_common_headfile.h"
/*宏定义*/
//按键设置
#define key_enter  (E2)
#define key_return (E3) 
#define key_down (E4) //K2
#define key_up (E5) //K1
//#define key_add 8//K3
//#define key_sub 2//K4


//函数
#define lcd_clear()     ips200_clear()  
#define delay_ms(x)     system_delay_ms (x);
#define lcd_showstr(x, y,dat)     ips200_show_string(x, y, dat)
#define lcd_showfloat(x, y, dat, num,pointnum)       ips200_show_float(x, y, dat, num,pointnum)
void menu_main();
void Menu_show_1();
#endif