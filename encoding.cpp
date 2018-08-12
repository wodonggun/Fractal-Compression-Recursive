#include "common_func.h"

bool WriteParameter_internal(FILE* a, EncodingResult_List** A, int x, int y) {	//재귀 함수로 처리하기 위해 파일포인터와 구조체를 재귀함수로 넘겨줍니다. 
	if (a == NULL) {															//매개변수로 받은 파일포인터가 NULL이면 에러 표시.
		printf("\n 내부 File포인터  *a 에러!\n");
		return false;  }

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			fprintf(a, "%d %d %d %d %f %d %d\n", A[i][j].x, A[i][j].y, A[i][j].geo, A[i][j].avg, A[i][j].alpha, A[i][j].error, A[i][j].flag);
			if (A[i][j].sub != NULL) {
				WriteParameter_internal(a, A[i][j].sub, x, y);
			}						//재귀 함수로 처리하기 위해 파일포인터와 구조체를 재귀함수로 넘겨줍니다.
		}
	}
	return true;  
}

bool WriteParameter(const char* name, EncodingResult_List** A, int x, int y) {						//txt로 구조체 정보를 파일로 저장

	FILE* fp = fopen(name, "w");

	if (fp == NULL){																				//파일포인터가 NULL일때 에러 표시
	printf("\n Failure in fopen!!");	
	return false; }

	for (int ty = 0; ty < y; ty++)
		for (int tx = 0; tx < x; tx++){
		fprintf(fp, "%d %d %d %d %f %d %d\n", A[ty][tx].x, A[ty][tx].y, A[ty][tx].geo, A[ty][tx].avg, A[ty][tx].alpha, A[ty][tx].error, A[ty][tx].flag);
			if (A[ty][tx].sub != NULL){
				WriteParameter_internal(fp, A[ty][tx].sub, x, y);									//재귀 함수로 처리하기 위해 파일포인터와 구조체를 재귀함수로 넘겨줍니다.
			}
		}
	fclose(fp);
	return true;
}

EncodingResult_List Encoding_Recrusive_Call(int** block, int bx, int by, int** image, int width, int height, int count,int x,int y) { //구조체 결과 값을 리턴하는 인코딩 함수
	
	EncodingResult_List struct_Tmp;														//리턴할 구조체 선언
	int error_min = INT_MAX;															//최소값은 비교를 위한 int최대값으로 초기화
	int** temp = (int**)IntAlloc2(bx * 2, by * 2);										//블럭사이즈의 2배만큼 읽어올 메모리 할당
	int** domain = (int**)IntAlloc2(bx, by);											//축소 처리하고 저장할 이미지 메모리 할당
	int** tmp_test = (int**)IntAlloc2(bx, by);											//isom처리하고 저장할 이미지 메모리 할당
	int block_avg = ComputeAVG(block, bx, by);											//읽어온 block의 평균값 계산
	struct_Tmp.avg = block_avg;															//블록의 평균은 일정하지만 매번 error의 최소값이 바뀔때마다 구조체에 넣으면 필요없는 연산이므로
	int** block_AC = (int**)IntAlloc2(bx, by);											//읽어온 블럭을 마음대로 수정해도 상관없는 이미지
	int** domain_AC = (int**)IntAlloc2(bx, by);											//모든 과정을 끝낸 이미지를 저장할 메모리 할당
	Copy_img(block, bx, by, block_AC);													//block에 대한 정보를 block_AC에 복사								
	Find_AC(block_AC, bx, by, block_avg);												//AC평균 제거
	
	//x,y좌표의 블럭을 사이즈의 2배만큼 읽음 -> 1/2축소 -> 평균계산 -> isom반복을 통해 에러가 최소일때 alpha와 정보들을 저장
	for (int i = 0; i < height - (by * 2)+1; i += by) {									//블럭단위로 이미지 반복 i(세로)
		for (int j = 0; j < width - (bx * 2)+1; j += bx) {								//블럭단위로 이미지 반복 j(가로)
			ReadBlock(image, j, i, bx * 2, by * 2, temp);								//블록크기의 2배 읽기
			Contraction(temp, domain, bx * 2, by * 2);									//  1/2배 축소
			int domain_avg = ComputeAVG(domain, bx, by);								//domain의 평균값 저장
			for (int n = 0; n < 8; n++) {												//isom반복
				Isometry(n, domain, bx, by, tmp_test);									
				Find_AC(tmp_test, bx, by, domain_avg);									//domain의 평균값 빼줌
				for (double d = 0.3; d <= 1.0; d += 0.1) {								//alpha값 반복 (double형 연산시 2.999999 부터시작 - 1024bit화)
					AC_control(tmp_test, bx, by, d, domain_AC);							//error값 구하기전에 알파 곱함
					int error = ComputeError(block_AC, bx, domain_AC, bx, by, 0, 0);	//error값 추출
					if (error < error_min) {											//error값이 최소값일때 구조체에 값 넣음.
						error_min = error; struct_Tmp.x = j; struct_Tmp.y = i; struct_Tmp.alpha = d; struct_Tmp.geo = n; struct_Tmp.error = error/bx/by;   }	//error값이 error_min보다 작을때 정보 저장
				}
			}
		}
	}
	//에러 문턱값 인코딩 함수 호출 조건(재귀)          ★★★ error 문턱값 조절 하는 곳 ★★★
	if ((struct_Tmp.error) > 6 && count <=2)														//에러문턱값 6이상일때, count는 블럭크기를 16에서 4까지만(16->8->4)하고 마지막 3번째일때 실행안함.
	{
		struct_Tmp.flag = 1;																		//쪼개기 flag비트 처리
		struct_Tmp.sub = ERAlloc2(2, 2);															//쪼개기에 들어가면 무조건 4개 분할 정보를 담을 구조체 배열[4] 할당
		int size_call = bx / 2;																		//블록 사이즈 결정(16에 대한 인코딩중이면 8로 변환)
		int** block_temp_call = (int**)IntAlloc2(size_call, size_call);
		int** block_AC_call = (int**)IntAlloc2(bx, by);												
		Copy_img(block, bx, by, block_AC_call);
		volatile int k = 0;																			//한번 쪼갤때마다 4번해야하므로,나타낼 변수(릴리즈모드로 컴파일시에 k값을 정확히 보고싶어서 volatile변수 사용)
		for (int i = 0; i < bx / size_call; i++) {													//   bx/size_call값과   by/size_call 값은 재귀함수로 실행시 2번씩 되므로 4번 반복(4개로 쪼개진 횟수)
			for (int j = 0; j < bx / size_call; j++) {
				ReadBlock(image, x+size_call*j, y+size_call*i, size_call, size_call, block_temp_call);
				struct_Tmp.sub[i][j] = Encoding_Recrusive_Call(block_temp_call, size_call, size_call, image, width, height, count + 1, x + size_call * j, y + size_call * i);	//재귀 인코딩 함수 호출																						
			}//k++																				    //i,j 릴리즈 모드로 디버깅시에 제대로 반복되고있는지 volatile 변수를 통해 확인.
		}
		IntFree2(block_AC_call, bx, by);
		IntFree2(block_temp_call, size_call, size_call);
	}
	IntFree2(temp, bx * 2, by * 2);																	//메모리 해제
	IntFree2(domain, bx, by);
	IntFree2(block_AC, bx, by);
	IntFree2(domain_AC, bx, by);
	IntFree2(tmp_test, bx, by);
	return struct_Tmp;																				//구조체 정보 반환
}

int main()
{
	int size = 16;																	//블록 사이즈 결정
	int width, height;
	int** img_in = ReadImage("lena256x512.bmp", &width, &height);					//이미지 읽어옴
	int** block_temp = (int**)IntAlloc2(size, size);								//블럭 읽어올 temp 할당
	EncodingResult_List** en_result = ERAlloc2(width / size, height / size);		//인코딩결과를 저장할 구조체 동적 할당
	for (int i = 0; i < height / size; i++) {
		for (int j = 0; j < width / size; j++) {
			ReadBlock(img_in, size * j, size * i, size, size, block_temp);												//x,y의 좌표의 size만큼의 블록을 읽어와서 temp에 저장.
			en_result[i][j] = Encoding_Recrusive_Call(block_temp, size, size, img_in, width, height, 1,j*size,i*size);  //인코딩 함수 실행(순서대로 구조체에 저장)
		}
	}
	WriteParameter("encoding.txt", en_result, width / size, height / size);												//구조체에 저장된 정보를 txt저장하기 위한 함수
	ERFree2(en_result, width / size, height / size);																	//메모리 해제
	IntFree2(block_temp, size, size);
	IntFree2(img_in, width, height);
	return 0;
}




