#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  

typedef struct EncodingResult_List {
	int geo; int avg; int error; double alpha; int x, y; int flag = 0;
	EncodingResult_List **sub = NULL;
}EncodingResult_List;


EncodingResult_List** ERAlloc2(int width, int height);
void ERFree2(EncodingResult_List** image, int width, int height);
void AC_control(int** image, int width, int height, double alpha, int** temp);
void Copy_img(int** image, int width, int height, int** img_out);
void Find_AC(int** image, int size_x, int size_y, int block_avg);
int ComputeError(int** block, int size_block, int** image, int width, int height, int x_temp, int y_temp);
void WriteBlock(int** image, int x, int y, int dx, int dy, int** block);
void ReadBlock(int** image, int x, int y, int dx, int dy, int** block);
int ComputeAVG(int** image, int width, int height);
void Isometry(int num, int** img_in, int width, int height, int** img_out);
void IsoM_7(int** img_in, int width, int height, int** img_out);
void IsoM_6(int** img_in, int width, int height, int** img_out);
void IsoM_5(int** img_in, int width, int height, int** img_out);
void IsoM_4(int** img_in, int width, int height, int** img_out);
void IsoM_3(int** img_in, int width, int height, int** img_out);
void IsoM_2(int** img_in, int width, int height, int** img_out);
void IsoM_1(int** img_in, int width, int height, int** img_out);
void IsoM_0(int** img_in, int width, int height, int** img_out);
void Contraction(int** image, int** image_out, int width, int height);

void ImageShow(const char* winname, int** image, int width, int height);
void WriteImage(const char* name, int** image, int width, int height);
int** ReadImage(const char* name, int* width, int* height);
void IntFree2(int** image, int width, int height);
int** IntAlloc2(int width, int height);
