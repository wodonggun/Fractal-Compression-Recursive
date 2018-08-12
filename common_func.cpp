#include "common_func.h"

using namespace std;
using namespace cv;

void ImageShow(const char* winname, int** image, int width, int height) {
	Mat img(height, width, CV_8UC1);

	for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
			img.at<unsigned char>(i, j) = (unsigned char)image[i][j];
	imshow(winname, img);
	waitKey(0);
}

void ERFree2(EncodingResult_List** image, int width, int height) {		//인코딩결과 구조체 동적할당받은거 메모리 해제 함수
	for (int i = 0; i<height; i++)									    //메모리 해제는 할당의 역순으로
		free(image[i]);
	free(image);
}

int** IntAlloc2(int width, int height) {
	int** tmp;
	tmp = (int**)calloc(height, sizeof(int*));
	for (int i = 0; i<height; i++)
		tmp[i] = (int*)calloc(width, sizeof(int));
	return(tmp);
}

void IntFree2(int** image, int width, int height) {
	for (int i = 0; i<height; i++)
		free(image[i]);
	free(image);
}

int** ReadImage(const char* name, int* width, int* height) {
	Mat img = imread(name, IMREAD_GRAYSCALE);
	int** image = (int**)IntAlloc2(img.cols, img.rows);
	*width = img.cols;
	*height = img.rows;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			image[i][j] = img.at<unsigned char>(i, j);
	return(image);
}

void WriteImage(const char* name, int** image, int width, int height) {
	Mat img(height, width, CV_8UC1);
	for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
			img.at<unsigned char>(i, j) = (unsigned char)image[i][j];
	imwrite(name, img);
}

void Contraction(int** image, int** image_out, int width, int height) {// 1/2 이미지 축소
	for (int y = 0; y < height - 1; y += 2)
		for (int x = 0; x < width - 1; x += 2)
			image_out[y / 2][x / 2] = (image[y][x] + image[y][x + 1] + image[y + 1][x] + image[y + 1][x + 1]) / 4;
}

void IsoM_0(int** img_in, int width, int height, int** img_out) {// 이미지 복사(a->b copy)
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			img_out[y][x] = img_in[y][x];
}

void IsoM_1(int** img_in, int width, int height, int** img_out) {// 좌우대칭(y축 대칭)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[i][(width - 1) - j];
}

void IsoM_2(int** img_in, int width, int height, int** img_out) {// 상하대칭(x축 대칭)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[(height - 1) - i][j];
}

void IsoM_3(int** img_in, int width, int height, int** img_out) {// y=-x 대칭(역슬래쉬 대칭)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[j][i];
}

void IsoM_4(int** img_in, int width, int height, int** img_out) {// y=x대칭(슬래쉬 대칭)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[(height - 1) - j][(width - 1) - i];
}

void IsoM_5(int** img_in, int width, int height, int** img_out) {//90도 회전 
	if (width != height) {
		for (int i = 0; i < width; i++)
			for (int j = 0; j < height; j++)
				img_out[i][j] = img_in[(height - 1) - j][i];
	}
	else
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
				img_out[i][j] = img_in[(width - 1) - j][i];
}

void IsoM_6(int** img_in, int width, int height, int** img_out) {//180도 회전
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[(height - 1) - i][(width - 1) - j];
}

void IsoM_7(int** img_in, int width, int height, int** img_out) {//-90도 회전
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[j][(height - 1) - i];
}

void Isometry(int num, int** img_in, int width, int height, int** img_out) {//Isom 선택함수
	switch (num) {
	case 0:
		IsoM_0(img_in, width, height, img_out); break;
	case 1:
		IsoM_1(img_in, width, height, img_out); break;
	case 2:
		IsoM_2(img_in, width, height, img_out); break;
	case 3:
		IsoM_3(img_in, width, height, img_out); break;
	case 4:
		IsoM_4(img_in, width, height, img_out); break;
	case 5:
		IsoM_5(img_in, width, height, img_out); break;
	case 6:
		IsoM_6(img_in, width, height, img_out); break;
	case 7:
		IsoM_7(img_in, width, height, img_out); break;
	default:
		printf("Isom default", num); break;
	}
}

int ComputeAVG(int** image, int width, int height) {//이미지 평균 계산
	int avg = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			avg += image[j][i];
	return avg = avg / (width * height) + 0.5; //0.5를 반올림해줘야 오차 최소가 됨.
}

void ReadBlock(int** image, int x, int y, int dx, int dy, int** block) {//image(a)의 x,y의 좌표에 블록크기만큼 block(b)에 씌움. 
	for (int i = 0; i < dy; i++)
		for (int j = 0; j < dx; j++)
			block[i][j] = image[y + i][x + j];
}

void WriteBlock(int** image, int x, int y, int dx, int dy, int** block) {//image(a)의 x,y의 좌표에다가 block(b)를 블록크기만큼 씌움. 
	for (int i = 0; i < dy; i++)
		for (int j = 0; j < dx; j++)
			image[y + i][x + j] = block[i][j];
}

int ComputeError(int** block, int size_block, int** image, int width, int height, int x_temp, int y_temp) {//error값 검출
	int temp = 0;
	for (int y = 0; y < size_block; y++)
		for (int x = 0; x < size_block; x++)
			temp += abs(image[y][x] - block[y + y_temp][x + x_temp]);
	return temp;
}

void Find_AC(int** image, int size_x, int size_y, int block_avg) {//AC 평균 제거
	for (int y = 0; y < size_y; y++)
		for (int x = 0; x < size_x; x++)
			image[y][x] = image[y][x] - block_avg;
}

void Copy_img(int** image, int width, int height, int** img_out) {//이미지 복사
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			img_out[y][x] = image[y][x];
}

void AC_control(int** image, int width, int height, double alpha, int** temp) {//이미지에 alpha곱하고
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			temp[y][x] = (int)(alpha * image[y][x] + 0.5);  //+0.5를 꼭 해줘야 손실 줄임.
}

EncodingResult_List** ERAlloc2(int width, int height) {//인코딩 결과 구조체 동적 할당함수
	EncodingResult_List** tmp;
	tmp = (EncodingResult_List**)calloc(height, sizeof(EncodingResult_List*)); // calloc은 기본적으로 값을 0으로 초기화시켜주며, sizeof(int*)의 크기의 height갯수만큼 할당(1차원)
	for (int i = 0; i<height; i++)
		tmp[i] = (EncodingResult_List*)calloc(width, sizeof(EncodingResult_List)); //calloc은 기본적으로 값을 0으로 초기화시켜주며, sizeof(int*)의 크기의 width갯수만큼 할당(2차원)
	return(tmp);

}
