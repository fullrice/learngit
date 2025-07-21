#include "motor.h"
#include "image.h"
#include "control.h"
#include <math.h>

#define BEEP                (D7 )
order my_order={
  .go=0 ,
	.cross=0,
	.island=0,
  .count_2s=0,
	.black=0,
	.start=0,
	.add=-39,
	.zebra=0,
	.beep_count=0,
	.count=0,
	.show=1,
	.page=1,  
	.encorder_time=0
};
island my_island={
  .right_down_line={80},
	.left_down_line={80},
	.island_state=0,
	.state1_count=0,
	.state2_count=0,
  .state3_count=0,
	.state4_count=0,
	.state5_count=0,
	.open=0,
	.monotonicity_change_line={0},
	.k=0,
  .point={0}//单调点的情况
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
int test=0;

void beep_on()
{
   if(my_order.beep_count>=20)
	 {
	     	  gpio_set_level(BEEP, GPIO_LOW);                                             // BEEP 停
	 }
	 else
	 {
          gpio_set_level(BEEP, GPIO_HIGH);                                            // BEEP 响
	        my_order.beep_count++;
	 }



}
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

/*
状态零,判断为左右丢线，左右连续性，搜索停止行，判断拐点，有拐点，进入状态一
状态一，判断为拐点消失，右下角的丢线很下，找到单调点过于向下，进入状态二
状态二，判断为最短白列太短，并且找到上角点进行补线，此时可以进入状态三

找到拐点和丢线情况进入状态一，有拐点，
根据拐点过于向下，和单调点的位置范围，进入状态二，此时只有单调点没有角点




*/
void island_detect()
{
	//左右，连续性
 //  Continuity_Change_Right(MT9V03X_H-1-5,10);
//   Continuity_Change_Left(MT9V03X_H-1-5,10);
	if(my_island.island_state==0)//此时是状态零,判断为左右丢线，左右连续性，搜索停止行，判断拐点，有拐点
	{
	 //  Continuity_Change_Right(MT9V03X_H-1-5,10);
   //  Continuity_Change_Left(MT9V03X_H-1-5,10);
	 //  Monotonicity_Change_Right(MT9V03X_H-1-5,10);//搜索行，可以适当调近  //找到右下拐点，算是进入状态一
		 if(       
			         my_image.continuity_change_flag_right>=20&& //右边不连续
		           my_image.continuity_change_flag_left<=5&&   //左边连续
               my_image.Right_Lost_Counter>=20&&           //右丢线多
               my_image.Right_Lost_Counter<=50&&           //右丢线不能太多
               my_image.Left_Lost_Counter<=5&&            //左丢线少
               my_image.Search_Stop_Line>=79&& //搜索截止行看到很远
               my_image.Boundry_Start_Left>=MT9V03X_H-20&&my_image.Boundry_Start_Right>=MT9V03X_H-20&& //边界起始点靠下
               my_image.Both_Lost_Counter<=10)  
            {
              //  Find_Right_Down_Point(MT9V03X_H-1,20);//右下点
//                if(my_island.right_down_line[0]>=20)//条件1很松，在这里加判拐点，位置不对，则是误判，跳出
//                {
                    my_island.island_state=1;//进入状态一
									  my_island.state1_count=0;
									  my_order.beep_count=0;
//                }//进入之后开始
//                else
//                {
//                    my_island.island_state=0;
//                }
            }
					}
						//单调点12-60
	   if(my_island.island_state==1)  //拐点消失
        {
					 beep_on();
          my_island.state1_count+=my_control.encoderl;
          // Monotonicity_Change_Right(70,10);//找角点
           my_island.monotonicity_change_line[1]=my_image.Right_Line[my_island.monotonicity_change_line[0]];//角点的行列
        //   Right_Add_Line((int)(MT9V03X_W-1-(my_island.monotonicity_change_line[1]*0.15)),MT9V03X_H-1,my_island.monotonicity_change_line[1],my_island.monotonicity_change_line[0]);//直接拉边界线
					if(my_island.state1_count>=10000&& my_island.monotonicity_change_line[0]>=20 && my_island.monotonicity_change_line[0]<=60 && my_island.right_down_line[0]>=76)//找到的单调点过于向下，开始进入
            {   
							 my_order.beep_count=0;
                my_island.island_state =2;
							  my_island.state2_count=0;
            }//5000
        }
		if(my_island.island_state==2)
		{
			   beep_on();
			  my_island.state2_count+=my_control.encoderl;   
		    // my_island.monotonicity_change_line[1]=my_image.Right_Line[my_island.monotonicity_change_line[0]];//角点的行列
		     if(my_island.state2_count>=15000)// && my_island.monotonicity_change_line[0]>=60) //找到的单调点过于向下，开始进入
            {
							   my_order.beep_count=0;
                my_island.island_state =3;
							  my_island.state3_count=0;
            }	//130000  
		}
   	if(my_island.island_state==3)//拉环
		{
			  beep_on();
			  my_island.state3_count+=my_control.encoderl;   
					if(my_island.state3_count>=18000)
					{
						 my_order.beep_count=0;
					   my_island.island_state =4;
					   my_island.state4_count=0;
					
					}

    }	
   if(my_island.island_state==4)//内部
		{		
			  beep_on();
      	  my_island.state4_count+=my_control.encoderl;   
					if(my_island.state4_count>5000)
					{
					   my_island.island_state =5;
					   my_island.state5_count=0;
					
					}

		}
		 if(my_island.island_state==5)//拉环
		{		
			   beep_on();
      	  my_island.state5_count+=my_control.encoderl;   
					if(my_island.state5_count>=20000)
					{
					   my_island.island_state =6;
					
					}

		}
		
		 
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     左下角点检测
  @param     起始点，终止点
  @return    返回角点所在的行数，找不到返回0
  Sample     Find_Left_Down_Point(int start,int end);
  @note      角点检测阈值可根据实际值更改
-------------------------------------------------------------------------------------------------------------------*/
void Find_Left_Down_Point(int start,int end)//找四个角点，返回值是角点所在的行数
{
    int i,t;
    int left_down_line=0;
    if(my_image.Right_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没有拐点判断的意义
        my_island.left_down_line[0]=left_down_line;
		    my_island.left_down_line[1]=my_image.Left_Line[my_island.left_down_line[0]];
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
        if(left_down_line==0&&//只找第一个符合条件的点
           abs(my_image.Left_Line[i]-my_image.Left_Line[i+1])<=5&&//角点的阈值可以更改
           abs(my_image.Left_Line[i+1]-my_image.Left_Line[i+2])<=5&&  
           abs(my_image.Left_Line[i+2]-my_image.Left_Line[i+3])<=5&&
              (my_image.Left_Line[i]-my_image.Left_Line[i-2])>=5&&
              (my_image.Left_Line[i]-my_image.Left_Line[i-3])>=10&&
              (my_image.Left_Line[i]-my_image.Left_Line[i-4])>=10)
        {
            left_down_line=i;//获取行数即可
            break;
        }
    }
		    my_island.left_down_line[0]=left_down_line;
		my_island.left_down_line[1]=my_image.Left_Line[my_island.left_down_line[0]];

    
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
    int right_down_line=80;
    if(my_image.Right_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没有拐点判断的意义
        my_island.right_down_line[0]=right_down_line;
		    my_island.right_down_line[1]=my_image.Right_Line[my_island.right_down_line[0]];
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
        if(right_down_line==80&&//只找第一个符合条件的点
           abs(my_image.Right_Line[i]-my_image.Right_Line[i+1])<=7&&//角点的阈值可以更改
           abs(my_image.Right_Line[i+1]-my_image.Right_Line[i+2])<=7&&  
           abs(my_image.Right_Line[i+2]-my_image.Right_Line[i+3])<=7&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-2])<=-5&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-3])<=-6&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-4])<=-6)
        {
            right_down_line=i+3;//获取行数即可
            break;
        }
//				if(right_down_line==80&&//只找第一个符合条件的点
//           abs(my_image.Right_Line[i]-my_image.Right_Line[i+1])<=5&&//角点的阈值可以更改
//           abs(my_image.Right_Line[i+1]-my_image.Right_Line[i+2])<=5&&  
//           abs(my_image.Right_Line[i+2]-my_image.Right_Line[i+3])<=5&&
//           abs(my_image.Right_Line[i]-my_image.Right_Line[i-2])>=8&&
//           abs(my_image.Right_Line[i]-my_image.Right_Line[i-3])>=12&&
//           abs(my_image.Right_Line[i]-my_image.Right_Line[i-4])>=12)
//        {
//            right_down_line=i;//获取行数即可
//            break;
//        }
    }
    my_island.right_down_line[0]=right_down_line;
		my_island.right_down_line[1]=my_image.Right_Line[my_island.right_down_line[0]];
}

/*
 * 斑马线检测函数，检测屏幕黑白跳变数，阈值可调，也要注意判断范围，近车靠头还是远离车头
 */
void Zebra_Detect(void)
{
    int i,j;
    int change=0;
    for(i=70;i>=68;i--)
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
    if(change>=25)
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
			  my_image.continuity_change_flag_right=0;
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
        if(abs(my_image.Right_Line[i]-my_image.Right_Line[i-5])>=7)//连续性阈值是5，可更改
       {
            my_image.continuity_change_flag_right=i;
            break;  
       }
			 else
			 {
			      my_image.continuity_change_flag_right=0;

			 }
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     左赛道连续性检测
  @param     起始点，终止点
  @return    连续返回0，不连续返回断线出行数
  Sample     Continuity_Change_Left(int start,int end);
  @note      连续性的阈值设置为5，可更改
-------------------------------------------------------------------------------------------------------------------*/
void Continuity_Change_Left(int start,int end)//连续性阈值设置为5
{
    int i;
    int t;
    if(my_image.Both_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没必要判断了
       my_image.continuity_change_flag_left=0;
    if(my_image.Search_Stop_Line<=5)//搜所截止行很矮
       my_image.continuity_change_flag_left=0;
    if(start>=MT9V03X_H-1-5)//数组越界保护
        start=MT9V03X_H-1-5;
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
       if(abs(my_image.Left_Line[i]-my_image.Left_Line[i-2])>=7)//连续判断阈值是5,可更改
       {
            my_image.continuity_change_flag_left=i;
            break;
       }
			 else
			{
			         my_image.continuity_change_flag_left=0;
  
			
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
//找角点
void Monotonicity_Change_Right(int start,int end)//单调性改变，返回值是单调性改变点所在的行数,从下到上
{
    int i;
    int temp;
//    if(my_image.Right_Lost_Counter>=0.9*MT9V03X_H)//大部分都丢线，没有单调性判断的意义
//		{
//        my_island.monotonicity_change_line[0]=0;
//		}
    if(start>=MT9V03X_H-1-5)//数组越界保护
        start=MT9V03X_H-1-5;
     if(end<=5)
        end=5;
    if(start<=end)
		{
      temp=end;
		   end=start;
		   start=temp;
		}
    for(i=start;i>=end;i--)//会读取前5后5数据，所以前面对输入范围有要求
    {
     
//        if(my_image.Right_Line[i] <my_image.Right_Line[i+5]&&my_image.Right_Line[i] <my_image.Right_Line[i-5]&&
//        my_image.Right_Line[i] <my_image.Right_Line[i+4]&&my_image.Right_Line[i] <my_image.Right_Line[i-4]&&
//        my_image.Right_Line[i]<=my_image.Right_Line[i+3]&&my_image.Right_Line[i]<=my_image.Right_Line[i-3]&&
//        my_image.Right_Line[i]<=my_image.Right_Line[i+2]&&my_image.Right_Line[i]<=my_image.Right_Line[i-2]&&
//        my_image.Right_Line[i]<=my_image.Right_Line[i+1]&&my_image.Right_Line[i]<=my_image.Right_Line[i-1])
//        {//就很暴力，这个数据是在前5，后5中最大的，那就是单调突变点
//            my_island.monotonicity_change_line[0]=i;
//            break;
//        }
//                if(my_image.Right_Line[i] <my_image.Right_Line[i+8]&&my_image.Right_Line[i] <my_image.Right_Line[i-8]&&
//			  my_image.Right_Line[i] <my_image.Right_Line[i+7]&&my_image.Right_Line[i] <my_image.Right_Line[i-7]&&
//			  my_image.Right_Line[i] <my_image.Right_Line[i+6]&&my_image.Right_Line[i] <my_image.Right_Line[i-6]&&
//				my_image.Right_Line[i] <my_image.Right_Line[i+5]&&my_image.Right_Line[i] <my_image.Right_Line[i-5]&&
//        my_image.Right_Line[i] <my_image.Right_Line[i+4]&&my_image.Right_Line[i] <my_image.Right_Line[i-4]&&
//        my_image.Right_Line[i]<=my_image.Right_Line[i+3]&&my_image.Right_Line[i]<=my_image.Right_Line[i-3]&&
//        my_image.Right_Line[i]<=my_image.Right_Line[i+2]&&my_image.Right_Line[i]<=my_image.Right_Line[i-2]&&
//        my_image.Right_Line[i]<=my_image.Right_Line[i+1]&&my_image.Right_Line[i]<=my_image.Right_Line[i-1])
//        {//就很暴力，这个数据是在前5，后5中最大的，那就是单调突变点
//            my_island.monotonicity_change_line[0]=i;
//            break;
//        }
            if(abs(my_image.Right_Line[i]-my_image.Right_Line[i-5])<=10&&abs(my_image.Right_Line[i]-my_image.Right_Line[i+5])<=14)//如果当前点与前后5个点相差小于10
        {
            if(my_image.Right_Line[i]==my_image.Right_Line[i-5]&&my_image.Right_Line[i]==my_image.Right_Line[i+5]&&
            my_image.Right_Line[i]==my_image.Right_Line[i-4]&&my_image.Right_Line[i]==my_image.Right_Line[i+4]&&
            my_image.Right_Line[i]==my_image.Right_Line[i-3]&&my_image.Right_Line[i]==my_image.Right_Line[i+3]&&
            my_image.Right_Line[i]==my_image.Right_Line[i-2]&&my_image.Right_Line[i]==my_image.Right_Line[i+2]&&
            my_image.Right_Line[i]==my_image.Right_Line[i-1]&&my_image.Right_Line[i]==my_image.Right_Line[i+1])
            {
                continue;//如果当前点与前后5个点相等，继续
            }
            else if(my_image.Right_Line[i]<=my_image.Right_Line[i-5]&&my_image.Right_Line[i]<=my_image.Right_Line[i+5]&&
                    my_image.Right_Line[i]<=my_image.Right_Line[i-4]&&my_image.Right_Line[i]<=my_image.Right_Line[i+4]&&
                    my_image.Right_Line[i]<=my_image.Right_Line[i-3]&&my_image.Right_Line[i]<=my_image.Right_Line[i+3]&&
                    my_image.Right_Line[i]<=my_image.Right_Line[i-2]&&my_image.Right_Line[i]<=my_image.Right_Line[i+2]&&
                    my_image.Right_Line[i]<=my_image.Right_Line[i-1]&&my_image.Right_Line[i]<=my_image.Right_Line[i+1])
            {
                 my_island.monotonicity_change_line[0]=i;
                break;
            }
        }

        
    }
   
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     通过斜率，定点补线
  @param     k       输入斜率
             startY  输入起始点纵坐标
             endY    结束点纵坐标
  @return    null    直接补边线
  Sample     K_Add_Boundry_Right(float k,int startY,int endY);
  @note      补得线直接贴在边线上
-------------------------------------------------------------------------------------------------------------------*/
void K_Add_Boundry_Right(float k,int startX,int startY,int endY)
{
    int i,t;
    if(startY>=MT9V03X_H-1)
        startY=MT9V03X_H-1;
    else if(startY<=0)
        startY=0;
    if(endY>=MT9V03X_H-1)
        endY=MT9V03X_H-1;
    else if(endY<=0)
        endY=0;
    if(startY<endY)
    {
        t=startY;
        startY=endY;
        endY=t;
    }
    for(i=startY;i>=endY;i--)
    {
        my_image.Right_Line[i]=(int)((i-startY)/k+startX);//(y-y1)=k(x-x1)变形，x=(y-y1)/k+x1
        if(my_image.Right_Line[i]>=MT9V03X_W-1)
        {
            my_image.Right_Line[i]=MT9V03X_W-1;
        }
        else if(my_image.Right_Line[i]<=0)
        {
            my_image.Right_Line[i]=0;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     画线
  @param     输入起始点，终点坐标，补一条宽度为2的黑线
  @return    null
  Sample     Draw_Line(0, 0,MT9V03X_W-1,MT9V03X_H-1);
             Draw_Line(MT9V03X_W-1, 0,0,MT9V03X_H-1);
                                    画一个大×
  @note     补的就是一条线，需要重新扫线
-------------------------------------------------------------------------------------------------------------------*/
void Draw_Line(int startX, int startY, int endX, int endY)
{
    int i,x,y;
    int start=0,end=0;
    if(startX>=MT9V03X_W-1)//限幅处理
        startX=MT9V03X_W-1;
    else if(startX<=0)
        startX=0;
    if(startY>=MT9V03X_H-1)
        startY=MT9V03X_H-1;
    else if(startY<=0)
        startY=0;
    if(endX>=MT9V03X_W-1)
        endX=MT9V03X_W-1;
    else if(endX<=0)
        endX=0;
    if(endY>=MT9V03X_H-1)
        endY=MT9V03X_H-1;
    else if(endY<=0)
        endY=0;
    if(startX==endX)//一条竖线
    {
        if (startY > endY)//互换
        {
            start=endY;
            end=startY;
        }
        for (i = start; i <= end; i++)
        {
            if(i<=1)
                i=1;
            my_image.image_two_value[i][startX]=IMG_BLACK;
            my_image.image_two_value[i-1][startX]=IMG_BLACK;
        }
    }
    else if(startY == endY)//补一条横线
    {
        if (startX > endX)//互换
        {
            start=endX;
            end=startX;
        }
        for (i = start; i <= end; i++)
        {
            if(startY<=1)
                startY=1;
            my_image.image_two_value[startY][i]=IMG_BLACK;
            my_image.image_two_value[startY-1][i]=IMG_BLACK;
        }
    }
    else //上面两个是水平，竖直特殊情况，下面是常见情况
    {
			if(startY>endY)//起始点矫正 从上到下
        {
            start=endY;
            end=startY;
        }
        else
        {
            start=startY;
            end=endY;
        }
        for (i = start; i <= end; i++)//纵向补线，保证每一行都有黑点
        {
            x =(int)(startX+(endX-startX)*(i-startY)/(endY-startY));//两点式变形
            if(x>=MT9V03X_W-1)
                x=MT9V03X_W-1;
            else if (x<=1)
                x=1;
            my_image.image_two_value[i][x] = IMG_BLACK;
            my_image.image_two_value[i][x-1] = IMG_BLACK;
        }
        if(startX>endX)
        {
            start=endX;
            end=startX;
        }
        else
        {
            start=startX;
            end=endX;
        }
        for (i = start; i <= end; i++)//横向补线，保证每一列都有黑点
        {
 
            y =(int)(startY+(endY-startY)*(i-startX)/(endX-startX));//两点式变形
            if(y>=MT9V03X_H-1)
                y=MT9V03X_H-1;
            else if (y<=0)
                y=0;
            my_image.image_two_value[y][i] = IMG_BLACK;
        }
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     根据斜率划线
  @param     输入斜率，定点，画一条黑线
  @return    null
  Sample     K_Draw_Line(k, 20,MT9V03X_H-1 ,0)
  @note      补的就是一条线，需要重新扫线
-------------------------------------------------------------------------------------------------------------------*/
void K_Draw_Line(float k, int startX, int startY,int endY)
{
    int endX=0;
 
    if(startX>=MT9V03X_W-1)//限幅处理
        startX=MT9V03X_W-1;
    else if(startX<=0)
        startX=0;
    if(startY>=MT9V03X_H-1)
        startY=MT9V03X_H-1;
    else if(startY<=0)
        startY=0;
    if(endY>=MT9V03X_H-1)
        endY=MT9V03X_H-1;
    else if(endY<=0)
        endY=0;
    endX=(int)((endY-startY)/k+startX);//(y-y1)=k(x-x1)变形，x=(y-y1)/k+x1
    Draw_Line(startX,startY,endX,endY);
}

/**
 * 斜截式线性插值函数
 * 在给定的行范围内计算列坐标并存入左边线数组
 * 
 * @param begin   起始行号 (y坐标，int类型)
 * @param end     结束行号 (y坐标，int类型)
 * @param y_begin 起始列号 (x坐标，int类型)
 * @param y_end   结束列号 (x坐标，int类型)
 * @param Left_Line 左边线数组(行号索引存储列号，volatile int类型)
 */
void xieji(int begin, int end, int y_begin, int y_end)
{
    float xielv;  // 直线斜率
    float jieju;  // 直线截距
    
    // 处理垂直线特殊情况(斜率无穷大)
    if (end == begin)  
    {
        // 在行范围内所有点设置为相同列值
        for (int i = begin; i < end; i++)
        {
            my_image.Left_Line[i] = y_begin;
            // 边界保护处理
            if (my_image.Left_Line[i] <= 1)                     // 防止左溢出
                my_image.Left_Line[i] = 1;
            else if (my_image.Left_Line[i] >= MT9V03X_W - 2)    // 防止右溢出
                my_image.Left_Line[i] = MT9V03X_W - 2;
        }
        return;
    }
    
    // 计算直线斜率和截距(浮点运算)
    xielv = (float)(y_end - y_begin) / (float)(end - begin);  // 斜率公式: k=(y2-y1)/(x2-x1)
    jieju = y_begin - xielv * begin;                         // 截距公式: b=y1-k*x1
    
    // 在起止行号之间进行线性插值
    for (int i = begin; i < end; i++)
    {
        // 四舍五入取整后存入左边线数组
        my_image.Left_Line[i] = (int)(xielv * i + jieju + 0.5f);  // y = k*x + b
        
        // 边界保护处理
        if (my_image.Left_Line[i] <= 1)                     // 最小列号限制为1
            my_image.Left_Line[i] = 1;
        else if (my_image.Left_Line[i] >= MT9V03X_W - 2)    // 最大列号限制为图像宽度-2
            my_image.Left_Line[i] = MT9V03X_W - 2;
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     通过斜率，定点补线--
  @param     k       输入斜率
             startY  输入起始点纵坐标
             endY    结束点纵坐标
  @return    null
  Sample     K_Add_Boundry_Left(float k,int startY,int endY);
  @note      补得线直接贴在边线上
-------------------------------------------------------------------------------------------------------------------*/
void K_Add_Boundry_Left(float k,int startX,int startY,int endY)
{
    int i,t;
    if(startY>=MT9V03X_H-1)
        startY=MT9V03X_H-1;
    else if(startY<=0)
        startY=0;
    if(endY>=MT9V03X_H-1)
        endY=MT9V03X_H-1;
    else if(endY<=0)
        endY=0;
    if(startY<endY)//--操作，start需要大
    {
        t=startY;
        startY=endY;
        endY=t;
    }
    for(i=startY;i>=endY;i--)
    {
        my_image.Left_Line[i]=(int)((i-startY)/k+startX);//(y-y1)=k(x-x1)变形，x=(y-y1)/k+x1
        if(my_image.Left_Line[i]>=MT9V03X_W-1)
        {
            my_image.Left_Line[i]=MT9V03X_W-1;
        }
        else if(my_image.Left_Line[i]<=0)
        {
            my_image.Left_Line[i]=0;
        }
    }
}
