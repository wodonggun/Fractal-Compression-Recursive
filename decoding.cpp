#include "common_func.h"

int** img_in2;					//��輱�� �׸� image ����

double computePSNR(int** A, int** B, int width, int height)
{
	double error = 0.0;
	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) {
		error += (double)(A[i][j] - B[i][j]) * (A[i][j] - B[i][j]);
	}
	error = error / (width*height);
	double psnr = 10.0 * log10(255.*255. / error);
	return(psnr);
}

void Write_boundary(int** image, int block_size, int x, int y)	//�ɰ��� �� �̹��� ��� ĥ�ϱ�
{
	for (int i = 0; i < block_size; i++)
	{
		image[i + y][0 + x] = 255;
		image[i + y][block_size - 1 + x] = 255;
	}
	for (int j = 0; j < block_size; j++)
	{
		image[0 + y][j + x] = 255;
		image[block_size - 1 + y][j + x] = 255;
	}
}

bool ReadParameter_recursive_call(FILE* fp, EncodingResult_List** A, int width, int height) {
	if (fp == NULL) {
		printf("\n File_Recursive_call NULL error "); return false;  }
	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++) {
			fscanf(fp, "%d%d%d%d%lf%d%d", &(A[j][i].x), &(A[j][i].y), &(A[j][i].geo), &(A[j][i].avg), &(A[j][i].alpha), &(A[j][i].error), &(A[j][i].flag));
			if (A[j][i].flag == 1) {
				A[j][i].sub = ERAlloc2(2, 2);
				ReadParameter_recursive_call(fp, A[j][i].sub, 2,2);
			}
		}
	return true;
}

bool ReadParameter(const char* name, EncodingResult_List** A, int width, int height) {
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		printf("\n Failure in fopen!!"); return false;  }

	ReadParameter_recursive_call(fp, A, width, height);
	fclose(fp);
	return true;
}


void Decoding_Recursive_Call(EncodingResult_List** en_Result, int** image_dec,int** image_dec_tmp_recur, int width, int height, int size_x, int size_y,int count,int cur_x,int cur_y) //���ڵ� �Լ�
{
	if (count > 3)					//�ɰ��� 3���̻� �ϸ� �ɰ��� ����(����Լ� ���� ����)
		return;
	int** block = IntAlloc2(size_x * 2, size_y * 2);
	int** block_contract_tmp = IntAlloc2(size_x, size_y);
	int** block_contract_tmp_aftercontrol = IntAlloc2(size_x, size_y);
	//x,y��ǥ���� size�� 2��ũ�⸸ŭ �о��->�̹��� ���->������̹����� ��ձ��ϰ� �� �̹������� ��ջ��� -> ����� isom���� -> ����� alpha �����ְ� -> ��� �����ְ�-> dec�� �׷���.
	for (int i = 0; i < height / size_y; i++) {
		for (int j = 0; j < width / size_x; j++) {
			if (en_Result[i][j].flag){	
				Decoding_Recursive_Call(en_Result[i][j].sub, image_dec,image_dec_tmp_recur, size_x, size_y, size_x/2, size_y/2, count + 1,j*size_x+cur_x,i*size_y+cur_y);	}
			else {
				ReadBlock(image_dec, en_Result[i][j].x, en_Result[i][j].y, size_x * 2, size_y * 2, block);													//x,y��ǥ�� ���ũ���� 2�踸ŭ �о��
				Contraction(block, block_contract_tmp, size_x * 2, size_y * 2);																				// �̹���1/2 ���
				int b_avg = ComputeAVG(block_contract_tmp, size_x, size_y);																					//��հ� ���
				Find_AC(block_contract_tmp, size_x, size_y, b_avg);																							//��հ� ����
				Isometry(en_Result[i][j].geo, block_contract_tmp, size_x, size_y, block_contract_tmp_aftercontrol);											//����� isom(geo)����
				AC_control(block_contract_tmp_aftercontrol, size_x, size_y, en_Result[i][j].alpha, block_contract_tmp_aftercontrol);						//alpha ������.
				Find_AC(block_contract_tmp_aftercontrol, size_x, size_y, -en_Result[i][j].avg);																//����� ��հ��� ������
				WriteBlock(image_dec_tmp_recur, (j * size_x)+cur_x, (i * size_y)+cur_y, size_x, size_y, block_contract_tmp_aftercontrol);					//img_dec_tmp�� x,y�� ��ǥ�� ���ũ�⸸ŭ ó�����̹����� ����.
				if(count>1)	//�ɰ��⸦ 1ȸ�̻� ������ ��輱 �׸���.	
				Write_boundary(img_in2, size_x, (j * size_x)+cur_x, (i * size_y) + cur_y);																	//��輱 �׸��� �Լ�.
			}
		}
	}
	Copy_img(image_dec_tmp_recur, width, height, image_dec);				//image_tmp(���ڵ�ó���� �̹���)�� image_dec(����� �̹���)�� ����.
																
	IntFree2(block, size_x * 2, size_y * 2);								//�޸� ����
	IntFree2(block_contract_tmp, size_x, size_y);							
	IntFree2(block_contract_tmp_aftercontrol, size_x, size_y);				

}

void main()//���ڵ� �����Լ�
{
	int size = 16;																				//��� ������ ����
	int width, height;																			
	int** img_in = ReadImage("lena256x512.bmp", &width, &height);								//�̹��� �о��
	int** image_dec = IntAlloc2(width, height);													//���ڵ� ��ȯ�Ҷ����� ����� �̹��� �Ҵ�
	img_in2 = ReadImage("lena256x512.bmp", &width, &height);
	EncodingResult_List** en_result = ERAlloc2(width / size, height / size);					//���ڵ��� ������ �������ִ� ����ü ���� �Ҵ�
	ReadParameter("encoding.txt", en_result, width / size, height / size);					//���ڵ� �� ������ �������ִ� txt���� �о��

	for (int i = 0; i< height; i++)
		for (int j = 0; j < width; j++)
			image_dec[i][j] = 128;																//image_dec ����Ʈ �� 128�� �ʱ�ȭ

	for (int i = 0; i < 5; i++){																//5�� ������ ���� 
		ImageShow("image_dec ���", image_dec, width, height);									//0������ �̹������� 4������ �̹������� ���������� ���ڵ�(ó������ ȸ���� ��ȭ��)
		int** image_dec_tmp = IntAlloc2(width, height);
		Decoding_Recursive_Call(en_result, image_dec, image_dec_tmp, width, height, size, size, 1,0,0);		//���ڵ� ����(���ڵ��� ������ �������ִ� ����ü, ���ڵ�������������̹���, ���� , ���� , ���x������, ���y������)
		IntFree2(image_dec_tmp, width, height);
	}
	ImageShow("image", img_in, width, height);													//�̹��� ���� ���
	ImageShow("image_dec2", img_in2, width, height);											//��� ���� ���												
	double p = computePSNR(img_in, image_dec, width, height);printf("PSNR�� ��� : %lf\n", p);	//PSNR�� ���
	IntFree2(image_dec, width, height);															//�޸�����
	IntFree2(img_in2, width, height);
	IntFree2(img_in, width, height);
	ERFree2(en_result, width / size, height / size);
}