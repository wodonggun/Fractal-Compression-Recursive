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

void ERFree2(EncodingResult_List** image, int width, int height) {		//���ڵ���� ����ü �����Ҵ������ �޸� ���� �Լ�
	for (int i = 0; i<height; i++)									    //�޸� ������ �Ҵ��� ��������
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

void Contraction(int** image, int** image_out, int width, int height) {// 1/2 �̹��� ���
	for (int y = 0; y < height - 1; y += 2)
		for (int x = 0; x < width - 1; x += 2)
			image_out[y / 2][x / 2] = (image[y][x] + image[y][x + 1] + image[y + 1][x] + image[y + 1][x + 1]) / 4;
}

void IsoM_0(int** img_in, int width, int height, int** img_out) {// �̹��� ����(a->b copy)
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			img_out[y][x] = img_in[y][x];
}

void IsoM_1(int** img_in, int width, int height, int** img_out) {// �¿��Ī(y�� ��Ī)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[i][(width - 1) - j];
}

void IsoM_2(int** img_in, int width, int height, int** img_out) {// ���ϴ�Ī(x�� ��Ī)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[(height - 1) - i][j];
}

void IsoM_3(int** img_in, int width, int height, int** img_out) {// y=-x ��Ī(�������� ��Ī)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[j][i];
}

void IsoM_4(int** img_in, int width, int height, int** img_out) {// y=x��Ī(������ ��Ī)
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[(height - 1) - j][(width - 1) - i];
}

void IsoM_5(int** img_in, int width, int height, int** img_out) {//90�� ȸ�� 
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

void IsoM_6(int** img_in, int width, int height, int** img_out) {//180�� ȸ��
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[(height - 1) - i][(width - 1) - j];
}

void IsoM_7(int** img_in, int width, int height, int** img_out) {//-90�� ȸ��
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img_out[i][j] = img_in[j][(height - 1) - i];
}

void Isometry(int num, int** img_in, int width, int height, int** img_out) {//Isom �����Լ�
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

int ComputeAVG(int** image, int width, int height) {//�̹��� ��� ���
	int avg = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			avg += image[j][i];
	return avg = avg / (width * height) + 0.5; //0.5�� �ݿø������ ���� �ּҰ� ��.
}

void ReadBlock(int** image, int x, int y, int dx, int dy, int** block) {//image(a)�� x,y�� ��ǥ�� ���ũ�⸸ŭ block(b)�� ����. 
	for (int i = 0; i < dy; i++)
		for (int j = 0; j < dx; j++)
			block[i][j] = image[y + i][x + j];
}

void WriteBlock(int** image, int x, int y, int dx, int dy, int** block) {//image(a)�� x,y�� ��ǥ���ٰ� block(b)�� ���ũ�⸸ŭ ����. 
	for (int i = 0; i < dy; i++)
		for (int j = 0; j < dx; j++)
			image[y + i][x + j] = block[i][j];
}

int ComputeError(int** block, int size_block, int** image, int width, int height, int x_temp, int y_temp) {//error�� ����
	int temp = 0;
	for (int y = 0; y < size_block; y++)
		for (int x = 0; x < size_block; x++)
			temp += abs(image[y][x] - block[y + y_temp][x + x_temp]);
	return temp;
}

void Find_AC(int** image, int size_x, int size_y, int block_avg) {//AC ��� ����
	for (int y = 0; y < size_y; y++)
		for (int x = 0; x < size_x; x++)
			image[y][x] = image[y][x] - block_avg;
}

void Copy_img(int** image, int width, int height, int** img_out) {//�̹��� ����
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			img_out[y][x] = image[y][x];
}

void AC_control(int** image, int width, int height, double alpha, int** temp) {//�̹����� alpha���ϰ�
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			temp[y][x] = (int)(alpha * image[y][x] + 0.5);  //+0.5�� �� ����� �ս� ����.
}

EncodingResult_List** ERAlloc2(int width, int height) {//���ڵ� ��� ����ü ���� �Ҵ��Լ�
	EncodingResult_List** tmp;
	tmp = (EncodingResult_List**)calloc(height, sizeof(EncodingResult_List*)); // calloc�� �⺻������ ���� 0���� �ʱ�ȭ�����ָ�, sizeof(int*)�� ũ���� height������ŭ �Ҵ�(1����)
	for (int i = 0; i<height; i++)
		tmp[i] = (EncodingResult_List*)calloc(width, sizeof(EncodingResult_List)); //calloc�� �⺻������ ���� 0���� �ʱ�ȭ�����ָ�, sizeof(int*)�� ũ���� width������ŭ �Ҵ�(2����)
	return(tmp);

}
