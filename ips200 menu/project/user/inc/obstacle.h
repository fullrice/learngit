#ifndef __OBSTACLE_H
#define __OBSTACLE_H



// 路障状态机状态定义
#define OBSTACLE_STATE_NONE     0  // 无路障
#define OBSTACLE_STATE_FOUND    1  // 发现路障
#define OBSTACLE_STATE_AVOIDING 2  // 避障中
#define OBSTACLE_STATE_RETURN   3  // 回归中

// 避障方向定义
#define AVOID_LEFT  1  // 左避障
#define AVOID_RIGHT 2  // 右避障

typedef struct {
    volatile int state;         // 当前状态(0-3)
    volatile int direction;     // 避障方向(1-左/2-右)
    volatile int step;          // 避障步骤
    volatile int counter[3];    // 各状态计数器 [0]状态1 [1]状态2 [2]状态3
    volatile int narrow_count;  // 赛道变窄计数
    volatile int encoder_sum;   // 编码器累计值
    volatile int flag;         // 标志位(bit0:总标志 bit1:状态1标志 bit2:状态2标志 bit3:状态3标志)
	  volatile int flag1;
  	volatile int counter1;
    volatile int counter3;
	  volatile int  current_width;
	  volatile int  standard_width;
} Obstacle;

extern Obstacle my_obstacle;

// 函数声明
void obstacle_init(void);
void obstacle_show();//22
void obstacle_detect(void);
void obstacle_detect_process(void);
void obstacle_avoid_control(void);
void obstacle_avoid_process(void);
int get_obstacle_state(void);
int get_obstacle_direction(void);
void reset_obstacle_detect(void);

#endif