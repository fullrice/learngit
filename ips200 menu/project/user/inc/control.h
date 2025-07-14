#ifndef _CONTROL_H
#define _CONTROL_H


typedef struct {
	volatile int go;             //发车阶段的标志位
	volatile int start;
	volatile int cross;             //发车阶段的标志位
	volatile int island; 	
	volatile int time;
	volatile int count_2s;
	volatile int black; 
	volatile int add; 
	volatile int zebra; 
}order;
typedef struct {
   volatile int right_down_line;//右下角点
	volatile int monotonicity_change_line;
	
}island;
extern order my_order;
void Cross_Detect();
void Zebra_Detect();
void Monotonicity_Change_Right(int start,int end);
void Continuity_Change_Right(int start,int end);
#endif