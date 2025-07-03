#ifndef _IMAGE_H
#define _IMAGE_H
/*头文件*/
#include "zf_common_headfile.h"
/*宏定义*/
//按键设置
#define IMG_WHITE  255
extern uint8 image_two_value[MT9V03X_H][MT9V03X_W];//二值化后的原数组
//按键设置
#define IMG_BLACK  0
//#define key_add 8//K3
//#define key_sub 2//K4
int My_Adapt_Threshold(uint8*image,uint16 width, uint16 height);
void Image_Binarization(int threshold);//图像二值化
int Distributed_Otsu_Threshold(uint8 *image, uint16 width, uint16 height);
void Image_Binarization2(uint8 (*src_image)[MT9V03X_W], uint8 (*dst_image)[MT9V03X_W], uint16 width, uint16 height, int threshold);
void Partitioned_Otsu_Binarization(
    uint8 (*src_image)[MT9V03X_W], 
    uint8 (*dst_image)[MT9V03X_W], 
    uint16 width, 
    uint16 height
) ;
void lq_sobelAutoThreshold (unsigned char imageIn[MT9V03X_H][MT9V03X_W], unsigned char imageOut[MT9V03X_H][MT9V03X_W]);
#endif