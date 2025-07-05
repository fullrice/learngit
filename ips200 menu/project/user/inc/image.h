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

    // 标准赛道宽度（可根据实际需求初始化）
    const uint8 Standard_Road_Wide[MT9V03X_H]; 
} Image;

// 声明全局的结构体变量
extern Image my_image;

// 函数声明
int My_Adapt_Threshold(uint8* image, uint16 width, uint16 height);
void Image_Binarization(int threshold);
int Distributed_Otsu_Threshold(uint8 *image, uint16 width, uint16 height);
void Image_Binarization2(uint8 (*src_image)[MT9V03X_W], uint8 (*dst_image)[MT9V03X_W], uint16 width, uint16 height, int threshold);
void Partitioned_Otsu_Binarization(uint8 (*src_image)[MT9V03X_W], uint8 (*dst_image)[MT9V03X_W], uint16 width, uint16 height);
void lq_sobelAutoThreshold (unsigned char imageIn[MT9V03X_H][MT9V03X_W], unsigned char imageOut[MT9V03X_H][MT9V03X_W]);
void Draw_Track_Boundary();
void Longest_White_Column();
void adaptiveThreshold(uint8_t* img_data, uint8_t* output_data, int width, int height, int block, uint8_t clip_value);

#endif