#ifndef _CONTROL_H
#define _CONTROL_H


typedef struct {
	volatile int go;             //发车阶段的标志位
	volatile int cross;             //发车阶段的标志位
	volatile int island; 	
	volatile int time;
}order;
extern order my_order;

extern uint32 key1_count;
extern uint32 key2_count;
extern uint32 key3_count;
extern uint32 key4_count;
extern uint8  key1_flag;
extern uint8  key2_flag;
extern uint8  key3_flag;
extern uint8  key4_flag;
extern uint32 count_time;
#endif