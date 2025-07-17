#include "image.h"
#include "control.h"
#include <math.h> 

// 定义并初始化全局的结构体变量 my_image
Image my_image = {0};
//标准赛宽，将车子放在长直道上面实测，以下数值仅供参考
int Standard_Road_Wide[80] = {
    // 第一行（前10个数据保持不变）
    31, 32, 33, 35, 36, 38, 39, 40, 41, 43,

    // 后续行（每行第一个数据保持不变，其余按合理步长递增）
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53,  // 第一列：44（不变），后续+1
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65,  // 第一列：56（不变），后续+1
    69, 70, 71, 72, 73, 74, 75, 76, 77, 78,  // 第一列：69（不变），后续+1
    82, 83, 84, 85, 86, 87, 88, 89, 90, 91,  // 第一列：82（不变），后续+1
    94, 95, 96, 97, 98, 99, 100, 101, 102, 103, // 第一列：94（不变），后续+1
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, // 第一列：107（不变），后续+1
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129  // 第一列：120（不变），后续+1
};
/*-------------------------------------------------------------------------------------------------------------------
  @brief     图像二值化处理函数
  @param     二值化阈值
  @return    NULL
  Sample     Image_Binarization(Threshold);//图像二值化
  @note      二值化后直接访问my_image.image_two_value[i][j]这个数组即可
-------------------------------------------------------------------------------------------------------------------*/
void Image_Binarization(int threshold)//图像二值化
{
    uint16 i, j;
    for (i = 0; i < MT9V03X_H; i++)
    {
        for (j = 0; j < MT9V03X_W; j++)//灰度图的数据只做判断，不进行更改，二值化图像放在了新数组中
        {
            if (mt9v03x_image[i][j] >= threshold)
                my_image.image_two_value[i][j] = IMG_WHITE;//白
            else
                my_image.image_two_value[i][j] = IMG_BLACK;//黑
        }
    }
}
//左右边界除上2
void draw_mid_line()
{
   for(int i=0;i<MT9V03X_H;i++)
	{
	   if(my_image.Right_Lost_Flag[i]==0 && my_image.Left_Lost_Flag[i]==0)
		 {
		    ips200_draw_point ((my_image.Left_Line[i]+my_image.Right_Line[i])>>1, i, RGB565_RED);
		 
		 }
	
	}


}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     普通大津求阈值
  @param     image       图像数组
             width       列 ，宽度
             height      行，长度
  @return    threshold   返回int类型的的阈值
  Sample     threshold = my_adapt_threshold(mt9v03x_image[0], MT9V03X_W, MT9V03X_H);//普通大津
  @note      据说没有山威大津快，我感觉两个区别不大
-------------------------------------------------------------------------------------------------------------------*/
int My_Adapt_Threshold(uint8* image, uint16 width, uint16 height)   //大津算法，注意计算阈值的一定要是原图像
{
    #define GrayScale 256
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j;
    int pixelSum = width * height / 4;
    int  threshold = 0;
    uint8* data = image;  //指向像素数据的指针

    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum = 0;
    for (i = 0; i < height; i += 2)//统计灰度级中每个像素在整幅图像中的个数
    {
        for (j = 0; j < width; j += 2)
        {
            pixelCount[(int)data[i * width + j]]++;  //将当前的点的像素值作为计数数组的下标
            gray_sum += (int)data[i * width + j];       //灰度值总和
        }
    }
    for (i = 0; i < GrayScale; i++) //计算每个像素值的点在整幅图像中的比例
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (j = 0; j < GrayScale; j++)//遍历灰度级[0,255]
    {
        w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和   即背景部分的比例
        u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值
        w1 = 1 - w0;
        u1tmp = gray_sum / pixelSum - u0tmp;
        u0 = u0tmp / w0;              //背景平均灰度
        u1 = u1tmp / w1;              //前景平均灰度
        u = u0tmp + u1tmp;            //全局平均灰度
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);//平方
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;//最大类间方差法
            threshold = j;
        }
        if (deltaTmp < deltaMax)
        {
            break;
        }
    }
    if (threshold > 255)
        threshold = 255;
    if (threshold < 0)
        threshold = 0;
    return threshold;
}



/*-------------------------------------------------------------------------------------------------------------------
  @brief      图像二值化处理函数（支持自定义输入/输出图像和阈值）
  @param      src_image    输入图像（灰度图）
              dst_image    输出图像（二值化图）
              width        图像宽度
              height       图像高度
              threshold    二值化阈值
  @return     void
  @note       不依赖全局变量，完全通过参数传递数据
-------------------------------------------------------------------------------------------------------------------*/
void Image_Binarization2(uint8 (*src_image)[MT9V03X_W], uint8 (*dst_image)[MT9V03X_W], 
                       uint16 width, uint16 height, int threshold) 
{
    for (uint16 i = 0; i < height; i++) 
    {
        for (uint16 j = 0; j < width; j++) 
        {
            dst_image[i][j] = (src_image[i][j] >= threshold) ? IMG_WHITE : IMG_BLACK;
        }
    }
}

/**
 * @brief 分区大津法二值化（每个子区域使用独立阈值）
 * @param src_image    输入图像（二维数组形式）
 * @param dst_image    输出图像（二值化结果）
 * @param width        图像宽度
 * @param height       图像高度
 * @note 图像被分为4个子区域，分别计算阈值后二值化
 */
void Partitioned_Otsu_Binarization(
    uint8 (*src_image)[MT9V03X_W], 
    uint8 (*dst_image)[MT9V03X_W], 
    uint16 width, 
    uint16 height
) {
    // 1. 划分四个子区域
    uint16 sub_width = width / 2;
    uint16 sub_height = height / 2;

    // 2. 计算每个子区域的阈值
    int th_top_left = My_Adapt_Threshold(&src_image[0][0], sub_width, sub_height);
    int th_top_right = My_Adapt_Threshold(&src_image[0][sub_width], sub_width, sub_height);
    int th_bottom_left = My_Adapt_Threshold(&src_image[sub_height][0], sub_width, sub_height);
    int th_bottom_right = My_Adapt_Threshold(&src_image[sub_height][sub_width], sub_width, sub_height);

    // 3. 对每个子区域独立二值化
    for (uint16 i = 0; i < height; i++) {
        for (uint16 j = 0; j < width; j++) {
            // 判断当前像素属于哪个子区域
            if (i < sub_height) {
                if (j < sub_width) {
                    // 左上区域
                    dst_image[i][j] = (src_image[i][j] >= th_top_left) ? IMG_WHITE : IMG_BLACK;
                } else {
                    // 右上区域
                    dst_image[i][j] = (src_image[i][j] >= th_top_right) ? IMG_WHITE : IMG_BLACK;
                }
            } else {
                if (j < sub_width) {
                    // 左下区域
                    dst_image[i][j] = (src_image[i][j] >= th_bottom_left) ? IMG_WHITE : IMG_BLACK;
                } else {
                    // 右下区域
                    dst_image[i][j] = (src_image[i][j] >= th_bottom_right) ? IMG_WHITE : IMG_BLACK;
                }
            }
        }
    }
}



/*-------------------------------------------------------------------------------------------------------------------
  @brief     双最长白列巡线
  @param     null
  @return    null
  Sample     Longest_White_Column()
  @note      最长白列巡线，寻找初始边界，丢线，最长白列等基础元素，后续读取这些变量来进行赛道识别
-------------------------------------------------------------------------------------------------------------------*/
void Longest_White_Column()//最长白列巡线
{
    int i, j = 0;
    int start_column = 20, end_column = MT9V03X_W - 20;
    my_image.Longest_White_Column_Left[0] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    my_image.Longest_White_Column_Left[1] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
	  my_image.shortest_White_Column_Left[0]=70;
	  my_image.shortest_White_Column_Left[1]=0;
    my_image.Longest_White_Column_Right[0] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    my_image.Longest_White_Column_Right[1] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    my_image.Left_Lost_Counter = 0;    //边界丢线数
    my_image.Right_Lost_Counter  = 0;
    my_image.Boundry_Start_Left  = 0;//第一个非丢线点,常规边界起始点
    my_image.Boundry_Start_Right = 0;
    my_image.Both_Lost_Counter = 0;//两边同时丢线数
    // 初始化最长白列数据
    my_image.Longest_White_Column_Left[0] = 0;
    my_image.Longest_White_Column_Left[1] = 0;
    my_image.Longest_White_Column_Right[0] = 0;
    my_image.Longest_White_Column_Right[1] = 0;
    
    // 数据清零,初始化
    for (i = 0; i <= MT9V03X_H - 1; i++)
    {
        my_image.Right_Lost_Flag[i] = 0;
        my_image.Left_Lost_Flag[i] = 0;
        my_image.Left_Line[i] = 0;
        my_image.Right_Line[i] = MT9V03X_W - 1;
    }
		//my_image.Both_Lost_Counter =0;
    for (i = 0; i <= MT9V03X_W - 1; i++)
    {
        my_image.White_Column[i] = 0;
			  my_image.white_line[i]=0;
    }


    // 统计每列白点数量
    for (j = start_column; j <= end_column; j++)
    {
        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (my_image.image_two_value[i][j] == IMG_BLACK)
						{
                my_image.white_line[j]=i;//记录每一列终止点的y坐标
                break;
						}
            else
						{
                my_image.White_Column[j]++;
						}
        }
    }
    
    // 从左到右找左边最长白列
    my_image.Longest_White_Column_Left[0] = 0;
    for (i = start_column; i <= end_column; i++)
    {
			//找长列
        if (my_image.Longest_White_Column_Left[0] < my_image.White_Column[i])
        {
            my_image.Longest_White_Column_Left[0] = my_image.White_Column[i];
            my_image.Longest_White_Column_Left[1] = i;
        }
//				//找短列
				if (my_image.shortest_White_Column_Left[0] > my_image.White_Column[i] && i>=80 && i<=130)
        {
            my_image.shortest_White_Column_Left[0] = my_image.White_Column[i];
            my_image.shortest_White_Column_Left[1] = i;
        }
    }
    
    // 从右到左找右边最长白列
    my_image.Longest_White_Column_Right[0] = 0;
    for (i = end_column; i >= start_column; i--)
    {
			 
        if (my_image.Longest_White_Column_Right[0] < my_image.White_Column[i])
        {
            my_image.Longest_White_Column_Right[0] = my_image.White_Column[i];
            my_image.Longest_White_Column_Right[1] = i;
					
        }
    }
    
    // 设置搜索截止行
    my_image.Search_Stop_Line = my_image.Longest_White_Column_Left[0];
    if(my_island.island_state==3)
		{
		  my_image.Longest_White_Column_Right[1]=140;
		
		}
    // 常规巡线
    for (i = MT9V03X_H - 1; i >= MT9V03X_H - my_image.Search_Stop_Line; i--)
    {
        int left_border = 0, right_border = 0;
        
        // 寻找右边界
        for (j = my_image.Longest_White_Column_Right[1]; j <= MT9V03X_W - 1 - 2; j++)
        {
            if (my_image.image_two_value[i][j] == IMG_WHITE && 
                my_image.image_two_value[i][j + 1] == IMG_BLACK && 
                my_image.image_two_value[i][j + 2] == IMG_BLACK)
            {
                right_border = j;
                my_image.Right_Lost_Flag[i] = 0;
                break;
            }
            else if (j >= MT9V03X_W - 1 - 2)
            {
                right_border = j;
                my_image.Right_Lost_Flag[i] = 1;
                break;
            }
        }
        
        // 寻找左边界
        for (j = my_image.Longest_White_Column_Left[1]; j >= 0 + 2; j--)
        {
            if (my_image.image_two_value[i][j] == IMG_WHITE && 
                my_image.image_two_value[i][j - 1] == IMG_BLACK && 
                my_image.image_two_value[i][j - 2] == IMG_BLACK)
            {
                left_border = j;
                my_image.Left_Lost_Flag[i] = 0;
                break;
            }
            else if (j <= 0 + 2)
            {
                left_border = j;
                my_image.Left_Lost_Flag[i] = 1;
                break;
            }
        }
        
        my_image.Left_Line[i] = left_border;
        my_image.Right_Line[i] = right_border;
    }
    
    // 赛道数据初步分析
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (my_image.Left_Lost_Flag[i] == 1)
            my_image.Left_Lost_Counter++;
        if (my_image.Right_Lost_Flag[i] == 1)
            my_image.Right_Lost_Counter++;
        if (my_image.Left_Lost_Flag[i] == 1 && my_image.Right_Lost_Flag[i] == 1)
            my_image.Both_Lost_Counter++;  
        if (my_image.Boundry_Start_Left == 0 && my_image.Left_Lost_Flag[i] != 1)
            my_image.Boundry_Start_Left = i;
        if (my_image.Boundry_Start_Right == 0 && my_image.Right_Lost_Flag[i] != 1)
            my_image.Boundry_Start_Right = i;
        my_image.Road_Wide[i] = my_image.Right_Line[i] - my_image.Left_Line[i];
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     左补线
  @param     补线的起点，终点
  @return    null
  Sample     Left_Add_Line(int x1,int y1,int x2,int y2);
  @note      补的直接是边界，点最好是可信度高的,不要乱补
-------------------------------------------------------------------------------------------------------------------*/
void Left_Add_Line(int x1,int y1,int x2,int y2)//左补线,补的是边界
{
    int i,max,a1,a2;
    int hx;
    if(x1>=MT9V03X_W-1)//起始点位置校正，排除数组越界的可能
       x1=MT9V03X_W-1;
    else if(x1<=0)
        x1=0;
     if(y1>=MT9V03X_H-1)
        y1=MT9V03X_H-1;
     else if(y1<=0)
        y1=0;
     if(x2>=MT9V03X_W-1)
        x2=MT9V03X_W-1;
     else if(x2<=0)
             x2=0;
     if(y2>=MT9V03X_H-1)
        y2=MT9V03X_H-1;
     else if(y2<=0)
             y2=0;
    a1=y1;
    a2=y2;
    if(a1>a2)//坐标互换
    {
        max=a1;
        a1=a2;
        a2=max;
    }
    for(i=a1;i<=a2;i++)//根据斜率补线即可
    {
        hx=(i-y1)*(x2-x1)/(y2-y1)+x1;
        if(hx>=MT9V03X_W-5)
            hx=MT9V03X_W-5;
        else if(hx<=0)
            hx=5;
        my_image.Left_Line[i]=hx;
		//		ips200_draw_point ((uint16)hx, (uint16)i,RGB565_BLUE);
    }
}

void Right_Add_Line(int x1, int y1, int x2, int y2) //右补线,补的是边界
{
    // 参数校验
    if (y1 == y2) {
        // 水平线处理
        int start_x = (x1 < x2) ? x1 : x2;
        int end_x = (x1 < x2) ? x2 : x1;
        start_x = (start_x < 0) ? 0 : start_x;
        end_x = (end_x >= MT9V03X_W) ? MT9V03X_W-1 : end_x;
        for (int x = start_x; x <= end_x; x++) {
            my_image.Right_Line[y1] = x;
        }
        return;
    }

    // 确保y1 < y2，同时交换对应的x坐标
    if (y1 > y2) {
        int temp = y1; y1 = y2; y2 = temp;
        temp = x1; x1 = x2; x2 = temp;
    }

    // 边界检查
    y1 = (y1 < 0) ? 0 : y1;
    y2 = (y2 >= MT9V03X_H) ? MT9V03X_H-1 : y2;
    
    // 使用浮点运算避免精度丢失
    float slope = (float)(x2 - x1) / (y2 - y1);
    
    for (int y = y1; y <= y2; y++) {
        int hx = (int)(x1 + slope * (y - y1) + 0.5); // 四舍五入
        hx = (hx < 0) ? 0 : hx;
        hx = (hx >= MT9V03X_W) ? MT9V03X_W-1 : hx;
        my_image.Right_Line[y] = hx;
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     左边界延长
  @param     延长起始行数，延长到某行
  @return    null
  Sample     Stop_Detect(void)
  @note      从起始点向上找5个点，算出斜率，向下延长，直至结束点
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Left_Boundry(int start,int end)//起始行，终止行即可//从上到下，点指的是行
{
    int i,t;
    float k=0;
    if(start>=MT9V03X_H-1)//起始点位置校正，排除数组越界的可能
        start=MT9V03X_H-1;
    else if(start<=0)
        start=0;
    if(end>=MT9V03X_H-1)
        end=MT9V03X_H-1;
    else if(end<=0)
        end=0;
    if(end<start)//++访问，坐标互换
    {
        t=end;
        end=start;
        start=t;
    }

    if(start<=5)//因为需要在开始点向上找3个点，对于起始点过于靠上，不能做延长，只能直接连线
    {
         Left_Add_Line(my_image.Left_Line[start],start,my_image.Left_Line[end],end);
    }

    else
    {
        k=(float)(my_image.Left_Line[start]-my_image.Left_Line[start-4])/5.0;//这里的k是1/斜率
        for(i=start;i<=end;i++)
        {
            my_image.Left_Line[i]=(int)(i-start)*k+my_image.Left_Line[start];//(x=(y-y1)*k+x1),点斜式变形
	//				ips200_draw_point ((uint16)my_image.Left_Line[i], (uint16)i,RGB565_BLUE);
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
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     右左边界延长
  @param     延长起始行数，延长到某行
  @return    null
  Sample     Stop_Detect(void)
  @note      从起始点向上找3个点，算出斜率，向下延长，直至结束点
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Right_Boundry(int start,int end)
{
    int i,t;
    float k=0;
    if(start>=MT9V03X_H-1)//起始点位置校正，排除数组越界的可能
        start=MT9V03X_H-1;
    else if(start<=0)
        start=0;
    if(end>=MT9V03X_H-1)
        end=MT9V03X_H-1;
    else if(end<=0)
        end=0;
    if(end<start)//++访问，坐标互换
    {
        t=end;
        end=start;
        start=t;
    }

    if(start<=5)//因为需要在开始点向上找3个点，对于起始点过于靠上，不能做延长，只能直接连线
    {
        Right_Add_Line(my_image.Right_Line[start],start,my_image.Right_Line[end],end);
    }
    else
    {
        k=(float)(my_image.Right_Line[start]-my_image.Right_Line[start-4])/5.0;//这里的k是1/斜率
        for(i=start;i<=end;i++)
        {
            my_image.Right_Line[i]=(int)(i-start)*k+my_image.Right_Line[start];//(x=(y-y1)*k+x1),点斜式变形
			//			ips200_draw_point ((uint16)my_image.Right_Line[i], (uint16)i,RGB565_BLUE);
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
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     绘制赛道边界图
  @param     null
  @return    null
  @note      根据找到的边线生成白色区域和黑色边界，并通过ips200显示
-------------------------------------------------------------------------------------------------------------------*/
void Draw_Track_Boundary()
{
    uint8 track_image[MT9V03X_H][MT9V03X_W];  // 临时存储要显示的图像
    
    // 1. 初始化全白图像
    memset(track_image, IMG_WHITE, sizeof(track_image));
    
    // 2. 绘制左右边界线（黑色）
    for (int i = MT9V03X_H - 1; i >= MT9V03X_H - my_image.Search_Stop_Line; i--)
    {
        // 绘制左边界线（加粗3像素）
        for (int offset = -1; offset <= 1; offset++) {
            int x = my_image.Left_Line[i] + offset;
            if (x >= 0 && x < MT9V03X_W) {
                track_image[i][x] = IMG_BLACK;
            }
        }
        
        // 绘制右边界线（加粗3像素）
        for (int offset = -1; offset <= 1; offset++) {
            int x = my_image.Right_Line[i] + offset;
            if (x >= 0 && x < MT9V03X_W) {
                track_image[i][x] = IMG_BLACK;
            }
        }
    }
    
    // 3. 绘制黑色外边框（上下左右各2像素宽）
    for (int i = 0; i < MT9V03X_H; i++) {
        for (int j = 0; j < 2; j++) {
            track_image[i][j] = IMG_BLACK;                      // 左边界
            track_image[i][MT9V03X_W - 1 - j] = IMG_BLACK;      // 右边界
        }
    }
    for (int j = 0; j < MT9V03X_W; j++) {
        for (int i = 0; i < 2; i++) {
            track_image[i][j] = IMG_BLACK;                      // 上边界
            track_image[MT9V03X_H - 1 - i][j] = IMG_BLACK;      // 下边界
        }
    }
    
    // 4. 显示图像
    ips200_show_gray_image(0, 180, (const uint8 *)track_image, 
                           MT9V03X_W, MT9V03X_H, 
                           MT9V03X_W, MT9V03X_H, 0);
}
// 在图像上绘制左右边界线（蓝点）
void draw_boundary_lines(void) {
    for (int y = 0; y < MT9V03X_H; y++) {
        // 画左边界点（Left_Line[y]）
        if (my_image.Left_Line[y] >= 0 && my_image.Left_Line[y] < MT9V03X_W) {
            ips200_draw_point(my_image.Left_Line[y], y, RGB565_BLUE);
        }
        
        // 画右边界点（Right_Line[y]）
        if (my_image.Right_Line[y] >= 0 && my_image.Right_Line[y] < MT9V03X_W) {
            ips200_draw_point(my_image.Right_Line[y], y, RGB565_BLUE);
        }
    }
		
}
void draw_boundary_lines_wide(void) {
    for (int y = 0; y < MT9V03X_H; y++) {
        // 画左边界点（Left_Line[y]）及左右相邻点
        if (my_image.Left_Line[y] >= 0 && my_image.Left_Line[y] < MT9V03X_W) {
            ips200_draw_point(my_image.Left_Line[y], y, RGB565_BLUE);
            // 向左扩展一个像素
            if (my_image.Left_Line[y] > 0) {
                ips200_draw_point(my_image.Left_Line[y] - 1, y, RGB565_BLUE);
            }
            // 向右扩展一个像素
            if (my_image.Left_Line[y] < MT9V03X_W - 1) {
                ips200_draw_point(my_image.Left_Line[y] + 1, y, RGB565_BLUE);
            }
        }
        
        // 画右边界点（Right_Line[y]）及左右相邻点
        if (my_image.Right_Line[y] >= 0 && my_image.Right_Line[y] < MT9V03X_W) {
            ips200_draw_point(my_image.Right_Line[y], y, RGB565_BLUE);
            // 向左扩展一个像素
            if (my_image.Right_Line[y] > 0) {
                ips200_draw_point(my_image.Right_Line[y] - 1, y, RGB565_BLUE);
            }
            // 向右扩展一个像素
            if (my_image.Right_Line[y] < MT9V03X_W - 1) {
                ips200_draw_point(my_image.Right_Line[y] + 1, y, RGB565_BLUE);
            }
        }
    }
}
/*-------------------------------------------------------------------------------------------------------------------
  @brief     找下面的两个拐点，供十字使用
  @param     搜索的范围起点，终点
  @return    修改两个全局变量
             Right_Down_Find=0;
             Left_Down_Find=0;
  Sample     Find_Down_Point(int start,int end)
  @note      运行完之后查看对应的变量，注意，没找到时对应变量将是0
-------------------------------------------------------------------------------------------------------------------*/
void Find_Down_Point(int start,int end)//从下到上
{
    int i,t;
    my_image.Right_Down_Find=0;
    my_image.Left_Down_Find=0;
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
        if(my_image.Left_Down_Find==0&&//只找第一个符合条件的点
           abs(my_image.Left_Line[i]-my_image.Left_Line[i+1])<=5&&//角点的阈值可以更改
           abs(my_image.Left_Line[i+1]-my_image.Left_Line[i+2])<=5&&
           abs(my_image.Left_Line[i+2]-my_image.Left_Line[i+3])<=5&&
              (my_image.Left_Line[i]-my_image.Left_Line[i-2])>=8&&
              (my_image.Left_Line[i]-my_image.Left_Line[i-3])>=15&&
              (my_image.Left_Line[i]-my_image.Left_Line[i-4])>=15)
        {
            my_image.Left_Down_Find=i;//获取行数即可
										  my_image.cross_count++;
			//		ips200_draw_point (my_image.Left_Line[i], i, RGB565_GREEN);
        }
        if(my_image.Right_Down_Find==0&&//只找第一个符合条件的点
           abs(my_image.Right_Line[i]-my_image.Right_Line[i+1])<=5&&//角点的阈值可以更改
           abs(my_image.Right_Line[i+1]-my_image.Right_Line[i+2])<=5&&
           abs(my_image.Right_Line[i+2]-my_image.Right_Line[i+3])<=5&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-2])<=-8&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-3])<=-15&&
              (my_image.Right_Line[i]-my_image.Right_Line[i-4])<=-15)
        {
            my_image.Right_Down_Find=i;
										  my_image.cross_count++;
			//		ips200_draw_point (my_image.Right_Line[i], i, RGB565_GREEN);
        }
        if(my_image.Left_Down_Find!=0&&my_image.Right_Down_Find!=0)//两个找到就退出
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     找上面的两个拐点，供十字使用
  @param     搜索的范围起点，终点
  @return    修改两个全局变量
             Left_Up_Find=0;
             Right_Up_Find=0;
  Sample     Find_Up_Point(int start,int end)
  @note      运行完之后查看对应的变量，注意，没找到时对应变量将是0
-------------------------------------------------------------------------------------------------------------------*/
void Find_Up_Point(int start,int end)//从下到上扫线
{
    int i,t;
    my_image.Left_Up_Find=0;
    my_image.Right_Up_Find=0;
    if(start<end)
    {
        t=start;
        start=end;
        end=t;
    }
    if(end<=MT9V03X_H-my_image.Search_Stop_Line)
        end=MT9V03X_H-my_image.Search_Stop_Line;
    if(end<=5)//及时最长白列非常长，也要舍弃部分点，防止数组越界
        end=5;
    if(start>=MT9V03X_H-1-5)//下面5行数据不稳定，不能作为边界点来判断，舍弃
        start=MT9V03X_H-1-5;
    for(i=start;i>=end;i--)
    {
        if(my_image.Left_Up_Find==0&&//只找第一个符合条件的点
           abs(my_image.Left_Line[i]-my_image.Left_Line[i-1])<=5&&
           abs(my_image.Left_Line[i-1]-my_image.Left_Line[i-2])<=5&&
           abs(my_image.Left_Line[i-2]-my_image.Left_Line[i-3])<=5&&
              (my_image.Left_Line[i]-my_image.Left_Line[i+2])>=8&&
              (my_image.Left_Line[i]-my_image.Left_Line[i+3])>=15&&
              (my_image.Left_Line[i]-my_image.Left_Line[i+4])>=15)
        {
            my_image.Left_Up_Find=i;//获取行数即可
					  my_image.cross_count++;
		//			  ips200_draw_point (my_image.Left_Line[i], i, RGB565_GREEN);

					
        }
        if(my_image.Right_Up_Find==0&&//只找第一个符合条件的点
           abs(my_image.Right_Line[i]-my_image.Right_Line[i-1])<=5&&//下面两行位置差不多
           abs(my_image.Right_Line[i-1]-my_image.Right_Line[i-2])<=5&&
           abs(my_image.Right_Line[i-2]-my_image.Right_Line[i-3])<=5&&
              (my_image.Right_Line[i]-my_image.Right_Line[i+2])<=-8&&
              (my_image.Right_Line[i]-my_image.Right_Line[i+3])<=-15&&
              (my_image.Right_Line[i]-my_image.Right_Line[i+4])<=-15)
        {
            my_image.Right_Up_Find=i;//获取行数即可
							my_image.cross_count++;
		//			 ips200_draw_point (my_image.Right_Line[i], i, RGB565_GREEN);
        }
        if(my_image.Left_Up_Find!=0&&my_image.Right_Up_Find!=0)//下面两个找到就出去
        {
            break;
        }
    }
    if(abs(my_image.Right_Up_Find-my_image.Left_Up_Find)>=30)//纵向撕裂过大，视为误判
    {
        my_image.Right_Up_Find=0;
        my_image.Left_Up_Find=0;
    }
} 
/* 前进方向定义：
 *   0
 * 3   1
 *   2
 */
/**
 * 自适应阈值二值化
 * @param img_data     输入图像数据(一维数组,行优先存储)
 * @param output_data  输出二值图像(0或255)
 * @param width        图像宽度
 * @param height       图像高度 
 * @param block        邻域窗口大小(必须为奇数)
 * @param clip_value   阈值调整值(通常5-15)
 */
void adaptiveThreshold(uint8_t* img_data, uint8_t* output_data, 
                      int width, int height, int block, uint8_t clip_value)
{
    int half_block = block / 2;
    
    // 遍历图像中心区域(避开边缘)
    for(int y = half_block; y < height - half_block; y++) {
        for(int x = half_block; x < width - half_block; x++) {
            // 计算block×block邻域像素和
            int sum = 0;
            for(int dy = -half_block; dy <= half_block; dy++) {
                for(int dx = -half_block; dx <= half_block; dx++) {
                    sum += img_data[(x + dx) + (y + dy) * width];
                }
            }
            // 计算局部阈值并二值化
            uint8_t thres = sum / (block * block) - clip_value;
            output_data[x + y * width] = img_data[x + y * width] > thres ? 255 : 0;
        }
    }
}

