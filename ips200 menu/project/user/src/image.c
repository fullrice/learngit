#include "image.h"
#include <math.h> 

// 定义并初始化全局的结构体变量 my_image
Image my_image = {0};

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
    
    // 初始化最长白列数据
    my_image.Longest_White_Column_Left[0] = 0;
    my_image.Longest_White_Column_Left[1] = 0;
    my_image.Longest_White_Column_Right[0] = 0;
    my_image.Longest_White_Column_Right[1] = 0;
    
    // 数据清零
    for (i = 0; i <= MT9V03X_H - 1; i++)
    {
        my_image.Right_Lost_Flag[i] = 0;
        my_image.Left_Lost_Flag[i] = 0;
        my_image.Left_Line[i] = 0;
        my_image.Right_Line[i] = MT9V03X_W - 1;
    }
    for (i = 0; i <= MT9V03X_W - 1; i++)
    {
        my_image.White_Column[i] = 0;
    }
    
    // 统计每列白点数量
    for (j = start_column; j <= end_column; j++)
    {
        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (my_image.image_two_value[i][j] == IMG_BLACK)
                break;
            else
                my_image.White_Column[j]++;
        }
    }
    
    // 从左到右找左边最长白列
    my_image.Longest_White_Column_Left[0] = 0;
    for (i = start_column; i <= end_column; i++)
    {
        if (my_image.Longest_White_Column_Left[0] < my_image.White_Column[i])
        {
            my_image.Longest_White_Column_Left[0] = my_image.White_Column[i];
            my_image.Longest_White_Column_Left[1] = i;
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