#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define PERM S_IRUSR|S_IWUSR 
#define MAX_SEQUENCE 10
typedef struct{
	long fib_sequence[MAX_SEQUENCE];
	int sequence_size;
}shared_data;

int main(int argc, char **argv)
{
	key_t shmid;
	char *p_addr, *c_addr;
	shared_data *data_ptr;
	pid_t pid;
	int parameterNum ;

	if(argc != 2) {
		fprintf(stderr, "Usage:%s\n\a", argv[0]);
		exit(1);
	}
	parameterNum = atoi(argv[1]) ;
	if( parameterNum  > MAX_SEQUENCE){
		fprintf(stderr , "parameter size %d is bigger than %d\n" ,parameterNum , MAX_SEQUENCE);
		exit(1);
	}	

	if( (shmid = shmget(IPC_PRIVATE, sizeof(shared_data), PERM)) == -1 ) {
		fprintf(stderr, "Create Share Memory Error:%s\n\a",strerror(errno));
		exit(1);
	}
	data_ptr = shmat(shmid, 0, 0);
	memset(data_ptr, '\0', sizeof(shared_data) );
	(*data_ptr).sequence_size = parameterNum;
	pid = fork();

	if(pid > 0) {
		sleep(1);
		
		for(int loop = 0 ; loop < (*data_ptr).sequence_size ; loop++){
			printf("%ld\n" , (*data_ptr).fib_sequence[loop] );
		}
		exit(0);
	}
	else if (pid == 0){
		(*data_ptr).fib_sequence[0] = 1;
		(*data_ptr).fib_sequence[1] = 1;
		for(int loop = 2 ; loop < (*data_ptr).sequence_size ; loop++){
			(*data_ptr).fib_sequence[loop] = (*data_ptr).fib_sequence[loop-1] 
												+ (*data_ptr).fib_sequence[loop-2];
		}
		wait(NULL);
		exit(0);
	}
}
