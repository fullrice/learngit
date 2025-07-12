#include "image_test.h"
#include "image.h"
/*-------------------------------------------------------------------------------------------------------------------
  @brief      分布大津法（四分法）
  @param      image    图像数组
              width    图像宽度
              height   图像高度
  @return     综合阈值（int）
  @note       调用原有的 My_Adapt_Threshold()，不修改原函数
-------------------------------------------------------------------------------------------------------------------*/
int Distributed_Otsu_Threshold(uint8 *image, uint16 width, uint16 height) 
{
    // 定义四个子区域的起始坐标和宽高
    uint16 sub_width = width / 2;
    uint16 sub_height = height / 2;

    // 四个子区域的指针（指向原图像数据）
    uint8 *top_left = image;
    uint8 *top_right = image + sub_width;
    uint8 *bottom_left = image + width * sub_height;
    uint8 *bottom_right = bottom_left + sub_width;

    // 计算四个子区域的阈值
    int th1 = My_Adapt_Threshold(top_left, sub_width, sub_height);       // 左上
    int th2 = My_Adapt_Threshold(top_right, sub_width, sub_height);      // 右上
    int th3 = My_Adapt_Threshold(bottom_left, sub_width, sub_height);    // 左下
    int th4 = My_Adapt_Threshold(bottom_right, sub_width, sub_height);   // 右下

    // 综合阈值（简单平均）
    int final_threshold = (th1 + th2 + th3 + th4) / 4;

    return final_threshold;
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     sobel二值化
  @param     imagein       原图数组
             imageout     二值化后的数组
  @return    null
  Sample     lq_sobelAutoThreshold(mt9v03x_image, my_image.image_two_value)
  @note      会比大津慢一些，效果比大津法好不少
-------------------------------------------------------------------------------------------------------------------*/
void lq_sobelAutoThreshold(unsigned char imageIn[MT9V03X_H][MT9V03X_W], 
                          unsigned char imageOut[MT9V03X_H][MT9V03X_W])
{
    /**卷积核大小**/
    short KERNEL_SIZE = 3;
    short xStart = KERNEL_SIZE / 2;
    short xEnd = MT9V03X_W - KERNEL_SIZE / 2;
    short yStart = KERNEL_SIZE / 2;
    short yEnd = MT9V03X_H - KERNEL_SIZE / 2;
    short i, j, k;
    short temp[4];
    
    for (i = yStart; i < yEnd; i++)
    {
        for (j = xStart; j < xEnd; j++)
        {
            /* 计算不同方向梯度幅值 */
            temp[0] = -(short)imageIn[i - 1][j - 1] + (short)imageIn[i - 1][j + 1]
                    - (short)imageIn[i][j - 1] + (short)imageIn[i][j + 1]
                    - (short)imageIn[i + 1][j - 1] + (short)imageIn[i + 1][j + 1];
                    
            temp[2] = -(short)imageIn[i - 1][j] + (short)imageIn[i][j - 1]
                    - (short)imageIn[i][j + 1] + (short)imageIn[i + 1][j]
                    - (short)imageIn[i - 1][j + 1] + (short)imageIn[i + 1][j - 1];
                    
            temp[3] = -(short)imageIn[i - 1][j] + (short)imageIn[i][j + 1]
                    - (short)imageIn[i][j - 1] + (short)imageIn[i + 1][j]
                    - (short)imageIn[i - 1][j - 1] + (short)imageIn[i + 1][j + 1];
                    
            temp[0] = abs(temp[0]);
            temp[1] = abs(temp[1]);
            temp[2] = abs(temp[2]);
            temp[3] = abs(temp[3]);
            
            /* 找出梯度幅值最大值 */
            for (k = 1; k < 4; k++)
            {
                if (temp[0] < temp[k])
                {
                    temp[0] = temp[k];
                }
            }
            
            /* 使用像素点邻域内像素点之和的一定比例作为阈值 */
            temp[3] = (short)imageIn[i - 1][j - 1] + (short)imageIn[i - 1][j] + (short)imageIn[i - 1][j + 1]
                    + (short)imageIn[i][j - 1] + (short)imageIn[i][j] + (short)imageIn[i][j + 1]
                    + (short)imageIn[i + 1][j - 1] + (short)imageIn[i + 1][j] + (short)imageIn[i + 1][j + 1];
 
            if (temp[0] > temp[3] / 12.0f)
            {
                imageOut[i][j] = 0;
            }
            else
            {
                imageOut[i][j] = 0xff;
            }
        }
    }
}


/* 前进方向定义：
 *   0
 * 3   1
 *   2
 */
//迷宫法
//左手迷宫
// 定义方向数组（直接显式写出）
const int dir_front[4][2] = {
    {0, -1},  // 上
    {1, 0},   // 右
    {0, 1},   // 下
    {-1, 0}   // 左
};

const int dir_frontleft[4][2] = {
    {-1, -1},  // 左上
    {1, -1},   // 右上
    {1, 1},    // 右下
    {-1, 1}    // 左下
};


/**
* 左手迷宫（从下到上）
 * @param block_size   领域的大小
 * @param clip_value   阈值的偏移值
 * @param x            起始点
 * @param y						 起始点 
 * @param pts          存储路径点的x和y
 * @param num          本次最多走的路径点个数
 */
void maze_left(int block_size, int clip_value,int x, int y, int pts[][2])   //, int* num
{
    int half = block_size / 2;
    int dir = 3, turn = 0;
	 my_image.stepl=0;
	// ips200_show_int(0, 0,dir, 3);
   //step < *num && 
    while (my_image.stepl<=100 && half < x && x < MT9V03X_W - half - 1 && 
           half < y && y < MT9V03X_H - half - 1 && 
           turn < 4) 
    {
        // 计算局部阈值
        int local_thres = 0;
        for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
                local_thres += mt9v03x_image[y + dy][x + dx]; // 直接访问二维数组
            }
        }
        local_thres /= block_size * block_size;
        local_thres -= clip_value;
  //      ips200_show_int(0, 0,dir, 3);
        // 像素值访问
        int current_value = mt9v03x_image[y][x];
        int front_value = mt9v03x_image[y + dir_front[dir][1]][x + dir_front[dir][0]];
        int frontleft_value = mt9v03x_image[y + dir_frontleft[dir][1]][x + dir_frontleft[dir][0]];

        // 路径决策逻辑
        if (front_value < local_thres) {
            dir = (dir + 1) % 4;  // 右转
            turn++;
        } 
				//左前是墙
        else if (frontleft_value < local_thres) {
            x += dir_front[dir][0];  // 直行
		//			  ips200_show_int(50, 50,x, 5);
					 // delay_ms(20); 
            y += dir_front[dir][1];
			//		  dir = (dir + 1) % 4;
            pts[my_image.stepl][0] = x;
            pts[my_image.stepl][1] = y;
            my_image.stepl++;
            turn = 0;
        } 
        else {
            x += dir_frontleft[dir][0];  // 左转
			//		 ips200_show_int(50, 50,x, 5);
            y += dir_frontleft[dir][1];
            dir = (dir + 3) % 4;
            pts[my_image.stepl][0] = x;
            pts[my_image.stepl][1] = y;
            my_image.stepl++;
            turn = 0;
        }
    }
		
  //  *num = step;
}

const int dir_frontright[4][2] = {
    {1, -1},  // 右上（原左手法中的左上对称）
    {1, 1},    // 右下
    {-1, 1},   // 左下
    {-1, -1}   // 左上
};

/**
 * 右手迷宫法（从下到上）
 * @param block_size   邻域大小
 * @param clip_value   阈值偏移值
 * @param x            起始点x
 * @param y            起始点y
 * @param pts          存储路径点的数组
 */
void maze_right(int block_size, int clip_value, int x, int y, int pts[][2]) {
    int half = block_size / 2;
    int dir = 1, turn = 0;
   my_image.stepr=0;
    while (my_image.stepr<=100 && half < x && x < MT9V03X_W - half - 1 && 
           half < y && y < MT9V03X_H - half - 1 && 
           turn < 4) {
        // 计算局部阈值
        int local_thres = 0;
        for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
                local_thres += mt9v03x_image[y + dy][x + dx];
            }
        }
        local_thres /= block_size * block_size;
			//	ips200_show_int(40, 130, local_thres, 5);
        local_thres -= clip_value;
     
        // 像素值访问
        int current_value = mt9v03x_image[y][x];
        int front_value = mt9v03x_image[y + dir_front[dir][1]][x + dir_front[dir][0]];
        int frontright_value = mt9v03x_image[y + dir_frontright[dir][1]][x + dir_frontright[dir][0]];

        // 右手法则路径决策（与左手法对称）
				//前面是墙就选转（右手定则）
        if (front_value < local_thres) {
            dir = (dir + 3) % 4;  // 左转（原右转取反）
            turn++;
        } 
        else if (frontright_value < local_thres) {
            x += dir_front[dir][0];  // 直行
            y += dir_front[dir][1];
	//				  dir = (dir + 3) % 4;
            pts[my_image.stepr][0] = x;
            pts[my_image.stepr][1] = y;
            my_image.stepr++;
            turn = 0;
        } 
        else {
					//先向前一步
            x += dir_frontright[dir][0];  // 右转（原左转取反）
            y += dir_frontright[dir][1];
    
				  	dir = (dir + 1) % 4;
            pts[my_image.stepr][0] = x;
            pts[my_image.stepr][1] = y;
            my_image.stepr++;
            turn = 0;
        }
    }
}

/**
 * @brief 标记路径点为黑色（0）
 * @param pts 路径点数组，格式为 pts[][2] (x,y)
 * @param num_points 点的数量
 */
void mark_path(int pts[][2], int num_points) {
	
    for (int i = 0; i < num_points; i++) {
        int x = pts[i][0];
        int y = pts[i][1];
        if (x >= 0 && x < MT9V03X_W && y >= 0 && y < MT9V03X_H) {
            my_image.maze_display[y][x] = 0;  // 直接赋值，标记黑点
        }
    }
}



void find_xy(int offset)
{
   int startx_l=MT9V03X_W/2;
	 int startx_r=MT9V03X_W/2;
	 int starty=MT9V03X_H-20;
	/*
	while(mt9v03x_image[starty][startx_l]-mt9v03x_image[starty][startx_l-1]<50)
	{
	   startx_l--;
	}
	my_image.start_x_l=startx_l;
	while(mt9v03x_image[starty][startx_r]-mt9v03x_image[starty][startx_r+1]<50)
	{
	   startx_r++;
	}
	my_image.start_x_r=startx_r;
	*/
	int thod=100,sum=0;
	for(int i=2;i<=MT9V03X_W-1;i++)
	{
	  sum+=mt9v03x_image[starty][i];
	}
	 thod=sum/160+offset;
	for(int i=2;i<=MT9V03X_W-1;i++)
	{
	  if(mt9v03x_image[starty][i]<=thod)
		{ 
       my_image.image_two_value[starty][i]=0;
		
		}
		else
		{
		   my_image.image_two_value[starty][i]=255;
		}
	}
	while(my_image.image_two_value[starty][startx_l]==255 && startx_l>0)
	{
	   startx_l--;
	}
	my_image.start_x_l=startx_l;
	while(my_image.image_two_value[starty][startx_r]==255 && startx_r<MT9V03X_W-5)
	{
	   startx_r++;
	}
//	while(mt9v03x_image[starty][startx_l]>100 && startx_l>0)
//	{
//	   startx_l--;
//	}
//	my_image.start_x_l=startx_l;
//	while(mt9v03x_image[starty][startx_r]>100 && startx_r<MT9V03X_W-5)
//	{
//	   startx_r++;
//	}
	my_image.start_x_r=startx_r;
}


void find_xy_enhanced() {
    const int starty = MT9V03X_H - 20;
    const int min_diff = 50;
 //   const int max_search = 100; // 防止无限循环
    
    // 左边界检测（寻找暗->亮边缘）
    int startx_l = MT9V03X_W / 2;
//    int search_count = 0;
    while (startx_l > 0 && 
           abs(mt9v03x_image[starty][startx_l] - mt9v03x_image[starty][startx_l - 1]) < min_diff) {
        startx_l--;
    }
    my_image.start_x_l = startx_l;

    // 右边界检测（寻找亮->暗边缘）
    int startx_r = MT9V03X_W / 2;
    while (startx_r < MT9V03X_W - 1 && 
           abs(mt9v03x_image[starty][startx_r] - mt9v03x_image[starty][startx_r + 1]) < min_diff) {
        startx_r++;
    }
    my_image.start_x_r = startx_r;

  
}