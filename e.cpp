#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main(){
	printf("a\n");
	pid_t pid1 = fork();

	if(pid1 == 0){
		printf("b\n");
		pid_t pid2 = fork();
		if(pid2 == 0){
			printf("c\n");		
		}
	}
}
