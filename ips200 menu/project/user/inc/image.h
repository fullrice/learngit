#ifndef _IMAGE_H
#define _IMAGE_H

/*头文件*/
#include "zf_common_headfile.h"

/*宏定义*/
// 图像颜色宏
#define IMG_WHITE  255
#define IMG_BLACK  0

// 定义整合所有图像相关数据的结构体
typedef struct 
{
    // 二值化后的图像数组
    uint8 image_two_value[MT9V03X_H][MT9V03X_W];
    uint8 maze_display[MT9V03X_H][MT9V03X_W];
    // 数组变量
    volatile int Left_Line[MT9V03X_H]; //左边线数组
		volatile int Right_Line[MT9V03X_H];//右边线数组
		volatile int Mid_Line[MT9V03X_H];  //中线数组
		volatile int Road_Wide[MT9V03X_H]; //赛宽数组
		volatile int White_Column[MT9V03X_W];    //每列白列长度

    // 单值变量
    volatile int Search_Stop_Line;     //搜索截止行,只记录长度，想要坐标需要用视野高度减去该值
		volatile int Boundry_Start_Left;   //左右边界起始点
		volatile int Boundry_Start_Right;  //第一个非丢线点,常规边界起始点
		volatile int Left_Lost_Counter;       //边界丢线数
		volatile int Right_Lost_Counter;
		volatile int Both_Lost_Counter;//两边同时丢线数

    // 结构类变量
    int Longest_White_Column_Left[2];  // 左最长白列：[0]=长度, [1]=列号
    int Longest_White_Column_Right[2]; // 右最长白列：[0]=长度, [1]=列号
    int Left_Lost_Flag[MT9V03X_H];     // 左丢线标记（1=丢线，0=正常）
    int Right_Lost_Flag[MT9V03X_H];    // 右丢线标记（1=丢线，0=正常）
    int stepl;
		int stepr;
		int ptsl[300][2];
		int ptsr[300][2];
		int start_x_l;
		int start_x_r;
		int start_y;
    // 标准赛道宽度（可根据实际需求初始化）
    const uint8 Standard_Road_Wide[MT9V03X_H]; 
		
		//十字
		volatile int Cross_Flag;
		volatile int Left_Down_Find; //十字使用，找到被置行数，没找到就是0
		volatile int Left_Up_Find;   //四个拐点标志
		volatile int Right_Down_Find;
		volatile int Right_Up_Find;
		volatile int cross_count;
		volatile int continuity_change_flag;
} Image;

// 声明全局的结构体变量
extern Image my_image;

// 函数声明
int My_Adapt_Threshold(uint8* image, uint16 width, uint16 height);
void Image_Binarization(int threshold);
int Distributed_Otsu_Threshold(uint8 *image, uint16 width, uint16 height);
void Image_Binarization2(uint8 (*src_image)[MT9V03X_W], uint8 (*dst_image)[MT9V03X_W], uint16 width, uint16 height, int threshold);
void Partitioned_Otsu_Binarization(uint8 (*src_image)[MT9V03X_W], uint8 (*dst_image)[MT9V03X_W], uint16 width, uint16 height);
void Draw_Track_Boundary();
void Longest_White_Column();
void adaptiveThreshold(uint8_t* img_data, uint8_t* output_data, int width, int height, int block, uint8_t clip_value);
void Left_Add_Line(int x1,int y1,int x2,int y2);
void Right_Add_Line(int x1,int y1,int x2,int y2);
void Lengthen_Left_Boundry(int start,int end);
void Lengthen_Right_Boundry(int start,int end);
void Find_Down_Point(int start,int end);
void Find_Up_Point(int start,int end);
void draw_mid_line();
void draw_boundary_lines(void);
void draw_boundary_lines_wide(void);
#endif