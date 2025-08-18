#include "motor.h"
#include "image.h"
#include "control.h"
#include "obstacle.h"

#include <math.h>

#define BEEP                (D7 )
order my_order={
  .go=0 ,
	.cross=0,
	.island=0,
	.ramp=0,
  .count_2s=0,
	.count_1s=0,
	.black=0,
	.start=0,
	.add=29       ,
	.zebra=0,
	.beep_count=0,
	.count=0,
	.show=1,
	.err_k=2.0,
	.page=1,  
	.encorder_time=0
};
island my_island={
  .right_down_line={80},
	.left_down_line={80},
	.Left_Up_Guai={0},
	.island_state=0  ,
	.state1_count=0,
	.state2_count=0,
  .state3_count=0,
	.state4_count=0,
	.state5_count=0,
	.open=1   ,
	.monotonicity_change_line_right={0},
	.monotonicity_change_line_left={0},
	.k=0,
	.detect=0,
	.gyro_x_angle=0,
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
   if(my_order.beep_count>=2)
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
	 //开始的时候清零
 //  my_order.cross=0;
	
	 my_image.Left_Up_Find=0;
	 my_image.Right_Up_Find=0;  
   Find_Up_Point( MT9V03X_H-1, 0 );	
	if(my_order.island == 0)
	{
		
    if(my_image.Left_Lost_Counter>=15 && my_image.Right_Lost_Counter>=15)
		{
			//  Find_Up_Point( MT9V03X_H-1, 0 );
			if(my_image.Left_Up_Find!=0 || my_image.Right_Up_Find!=0) //缺线并且找到角点
				{
				     my_order.cross=1;
				}
			else
			{
			       my_order.cross=0;
			}
				
			
		}
		else
			{
			       my_order.cross=0;
			}
	}
	else
	{
	   my_order.cross=0;
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
int black_stop(){
	uint16 i,j,count=0;
	for(i =MT9V03X_H-1;i>MT9V03X_H-2;i--)
  {
      {
      for(j = 30;j<MT9V03X_W-30;j++)
          if((my_image.image_two_value[i][j]==0&&my_image.image_two_value[i][j+1]==255)||(my_image.image_two_value[i][j]==255&&my_image.image_two_value[i][j+1]==0))
         count++;
      }
  }
	if(count>=8)
		return 1;
	else
		return 0;
}
void zebra()
{
       if(black_stop()&&my_order.zebra==0){
					   my_order.zebra=1;
					}
					if(my_order.zebra==1&&black_stop()==0){
						my_order.zebra=2;
					}
					if(my_order.zebra==2&&black_stop()){
						my_order.zebra=3;
					}
}

void Ramp_Detect(void)
{
    int i=0;
    int count=0;

    if(my_image.Search_Stop_Line>=75    )//截止行长
    {
        for(i=MT9V03X_H-1;i>MT9V03X_H-my_image.Search_Stop_Line;i--)//赛宽过长计数
        {
            if(my_image.Road_Wide[i]-Standard_Road_Wide[i]>10)//图像赛宽比标准赛宽大
            {
                count++;//赛宽过宽行
            }
        }

    }
    if(count>=20 && my_abs(my_control.err)<=5 && my_order.cross ==0  && my_image.Right_Lost_Counter<=15 && my_image.Left_Lost_Counter <=15    )//赛道过宽超过某一阈值
    { 
			 my_order.ramp=1;   
    }
		else
		{
		   my_order.ramp=0;
		}
			
			

	
//赛道超宽行计数，debug使用
//ips200_show_int(50,10*16,count,5);
}
void island_detect()
{
        // 初始化检测标志
	if(my_island.detect==0)
	{
    my_image.continuity_change_flag_left = 0;
    my_image.continuity_change_flag_right = 0;
    my_island.monotonicity_change_line_right[0] = 0;
    my_island.monotonicity_change_line_right[1] = 0;
		my_island.monotonicity_change_line_left[0]=0;
		my_island.monotonicity_change_line_left[1]=0;
    // 执行边界检测（修改为右侧检测）
    Continuity_Change_Left(30, MT9V03X_H-1-5-5);
    Continuity_Change_Right(30, MT9V03X_H-1-5-5);
    Monotonicity_Change_Right(30, 70);  // 修改为右侧单调性检测
	  Monotonicity_Change_Left(30,70);
		if(my_image.continuity_change_flag_right >= 20 &&    // 右边不连续
               my_image.continuity_change_flag_left <= 5 &&     // 左边连续
               my_image.Right_Lost_Counter >= 12 &&             // 右丢线多
               my_image.Right_Lost_Counter <= 50 &&            // 右丢线数量合理
               my_image.Left_Lost_Counter <= 5 &&               // 左丢线少
               my_image.Search_Stop_Line >= 79 &&              // 视野足够远
               my_image.Both_Lost_Counter <= 5)
		{
		   my_island.detect=2;//右圆环
		}
		else if(       
                my_image.continuity_change_flag_left >= 20 &&    // 左边不连续
                my_image.continuity_change_flag_right <= 5 &&    // 右边连续
                my_image.Left_Lost_Counter >= 12 &&             // 左丢线多
                my_image.Left_Lost_Counter <= 50 &&             // 左丢线数量在合理范围
                my_image.Right_Lost_Counter <= 5 &&              // 右丢线少
                my_image.Search_Stop_Line >= 79 &&               // 视野足够远
                my_image.Both_Lost_Counter <= 5)
		{
			 my_island.detect=1;//左圆环
		}
	}

}
/**
* @brief 右圆环检测函数
*/
void island_detect_right()
{
    // 初始化检测标志
    my_image.continuity_change_flag_left = 0;
    my_image.continuity_change_flag_right = 0;
    my_island.right_down_line[0] = 0;
    my_island.right_down_line[1] = 0;
    my_island.monotonicity_change_line_right[0] = 0;
    my_island.monotonicity_change_line_right[1] = 0;

    // 执行边界检测（修改为右侧检测）
    Continuity_Change_Left(30, MT9V03X_H-1-5-5);
    Continuity_Change_Right(30, MT9V03X_H-1-5-5);
    Monotonicity_Change_Right(30, 70);  // 修改为右侧单调性检测

    // 只在非十字、非坡道状态下检测
    if(my_order.cross == 0 && my_order.ramp == 0) 
    {
        // 获取右上特征点（修改为右侧）
        my_island.monotonicity_change_line_right[1] = 
            my_image.Right_Line[my_island.monotonicity_change_line_right[0]];

        // 状态0：初始检测右圆环入口特征（条件对称修改）
        if(my_island.island_state == 0 && my_abs(my_control.err) <= 20)
        {
            if(my_image.continuity_change_flag_right >= 20 &&    // 右边不连续
               my_image.continuity_change_flag_left <= 5 &&     // 左边连续
               my_image.Right_Lost_Counter >= 12 &&             // 右丢线多
               my_image.Right_Lost_Counter <= 50 &&            // 右丢线数量合理
               my_image.Left_Lost_Counter <= 5 &&               // 左丢线少
               my_image.Search_Stop_Line >= 79 &&              // 视野足够远
               my_image.Both_Lost_Counter <= 5)
            {
                my_island.island_state = 1;  // 进入状态1
                my_island.state1_count = 0;
                my_order.beep_count = 0;
            }
        }
        
        // 状态1：寻找右下方拐点（对称修改）
        if(my_island.island_state == 1)
        {
            beep_on();
            if(my_image.Boundry_Start_Right < 50)  // 右下角点消失
            {
                my_order.beep_count = 0;
                my_island.island_state = 2;
                my_island.state2_count = 0;
            }
        }
        
        // 状态2：进入圆环曲线（修改为右侧逻辑）
        if(my_island.island_state == 2)
        {
            beep_on();
            my_island.state2_count += my_control.encoderl;
            
            if(my_island.state2_count >= 400)  // 编码器计数触发
            {
                my_order.beep_count = 0;
                my_island.island_state = 3;
                my_island.gyro_x_angle = 0;  // 重置陀螺仪角度
                my_island.state3_count = 0;
            }
        }
        
        // 状态3：沿圆环行驶（陀螺仪角度判断方向相反）
        if(my_island.island_state == 3)
        {
            beep_on();
            my_island.state3_count += my_control.encoderl;
            
            if(my_island.gyro_x_angle <= -340)  // 右转角度为负值
            {
                my_order.beep_count = 0;
                my_island.island_state = 4;
                my_island.state4_count = 0;
            }
        }
        
        // 状态4：圆环内部（逻辑相同）
        if(my_island.island_state == 4)
        {
            beep_on();
            my_island.state4_count += my_control.encoderl;
            
            if(my_island.state4_count >= 2000)
            {
                my_order.beep_count = 0;
                my_island.island_state = 5;
                my_island.state5_count = 0;
            }
        }
        
        // 状态5：驶出圆环（逻辑相同）
        if(my_island.island_state == 5)
        {
            beep_on();
            my_island.state5_count += my_control.encoderl;
            
            if(my_island.state5_count >= 1000)
            {
                my_order.beep_count = 0;
                my_island.island_state = 0;
                my_island.state5_count = 0;
            }
        }
    }
}
void island_detect_left()
{
	//每次进入的时候都会清零然后寻找，相对每次更改，这样逻辑上更清晰
	  my_image.continuity_change_flag_left=0;
	  my_image.continuity_change_flag_right=0;
		my_island.left_down_line[0]=0;
		my_island.left_down_line[1]=0;
	  my_island.monotonicity_change_line_left[0]=0;
		my_island.monotonicity_change_line_left[1]=0;
    Continuity_Change_Right(30,MT9V03X_H-1-5-5);
    Continuity_Change_Left(30,MT9V03X_H-1-5-5);
	  Monotonicity_Change_Left(30,70);
    // 左右连续性检测
    if(my_order.cross == 0 && my_order.ramp == 0)
    {
//			 Find_Left_Down_Point(0,MT9V03X_H-1);
//				Continuity_Change_Right(30,MT9V03X_H-1-5-5);
//         Continuity_Change_Left(30,MT9V03X_H-1-5-5);
////				 Monotonicity_Change_Right(10,70);
//					 Monotonicity_Change_Left(10,70);
	//				Find_Left_Down_Point(0,MT9V03X_H-1);
		//		 Find_Right_Down_Point(MT9V03X_H-1,20);//右下点
	//			  Find_Left_Down_Point(MT9V03X_H-1,0);//找四个角点，返回值是角点所在的行数
		//			Find_Up_Point( MT9V03X_H-1, 0 );	
		//	    Find_Down_Point( MT9V03X_H-1, 0 );
//				 my_island.monotonicity_change_line_right[1]=my_image.Right_Line[my_island.monotonicity_change_line_right[0]];//角点的行列
							my_island.monotonicity_change_line_left[1]=my_image.Left_Line[my_island.monotonicity_change_line_left[0]];//角点的行列
        // 状态0：初始状态检测左圆环特征
			//                my_image.Boundry_Start_Right >= MT9V03X_H-20 && // 右边界起始点靠下            my_image.Boundry_Start_Left >= MT9V03X_H-20 &&   // 左边界起始点靠下
        if(my_island.island_state == 0 && my_abs(my_control.err) <= 20)
        {
					  //  Find_Left_Down_Point(MT9V03X_H-1,0);
            if(       
                my_image.continuity_change_flag_left >= 20 &&    // 左边不连续
                my_image.continuity_change_flag_right <= 5 &&    // 右边连续
                my_image.Left_Lost_Counter >= 12 &&             // 左丢线多
                my_image.Left_Lost_Counter <= 50 &&             // 左丢线数量在合理范围
                my_image.Right_Lost_Counter <= 5 &&              // 右丢线少
                my_image.Search_Stop_Line >= 79 &&               // 视野足够远
                my_image.Both_Lost_Counter <= 5)// && my_island.left_down_line[0]>=15  )  
            {
                my_island.island_state = 1;  // 进入状态1
                my_island.state1_count = 0;
                my_order.beep_count = 0;
            }
        }       
        // 状态1：寻找左下方拐点
        if(my_island.island_state == 1)
        {
            beep_on();
//            my_island.state1_count += my_control.encoderl;
            
            // 使用左边界线数据
    //        my_island.monotonicity_change_line_left[1] = my_image.Left_Line[my_island.monotonicity_change_line_left[0]];
            //my_island.state1_count >= 8000 && 
//            if(
//               my_island.monotonicity_change_line_left[0] >= 20 && 
//               my_island.monotonicity_change_line_left[0] <= 60 && my_island.left_down_line[0]<=15 &&  my_island.state1_count>=5000)
//            {
					  if(my_image.Boundry_Start_Left<50)//使用左下角点消失的时候，也就是坐下丢线的起始点是很下方的
					  {
                my_order.beep_count = 0;
                my_island.island_state = 2;
                my_island.state2_count = 0;
            }
        }
        
        // 状态2：进入圆环曲线
        if(my_island.island_state == 2)
        {
            beep_on();
            my_island.state2_count += my_control.encoderl;
				//	  my_image.Left_Up_Find=60;
        //    Find_Up_Point( MT9V03X_H-1, 0 );
			//		L_duan_V();
			//		  Find_Left_Up_Point_new(MT9V03X_H-1, 0 );
//            if(my_image.Left_Up_Find>=25)
//            {
//                my_order.beep_count = 0;
//                my_island.island_state = 3;
//							  my_island.gyro_x_angle=0;
//                my_island.state3_count = 0;
//            }
					if(my_island.state2_count>=300)
					{
					      my_order.beep_count = 0;
                my_island.island_state = 3;
							  my_island.gyro_x_angle=0;
                my_island.state3_count = 0;
					}
        }
        
        // 状态3：沿圆环行驶
        if(my_island.island_state == 3)
        {
            beep_on();
					  
            my_island.state3_count += my_control.encoderl;
//					 if(my_image.Left_Up_Find<=5 &&  my_island.state3_count>=2500)  //5000
//            {
//                my_order.beep_count = 0;
//                my_island.island_state = 4;
//                my_island.state4_count = 0;
//            }
					   if(my_island.gyro_x_angle>=340)  //5000
            {
                my_order.beep_count = 0;
                my_island.island_state = 4;
                my_island.state4_count = 0;
            }

//            Find_Up_Point( MT9V03X_H-1, 0 );	
//             if(my_island.k!=0)  //已经进入过状态了
//            {
//                K_Draw_Line(my_island.k,MT9V03X_W-30,MT9V03X_H-1,0);//k是刚刚算出来的，静态变量存着
//                Longest_White_Column();//刷新边界数据
//            }
//            else
//            {
//                my_island.Left_Up_Guai[0]= my_image.Left_Up_Find;
//                my_island.Left_Up_Guai[1]=my_image.Left_Line[my_island.Left_Up_Guai[0]];

////                if (Left_Up_Guai[0]<5)//此处为了防止误判，如果经常从3状态归零，建议修改此处判断条件
////                {
////                    Island_State=0;
////                    Left_Island_Flag=0;
////                }

//                if(my_island.k==0&&(25<=my_island.Left_Up_Guai[0]&&my_island.Left_Up_Guai[0]<50)&&(50<my_island.Left_Up_Guai[1]&&my_island.Left_Up_Guai[1]<110))//拐点出现在一定范围内，认为是拐点出现
//                {
////                    island_state_3_up[0]= Left_Up_Guai[0];
////                    island_state_3_up[1]= Left_Up_Guai[1];
//                    my_island.k=(float)((float)(MT9V03X_H-my_island.Left_Up_Guai[0])/(float)(MT9V03X_W-20-my_island.Left_Up_Guai[1]));
//                    K_Draw_Line(my_island.k,MT9V03X_W-30,MT9V03X_H-1,0);//记录下第一次上点出现时位置，针对这个环岛拉一条死线，入环
//                    Longest_White_Column();//刷新边界数据
//                }
//          }
			  }
//        
        // 状态4：圆环内部
        if(my_island.island_state == 4)
        {
            beep_on();
            my_island.state4_count += my_control.encoderl;
//            
					  if(my_island.state4_count>=2000)
            {
                my_order.beep_count = 0;
                my_island.island_state = 5  ;
                my_island.state5_count = 0;
            }
        }
////        
//        // 状态5：驶出圆环
        if(my_island.island_state == 5)
        {
            beep_on();
            my_island.state5_count += my_control.encoderl;
						if(my_island.state5_count>=1000)
            {
                my_order.beep_count = 0;
                my_island.island_state = 0  ;
                my_island.state5_count = 0;
            }
//            my_island.right_down_line[0]= my_image.Right_Down_Find;
//             my_island.right_down_line[1]= my_image.Right_Line[my_island.right_down_line[0]];
//					  xieji_right(my_island.right_down_line[1], 90, my_island.right_down_line[0], 10);

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
		{
        my_island.left_down_line[0]=left_down_line;
		    my_island.left_down_line[1]=my_image.Left_Line[my_island.left_down_line[0]];
		}
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
		{
        my_island.right_down_line[0]=right_down_line;
		    my_island.right_down_line[1]=my_image.Right_Line[my_island.right_down_line[0]];
		}
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
 
			my_order.zebra+=1;  //遇上斑马线了
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
                 my_island.monotonicity_change_line_right[0]=i;
                break;
            }
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
/**
 * @brief 左边线单调性变化检测（从下往上扫描）
 * @param start 起始行号（图像底部）
 * @param end 结束行号（图像顶部）
 * @note 检测原理：寻找左边线数组中的局部极小值点（凹点）
 */
void Monotonicity_Change_Left(int start, int end)
{
    int i;
    int temp;
    
    /* 边界保护 */
    if(start >= MT9V03X_H-1-5)  // 防止数组越界（保留5行余量）
        start = MT9V03X_H-1-5;
    if(end <= 5)                // 防止数组越界（保留5行余量）
        end = 5;
    
    /* 确保start > end（从下往上扫描） */
    if(start <= end) {
        temp = end;
        end = start;
        start = temp;
    }
    
    /* 主检测循环 */
    for(i = start; i >= end; i--) 
    {
        /* 条件1：当前点与前后5个点的差值在阈值范围内 */
        if(abs(my_image.Left_Line[i] - my_image.Left_Line[i-5]) <= 10 && 
           abs(my_image.Left_Line[i] - my_image.Left_Line[i+5]) <= 14)
        {
            /* 情况1：连续平坦区域（非特征点） */
            if(my_image.Left_Line[i] == my_image.Left_Line[i-5] && 
               my_image.Left_Line[i] == my_image.Left_Line[i+5] &&
               my_image.Left_Line[i] == my_image.Left_Line[i-4] && 
               my_image.Left_Line[i] == my_image.Left_Line[i+4] &&
               my_image.Left_Line[i] == my_image.Left_Line[i-3] && 
               my_image.Left_Line[i] == my_image.Left_Line[i+3] &&
               my_image.Left_Line[i] == my_image.Left_Line[i-2] && 
               my_image.Left_Line[i] == my_image.Left_Line[i+2] &&
               my_image.Left_Line[i] == my_image.Left_Line[i-1] && 
               my_image.Left_Line[i] == my_image.Left_Line[i+1])
            {
                continue;  // 跳过平坦区域
            }
            /* 情况2：检测到局部极小值（凹点） */
            else if(my_image.Left_Line[i] <= my_image.Left_Line[i-5] && 
                    my_image.Left_Line[i] <= my_image.Left_Line[i+5] &&
                    my_image.Left_Line[i] <= my_image.Left_Line[i-4] && 
                    my_image.Left_Line[i] <= my_image.Left_Line[i+4] &&
                    my_image.Left_Line[i] <= my_image.Left_Line[i-3] && 
                    my_image.Left_Line[i] <= my_image.Left_Line[i+3] &&
                    my_image.Left_Line[i] <= my_image.Left_Line[i-2] && 
                    my_image.Left_Line[i] <= my_image.Left_Line[i+2] &&
                    my_image.Left_Line[i] <= my_image.Left_Line[i-1] && 
                    my_image.Left_Line[i] <= my_image.Left_Line[i+1])
            {
                my_island.monotonicity_change_line_left[0] = i;  // 记录行号
                my_island.monotonicity_change_line_left[1] = my_image.Left_Line[i]; // 记录列号
                break;  // 找到第一个特征点即退出
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
/**
 * 斜截式线性插值函数（右边线版本）
 * 在给定的行范围内计算列坐标并存入右边线数组
 * 
 * @param begin    起始行号 (y坐标，int类型)
 * @param end      结束行号 (y坐标，int类型)
 * @param y_begin  起始列号 (x坐标，int类型)
 * @param y_end    结束列号 (x坐标，int类型)
 * @param Right_Line 右边线数组(行号索引存储列号，volatile int类型)
 */
void xieji_right(int begin, int end, int y_begin, int y_end)
{
    float xielv;  // 直线斜率
    float jieju;  // 直线截距
    
    // 处理垂直线特殊情况(斜率无穷大)
    if (end == begin)  
    {
        // 在行范围内所有点设置为相同列值
        for (int i = begin; i < end; i++)
        {
            my_image.Right_Line[i] = y_begin;
            // 边界保护处理
            if (my_image.Right_Line[i] <= 1)                     // 防止左溢出
                my_image.Right_Line[i] = 1;
            else if (my_image.Right_Line[i] >= MT9V03X_W - 2)    // 防止右溢出
                my_image.Right_Line[i] = MT9V03X_W - 2;
        }
        return;
    }
    
    // 计算直线斜率和截距(浮点运算)
    xielv = (float)(y_end - y_begin) / (float)(end - begin);  // 斜率公式: k=(y2-y1)/(x2-x1)
    jieju = y_begin - xielv * begin;                         // 截距公式: b=y1-k*x1
    
    // 在起止行号之间进行线性插值
    for (int i = begin; i < end; i++)
    {
        // 四舍五入取整后存入右边线数组
        my_image.Right_Line[i] = (int)(xielv * i + jieju      );  // y = k*x + b
        
        // 边界保护处理
        if (my_image.Right_Line[i] <= 1)                     // 最小列号限制为1
            my_image.Right_Line[i] = 1;
        else if (my_image.Right_Line[i] >= MT9V03X_W - 2)    // 最大列号限制为图像宽度-2
            my_image.Right_Line[i] = MT9V03X_W - 2;
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
/**
 * @brief 查找左上角点
 * @param start 起始行（图像底部为高行号）
 * @param end 结束行（图像顶部为低行号）
 * @return 左上角点所在的行号，未找到返回0
 */
void Find_Left_Up_Point_new(int start, int end)
{
    int i,t;

    // 参数校验和交换
    if(start > end) {
        t = start;
        start = end;
        end = t;
    }

    // 边界保护
    if(start <= 1) start = 1;                   // 防止数组下越界
    if(end >= MT9V03X_H - 1 - 4) end = MT9V03X_H - 1 - 4; // 防止数组上越界

    // 左上角点检测逻辑（与右上点对称修改）
    for(i = start; i <= end; i++) {
        if(((my_image.Left_Line[i] - my_image.Left_Line[i+2]) >= 8 || my_image.Left_Line[i+2] == 0) &&    // 左边界向左突变
           ((my_image.Left_Line[i] - my_image.Left_Line[i+3]) >= 15 || my_image.Left_Line[i+3] == 0) &&
           ((my_image.Left_Line[i] - my_image.Left_Line[i+4]) >= 15 || my_image.Left_Line[i+4] == 0))
        {
           my_image.Left_Up_Find= i;  // 返回找到的行号

        }
    }

}