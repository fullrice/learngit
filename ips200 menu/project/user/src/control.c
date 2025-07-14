#include "motor.h"
#include "image.h"
#include "control.h"
#include <math.h>

order my_order={
  .go=0 ,
	.cross=0,
	.island=0,
  .count_2s=0,
	.black=0,
	.start=0,
	.add=0,
	.zebra=0
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

island my_island={0};

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
       // my_image.Left_Up_Find=0;
     //   my_image.Right_Up_Find=0;
        if(my_image.Both_Lost_Counter>=10)//十字必定有双边丢线，在有双边丢线的情况下再开始找角点
        {
            Find_Up_Point( MT9V03X_H-1, 0 );
            if(my_image.cross_count<2)//只要没有同时找到两个点，直接结束
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



/*-------------------------------------------------------------------------------------------------------------------
  @brief     右下角点检测
  @param     起始点，终止点
  @return    返回角点所在的行数，找不到返回0
  Sample     Find_Right_Down_Point(int start,int end);
  @note      角点检测阈值可根据实际值更改
-------------------------------------------------------------------------------------------------------------------*/
void Find_Right_Down_Point(int start,int end)//找四个角点，返回值是角点所在的行数
{
    int i,t;
    int right_down_line=0;
    if(my_image.Right_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没有拐点判断的意义
        my_island.right_down_line=right_down_line;
    if(start<end)
    {
        t=start;
        start=end;
        end=t;
    }
    if(start>=MT9V03X_H-1-5)//下面5行数据不稳定，不能作为边界点来判断，舍弃
        start=MT9V03X_H-1-5;
    if(end<=MT9V03X_H-my_image.Search_Stop_Line)
        end=MT9V03X_H-my_image.Search_Stop_Line;
    if(end<=5)
       end=5;
    for(i=start;i>=end;i--)
    {
        if(right_down_line==0&&//只找第一个符合条件的点
           abs(my_image.Right_Line[i]-my_image.Right_Line[i+1])<=5&&//角点的阈值可以更改
           abs(my_image.Right_Line[i+1]-my_image.Right_Line[i+2])<=5&&  
           abs(my_image.Right_Line[i+2]-my_image.Right_Line[i+3])<=5&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-2])<=-5&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-3])<=-10&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-4])<=-10)
        {
            right_down_line=i;//获取行数即可
            break;
        }
    }
    my_island.right_down_line=right_down_line;
}

/*
 * 斑马线检测函数，检测屏幕黑白跳变数，阈值可调，也要注意判断范围，近车靠头还是远离车头
 */
void Zebra_Detect(void)
{
    int i,j;
    int change=0;
    for(i=38;i>=35;i--)
    {
        for(j=10;j<=100;j++)
        {
            if(my_image.image_two_value[i][j+1]-my_image.image_two_value[i][j]!=0)
            {
                change++;
            }
        }
    }
   // lcd_showint16(100,6,change);
    if(change>=20)
    {
 
			my_order.zebra=1;  //遇上斑马线了
    }
   
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     右赛道连续性检测
  @param     起始点，终止点
  @return    连续返回0，不连续返回断线出行数
  Sample     continuity_change_flag=Continuity_Change_Right(int start,int end)
  @note      连续性的阈值设置为5，可更改
-------------------------------------------------------------------------------------------------------------------*/
void Continuity_Change_Right(int start,int end)
{
    int i;
    int t;
    if(my_image.Right_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没必要判断了
		{
			  my_image.continuity_change_flag=0;
		}
    if(start>=MT9V03X_H-5)//数组越界保护
        start=MT9V03X_H-5;
    if(end<=5)
       end=5;
    if(start<end)//都是从下往上计算的，反了就互换一下
    {
       t=start;
       start=end;
       end=t;
    }
 
    for(i=start;i>=end;i--)
    {
        if(abs(my_image.Right_Line[i]-my_image.Right_Line[i-1])>=5)//连续性阈值是5，可更改
       {
            my_image.continuity_change_flag=i;
            break;
       }
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     单调性突变检测
  @param     起始点，终止行
  @return    点所在的行数，找不到返回0
  Sample     Find_Right_Up_Point(int start,int end);
  @note      前5后5它最大（最小），那他就是角点
-------------------------------------------------------------------------------------------------------------------*/
void Monotonicity_Change_Right(int start,int end)//单调性改变，返回值是单调性改变点所在的行数
{
    int i;
    int temp;
    if(my_image.Right_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没有单调性判断的意义
		{
        my_island.monotonicity_change_line=0;
		}
    if(start>=MT9V03X_H-1-5)//数组越界保护
        start=MT9V03X_H-1-5;
     if(end<=5)
        end=5;
    if(start<=end)
      temp=end;
		   end=start;
		   start=temp;
        my_island.monotonicity_change_line=0;
    for(i=start;i>=end;i--)//会读取前5后5数据，所以前面对输入范围有要求
    {
        if(my_image.Right_Line[i]==my_image.Right_Line[i+5]&&my_image.Right_Line[i]==my_image.Right_Line[i-5]&&
        my_image.Right_Line[i]==my_image.Right_Line[i+4]&&my_image.Right_Line[i]==my_image.Right_Line[i-4]&&
        my_image.Right_Line[i]==my_image.Right_Line[i+3]&&my_image.Right_Line[i]==my_image.Right_Line[i-3]&&
        my_image.Right_Line[i]==my_image.Right_Line[i+2]&&my_image.Right_Line[i]==my_image.Right_Line[i-2]&&
        my_image.Right_Line[i]==my_image.Right_Line[i+1]&&my_image.Right_Line[i]==my_image.Right_Line[i-1])
        {//一堆数据一样，显然不能作为单调转折点
            continue;
        }
        else if(my_image.Right_Line[i] <my_image.Right_Line[i+5]&&my_image.Right_Line[i] <my_image.Right_Line[i-5]&&
        my_image.Right_Line[i] <my_image.Right_Line[i+4]&&my_image.Right_Line[i] <my_image.Right_Line[i-4]&&
        my_image.Right_Line[i]<=my_image.Right_Line[i+3]&&my_image.Right_Line[i]<=my_image.Right_Line[i-3]&&
        my_image.Right_Line[i]<=my_image.Right_Line[i+2]&&my_image.Right_Line[i]<=my_image.Right_Line[i-2]&&
        my_image.Right_Line[i]<=my_image.Right_Line[i+1]&&my_image.Right_Line[i]<=my_image.Right_Line[i-1])
        {//就很暴力，这个数据是在前5，后5中最大的，那就是单调突变点
            my_island.monotonicity_change_line=i;
            break;
        }
    }
   
}
