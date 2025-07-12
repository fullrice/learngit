#ifndef _IMAGE_TEST_H
#define _IMAGE_TEST_H
/*头文件*/
#include "zf_common_headfile.h"
void maze_left(int block_size, int clip_value,int x, int y, int pts[][2]);
void maze_right(int block_size, int clip_value, int x, int y, int pts[][2]);
void mark_path(int pts[][2], int num_points);
void find_xy();
void find_xy_enhanced() ;

#endif