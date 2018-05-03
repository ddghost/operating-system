#include <stdio.h>
#include <pthread.h>
#define M 3
#define K 2
#define N 3

int A[M][K] = {{1,4} ,{2,5} , {3,6} };
int B[K][N] = { {8,7,6} , {5,4,3} };
int C[M][N] ;
/*
1 4
2 5
3 6
*
8 7 6
5 4 3
=
28 23 18 
41 34 27
54 45 36
*/


typedef struct{
	int row ;
	int col;
	
}data;


void *runner( void *parameter){
	data *getData = (data*)parameter;
	int row = getData->row , col = getData->col;
	C[row][col] = 0;
	for(int loop = 0 ; loop < K ; loop++){
		C[row][col] += A[row][loop] * B[loop][col];
	}	

	pthread_exit(0);
}


int main(){
	pthread_t th[M * N];
	data dataArr[M * N];
	
	for(int loop0 = 0 ; loop0 < M ; loop0++){
		for(int loop1 = 0 ; loop1 < N ; loop1++){
			dataArr[loop1 + loop0*N].row = loop0;
			dataArr[loop1 + loop0*N].col = loop1;
			pthread_create(&th[loop1 + loop0*N] , NULL , runner , (void*)&dataArr[loop1 + loop0*N] );

		}
	}
	for(int loop0 = 0 ; loop0 < M ; loop0++){
		for(int loop1 = 0 ; loop1 < N ; loop1++){
			pthread_join(th[loop1 + loop0*N] , NULL  );

		}
	}
	for(int loop0 = 0 ; loop0 < M ; loop0++){
		for(int loop1 = 0 ; loop1 < N ; loop1++){
			printf("%d " , C[loop0][loop1]);
		}
		printf("\n");
	}

}
