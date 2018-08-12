#include "common_func.h"

bool WriteParameter_internal(FILE* a, EncodingResult_List** A, int x, int y) {	//��� �Լ��� ó���ϱ� ���� ���������Ϳ� ����ü�� ����Լ��� �Ѱ��ݴϴ�. 
	if (a == NULL) {															//�Ű������� ���� ���������Ͱ� NULL�̸� ���� ǥ��.
		printf("\n ���� File������  *a ����!\n");
		return false;  }

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			fprintf(a, "%d %d %d %d %f %d %d\n", A[i][j].x, A[i][j].y, A[i][j].geo, A[i][j].avg, A[i][j].alpha, A[i][j].error, A[i][j].flag);
			if (A[i][j].sub != NULL) {
				WriteParameter_internal(a, A[i][j].sub, x, y);
			}						//��� �Լ��� ó���ϱ� ���� ���������Ϳ� ����ü�� ����Լ��� �Ѱ��ݴϴ�.
		}
	}
	return true;  
}

bool WriteParameter(const char* name, EncodingResult_List** A, int x, int y) {						//txt�� ����ü ������ ���Ϸ� ����

	FILE* fp = fopen(name, "w");

	if (fp == NULL){																				//���������Ͱ� NULL�϶� ���� ǥ��
	printf("\n Failure in fopen!!");	
	return false; }

	for (int ty = 0; ty < y; ty++)
		for (int tx = 0; tx < x; tx++){
		fprintf(fp, "%d %d %d %d %f %d %d\n", A[ty][tx].x, A[ty][tx].y, A[ty][tx].geo, A[ty][tx].avg, A[ty][tx].alpha, A[ty][tx].error, A[ty][tx].flag);
			if (A[ty][tx].sub != NULL){
				WriteParameter_internal(fp, A[ty][tx].sub, x, y);									//��� �Լ��� ó���ϱ� ���� ���������Ϳ� ����ü�� ����Լ��� �Ѱ��ݴϴ�.
			}
		}
	fclose(fp);
	return true;
}

EncodingResult_List Encoding_Recrusive_Call(int** block, int bx, int by, int** image, int width, int height, int count,int x,int y) { //����ü ��� ���� �����ϴ� ���ڵ� �Լ�
	
	EncodingResult_List struct_Tmp;														//������ ����ü ����
	int error_min = INT_MAX;															//�ּҰ��� �񱳸� ���� int�ִ밪���� �ʱ�ȭ
	int** temp = (int**)IntAlloc2(bx * 2, by * 2);										//���������� 2�踸ŭ �о�� �޸� �Ҵ�
	int** domain = (int**)IntAlloc2(bx, by);											//��� ó���ϰ� ������ �̹��� �޸� �Ҵ�
	int** tmp_test = (int**)IntAlloc2(bx, by);											//isomó���ϰ� ������ �̹��� �޸� �Ҵ�
	int block_avg = ComputeAVG(block, bx, by);											//�о�� block�� ��հ� ���
	struct_Tmp.avg = block_avg;															//����� ����� ���������� �Ź� error�� �ּҰ��� �ٲ𶧸��� ����ü�� ������ �ʿ���� �����̹Ƿ�
	int** block_AC = (int**)IntAlloc2(bx, by);											//�о�� ���� ������� �����ص� ������� �̹���
	int** domain_AC = (int**)IntAlloc2(bx, by);											//��� ������ ���� �̹����� ������ �޸� �Ҵ�
	Copy_img(block, bx, by, block_AC);													//block�� ���� ������ block_AC�� ����								
	Find_AC(block_AC, bx, by, block_avg);												//AC��� ����
	
	//x,y��ǥ�� ���� �������� 2�踸ŭ ���� -> 1/2��� -> ��հ�� -> isom�ݺ��� ���� ������ �ּ��϶� alpha�� �������� ����
	for (int i = 0; i < height - (by * 2)+1; i += by) {									//�������� �̹��� �ݺ� i(����)
		for (int j = 0; j < width - (bx * 2)+1; j += bx) {								//�������� �̹��� �ݺ� j(����)
			ReadBlock(image, j, i, bx * 2, by * 2, temp);								//���ũ���� 2�� �б�
			Contraction(temp, domain, bx * 2, by * 2);									//  1/2�� ���
			int domain_avg = ComputeAVG(domain, bx, by);								//domain�� ��հ� ����
			for (int n = 0; n < 8; n++) {												//isom�ݺ�
				Isometry(n, domain, bx, by, tmp_test);									
				Find_AC(tmp_test, bx, by, domain_avg);									//domain�� ��հ� ����
				for (double d = 0.3; d <= 1.0; d += 0.1) {								//alpha�� �ݺ� (double�� ����� 2.999999 ���ͽ��� - 1024bitȭ)
					AC_control(tmp_test, bx, by, d, domain_AC);							//error�� ���ϱ����� ���� ����
					int error = ComputeError(block_AC, bx, domain_AC, bx, by, 0, 0);	//error�� ����
					if (error < error_min) {											//error���� �ּҰ��϶� ����ü�� �� ����.
						error_min = error; struct_Tmp.x = j; struct_Tmp.y = i; struct_Tmp.alpha = d; struct_Tmp.geo = n; struct_Tmp.error = error/bx/by;   }	//error���� error_min���� ������ ���� ����
				}
			}
		}
	}
	//���� ���ΰ� ���ڵ� �Լ� ȣ�� ����(���)          �ڡڡ� error ���ΰ� ���� �ϴ� �� �ڡڡ�
	if ((struct_Tmp.error) > 6 && count <=2)														//�������ΰ� 6�̻��϶�, count�� ��ũ�⸦ 16���� 4������(16->8->4)�ϰ� ������ 3��°�϶� �������.
	{
		struct_Tmp.flag = 1;																		//�ɰ��� flag��Ʈ ó��
		struct_Tmp.sub = ERAlloc2(2, 2);															//�ɰ��⿡ ���� ������ 4�� ���� ������ ���� ����ü �迭[4] �Ҵ�
		int size_call = bx / 2;																		//��� ������ ����(16�� ���� ���ڵ����̸� 8�� ��ȯ)
		int** block_temp_call = (int**)IntAlloc2(size_call, size_call);
		int** block_AC_call = (int**)IntAlloc2(bx, by);												
		Copy_img(block, bx, by, block_AC_call);
		volatile int k = 0;																			//�ѹ� �ɰ������� 4���ؾ��ϹǷ�,��Ÿ�� ����(��������� �����Ͻÿ� k���� ��Ȯ�� ����; volatile���� ���)
		for (int i = 0; i < bx / size_call; i++) {													//   bx/size_call����   by/size_call ���� ����Լ��� ����� 2���� �ǹǷ� 4�� �ݺ�(4���� �ɰ��� Ƚ��)
			for (int j = 0; j < bx / size_call; j++) {
				ReadBlock(image, x+size_call*j, y+size_call*i, size_call, size_call, block_temp_call);
				struct_Tmp.sub[i][j] = Encoding_Recrusive_Call(block_temp_call, size_call, size_call, image, width, height, count + 1, x + size_call * j, y + size_call * i);	//��� ���ڵ� �Լ� ȣ��																						
			}//k++																				    //i,j ������ ���� �����ÿ� ����� �ݺ��ǰ��ִ��� volatile ������ ���� Ȯ��.
		}
		IntFree2(block_AC_call, bx, by);
		IntFree2(block_temp_call, size_call, size_call);
	}
	IntFree2(temp, bx * 2, by * 2);																	//�޸� ����
	IntFree2(domain, bx, by);
	IntFree2(block_AC, bx, by);
	IntFree2(domain_AC, bx, by);
	IntFree2(tmp_test, bx, by);
	return struct_Tmp;																				//����ü ���� ��ȯ
}

int main()
{
	int size = 16;																	//��� ������ ����
	int width, height;
	int** img_in = ReadImage("lena256x512.bmp", &width, &height);					//�̹��� �о��
	int** block_temp = (int**)IntAlloc2(size, size);								//�� �о�� temp �Ҵ�
	EncodingResult_List** en_result = ERAlloc2(width / size, height / size);		//���ڵ������ ������ ����ü ���� �Ҵ�
	for (int i = 0; i < height / size; i++) {
		for (int j = 0; j < width / size; j++) {
			ReadBlock(img_in, size * j, size * i, size, size, block_temp);												//x,y�� ��ǥ�� size��ŭ�� ����� �о�ͼ� temp�� ����.
			en_result[i][j] = Encoding_Recrusive_Call(block_temp, size, size, img_in, width, height, 1,j*size,i*size);  //���ڵ� �Լ� ����(������� ����ü�� ����)
		}
	}
	WriteParameter("encoding.txt", en_result, width / size, height / size);												//����ü�� ����� ������ txt�����ϱ� ���� �Լ�
	ERFree2(en_result, width / size, height / size);																	//�޸� ����
	IntFree2(block_temp, size, size);
	IntFree2(img_in, width, height);
	return 0;
}




