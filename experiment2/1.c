#include <stdio.h>
#include <pthread.h>

int arr [100];

void *generateFibonacci( void *parameter){
	int *num = (int *)parameter;
	if( *num > 100 || *num < 1) {
		printf("parameter %d out of bound [1,100]\n" , *num);
	}
	else{
		arr[0] = 0;
		arr[1] = 1;
		for(int loop = 2 ; loop < *num ; loop++){
			arr[loop] = arr[loop - 1] + arr[loop - 2];
		}
	}
	pthread_exit(0);
}


int main(){
	pthread_t th;
	int num;
	scanf("%d" , &num);
	pthread_create(&th , NULL , generateFibonacci , (void*)&num );

	pthread_join(th , NULL);
	for(int loop = 0 ; loop < num ; loop++){
		printf("%d\n" , arr[loop] );
	}
}
