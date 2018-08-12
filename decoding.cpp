#include "common_func.h"

int** img_in2;					//경계선을 그릴 image 변수

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

void Write_boundary(int** image, int block_size, int x, int y)	//쪼개기 들어간 이미지 경계 칠하기
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


void Decoding_Recursive_Call(EncodingResult_List** en_Result, int** image_dec,int** image_dec_tmp_recur, int width, int height, int size_x, int size_y,int count,int cur_x,int cur_y) //디코딩 함수
{
	if (count > 3)					//쪼개기 3번이상 하면 쪼개기 종료(재귀함수 종료 조건)
		return;
	int** block = IntAlloc2(size_x * 2, size_y * 2);
	int** block_contract_tmp = IntAlloc2(size_x, size_y);
	int** block_contract_tmp_aftercontrol = IntAlloc2(size_x, size_y);
	//x,y좌표에서 size의 2배크기만큼 읽어옴->이미지 축소->축소한이미지의 평균구하고 그 이미지에서 평균빼줌 -> 저장된 isom진행 -> 저장된 alpha 곱해주고 -> 평균 더해주고-> dec에 그려줌.
	for (int i = 0; i < height / size_y; i++) {
		for (int j = 0; j < width / size_x; j++) {
			if (en_Result[i][j].flag){	
				Decoding_Recursive_Call(en_Result[i][j].sub, image_dec,image_dec_tmp_recur, size_x, size_y, size_x/2, size_y/2, count + 1,j*size_x+cur_x,i*size_y+cur_y);	}
			else {
				ReadBlock(image_dec, en_Result[i][j].x, en_Result[i][j].y, size_x * 2, size_y * 2, block);													//x,y좌표의 블록크기의 2배만큼 읽어옴
				Contraction(block, block_contract_tmp, size_x * 2, size_y * 2);																				// 이미지1/2 축소
				int b_avg = ComputeAVG(block_contract_tmp, size_x, size_y);																					//평균값 계산
				Find_AC(block_contract_tmp, size_x, size_y, b_avg);																							//평균값 빼줌
				Isometry(en_Result[i][j].geo, block_contract_tmp, size_x, size_y, block_contract_tmp_aftercontrol);											//저장된 isom(geo)진행
				AC_control(block_contract_tmp_aftercontrol, size_x, size_y, en_Result[i][j].alpha, block_contract_tmp_aftercontrol);						//alpha 곱해줌.
				Find_AC(block_contract_tmp_aftercontrol, size_x, size_y, -en_Result[i][j].avg);																//저장된 평균값을 더해줌
				WriteBlock(image_dec_tmp_recur, (j * size_x)+cur_x, (i * size_y)+cur_y, size_x, size_y, block_contract_tmp_aftercontrol);					//img_dec_tmp의 x,y의 좌표에 블록크기만큼 처리된이미지를 씌움.
				if(count>1)	//쪼개기를 1회이상 햇을때 경계선 그리기.	
				Write_boundary(img_in2, size_x, (j * size_x)+cur_x, (i * size_y) + cur_y);																	//경계선 그리기 함수.
			}
		}
	}
	Copy_img(image_dec_tmp_recur, width, height, image_dec);				//image_tmp(디코딩처리한 이미지)를 image_dec(출력할 이미지)에 복사.
																
	IntFree2(block, size_x * 2, size_y * 2);								//메모리 해제
	IntFree2(block_contract_tmp, size_x, size_y);							
	IntFree2(block_contract_tmp_aftercontrol, size_x, size_y);				

}

void main()//디코딩 메인함수
{
	int size = 16;																				//블록 사이즈 결정
	int width, height;																			
	int** img_in = ReadImage("lena256x512.bmp", &width, &height);								//이미지 읽어옴
	int** image_dec = IntAlloc2(width, height);													//디코딩 변환할때마다 출력할 이미지 할당
	img_in2 = ReadImage("lena256x512.bmp", &width, &height);
	EncodingResult_List** en_result = ERAlloc2(width / size, height / size);					//디코딩할 정보를 가지고있는 구조체 동적 할당
	ReadParameter("encoding.txt", en_result, width / size, height / size);					//디코딩 할 정보를 가지고있는 txt파일 읽어옴

	for (int i = 0; i< height; i++)
		for (int j = 0; j < width; j++)
			image_dec[i][j] = 128;																//image_dec 디폴트 값 128로 초기화

	for (int i = 0; i < 5; i++){																//5번 실행을 통해 
		ImageShow("image_dec 출력", image_dec, width, height);									//0번돌린 이미지부터 4번돌린 이미지까지 순차적으로 디코딩(처음에는 회색의 도화지)
		int** image_dec_tmp = IntAlloc2(width, height);
		Decoding_Recursive_Call(en_result, image_dec, image_dec_tmp, width, height, size, size, 1,0,0);		//디코딩 실행(디코딩할 정보를 가지고있는 구조체, 디코딩결과값저장할이미지, 가로 , 새로 , 블록x사이즈, 블록y사이즈)
		IntFree2(image_dec_tmp, width, height);
	}
	ImageShow("image", img_in, width, height);													//이미지 원본 출력
	ImageShow("image_dec2", img_in2, width, height);											//블록 분할 출력												
	double p = computePSNR(img_in, image_dec, width, height);printf("PSNR값 출력 : %lf\n", p);	//PSNR값 출력
	IntFree2(image_dec, width, height);															//메모리해제
	IntFree2(img_in2, width, height);
	IntFree2(img_in, width, height);
	ERFree2(en_result, width / size, height / size);
}