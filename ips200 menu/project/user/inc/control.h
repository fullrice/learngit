#ifndef _CONTROL_H
#define _CONTROL_H
#define BEEP                (D7 )

typedef struct {
	volatile int go;             //发车阶段的标志位
	volatile int start;
	volatile int cross;             //发车阶段的标志位
	volatile int island; 	
	volatile int time;
	volatile int count_2s;
	volatile int count;
	volatile int beep_count;
	volatile int black; 
	volatile int add; 
	volatile int zebra; 
	volatile int show; 
	volatile int page;
	volatile int encorder_time;
}order;
typedef struct {
   volatile int right_down_line[2];//右下角点 0是行，1是列
	 volatile int left_down_line[2];//右下角点 0是行，1是列
	volatile int monotonicity_change_line[2];
	volatile int island_state;
	volatile int state1_count;
	volatile int state2_count;
	volatile int state3_count;
	volatile int state4_count;
	volatile int state5_count;
	volatile int k;
	volatile int point[2];//0是列，1是行
	volatile int open;
}island;
extern order my_order;
	extern island my_island;
extern int test;
void Cross_Detect();
void Zebra_Detect();
void island_detect();
void Monotonicity_Change_Right(int start,int end);
void Continuity_Change_Right(int start,int end);
void Find_Right_Down_Point(int start,int end);//找四个角点，返回值是角点所在的行数
void Find_Left_Down_Point(int start,int end);//找四个角点，返回值是角点所在的行数
void Continuity_Change_Left(int start,int end);//连续性阈值设置为5
void K_Draw_Line(float k, int startX, int startY,int endY);
void Draw_Line(int startX, int startY, int endX, int endY);
void beep_on();
void xieji(int begin, int end, int y_begin, int y_end);
#endif