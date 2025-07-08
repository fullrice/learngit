#include "motor.h"
#include "image.h"
#include "control.h"
#include <math.h>

order my_order={
  .go=0 ,
	.cross=0,
	.island=0

};
uint32 key1_count;
uint32 key2_count;
uint32 key3_count;
uint32 key4_count;
uint8  key1_flag;
uint8  key2_flag;
uint8  key3_flag;
uint8  key4_flag;
uint32 count_time;


/*-------------------------------------------------------------------------------------------------------------------
  @brief     十字检测
  @param     null
  @return    null
  Sample     Cross_Detect(void);
  @note      利用四个拐点判别函数，查找四个角点，根据找到拐点的个数决定是否补线
-------------------------------------------------------------------------------------------------------------------*/
void Cross_Detect()
{
    int down_search_start=0;//下点搜索开始行
    my_order.cross=0;
    if(my_order.island==0)//与环岛互斥开
    {
        my_image.Left_Up_Find=0;
        my_image.Right_Up_Find=0;
        if(my_image.Both_Lost_Counter>=10)//十字必定有双边丢线，在有双边丢线的情况下再开始找角点
        {
            Find_Up_Point( MT9V03X_H-1, 0 );
            if(my_image.Left_Up_Find==0&&my_image.Right_Up_Find==0)//只要没有同时找到两个上点，直接结束
            {
                return;
            }
        }
        if(my_image.Left_Up_Find!=0&&my_image.Right_Up_Find!=0)//找到两个上点，就找到十字了
        {
                        my_order.cross=1;//对应标志位，便于各元素互斥掉
            down_search_start=my_image.Left_Up_Find>my_image.Right_Up_Find?my_image.Left_Up_Find:my_image.Right_Up_Find;//用两个上拐点坐标靠下者作为下点的搜索上限
            Find_Down_Point(MT9V03X_H-5,down_search_start+2);//在上拐点下2行作为下点的截止行
            if(my_image.Left_Down_Find<=my_image.Left_Up_Find)
            {
                my_image.Left_Down_Find=0;//下点不可能比上点还靠上
            }
            if(my_image.Right_Down_Find<=my_image.Right_Up_Find)
            {
                my_image.Right_Down_Find=0;//下点不可能比上点还靠上
            }
            if(my_image.Left_Down_Find!=0&&my_image.Right_Down_Find!=0)
            {//四个点都在，无脑连线，这种情况显然很少
                Left_Add_Line (my_image.Left_Line [my_image.Left_Up_Find ],my_image.Left_Up_Find ,my_image.Left_Line [my_image.Left_Down_Find ] ,my_image.Left_Down_Find);
                Right_Add_Line(my_image.Right_Line[my_image.Right_Up_Find],my_image.Right_Up_Find,my_image.Right_Line[my_image.Right_Down_Find],my_image.Right_Down_Find);
            }
            else if(my_image.Left_Down_Find==0&&my_image.Right_Down_Find!=0)//11//这里使用的都是斜率补线
            {//三个点                                     //01
                Lengthen_Left_Boundry(my_image.Left_Up_Find-1,MT9V03X_H-1);
                Right_Add_Line(my_image.Right_Line[my_image.Right_Up_Find],my_image.Right_Up_Find,my_image.Right_Line[my_image.Right_Down_Find],my_image.Right_Down_Find);
            }
            else if(my_image.Left_Down_Find!=0&&my_image.Right_Down_Find==0)//11
            {//三个点                                     //10
                Left_Add_Line (my_image.Left_Line [my_image.Left_Up_Find ],my_image.Left_Up_Find ,my_image.Left_Line [my_image.Left_Down_Find ] ,my_image.Left_Down_Find);
                Lengthen_Right_Boundry(my_image.Right_Up_Find-1,MT9V03X_H-1);
            }
            else if(my_image.Left_Down_Find==0&&my_image.Right_Down_Find==0)//11
            {//就俩上点                                   //00
                Lengthen_Left_Boundry (my_image.Left_Up_Find-1,MT9V03X_H-1);
                Lengthen_Right_Boundry(my_image.Right_Up_Find-1,MT9V03X_H-1);
            }
        }
        else
        {
            my_order.cross=0;
        }
    }
    //角点相关变量，debug使用
    //ips200_showuint8(0,12,Cross_Flag);
//    ips200_showuint8(0,13,Island_State);
//    ips200_showuint8(50,12,Left_Up_Find);
//    ips200_showuint8(100,12,Right_Up_Find);
//    ips200_showuint8(50,13,Left_Down_Find);
//    ips200_showuint8(100,13,Right_Down_Find);
}