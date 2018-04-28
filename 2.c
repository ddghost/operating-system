#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#define PERM S_IRUSR|S_IWUSR
#define MAX_LINE 80
#define MAXSTOREDCOMMANDS 3
typedef struct{
	char commands[MAXSTOREDCOMMANDS + 1][MAX_LINE];
	bool ifValid[MAXSTOREDCOMMANDS + 1];
	int head ;
	int tail ;
	int commandsError[MAXSTOREDCOMMANDS + 1];
}storedCommand;

int *ifError;
storedCommand commands ; 
bool ifExecvp = true;
bool ifCatchControlC = false;

void handle_SIGINT(){
	ifCatchControlC = true;
	showCommands();
}


void initialCommands(){
	commands.head = 0;
	commands.tail = -1;
	
}

void putCommands(char *thisCommand){
	commands.tail ++;	
	commands.tail %= (MAXSTOREDCOMMANDS + 1);
	for(int loop = 0 ; loop < MAX_LINE ; loop++){
		if(thisCommand[loop] == '\n'){
			thisCommand[loop] = '\0';
			break;
		}
	}
	strcpy(commands.commands[commands.tail] , thisCommand);
	if( (commands.tail + 1) % (MAXSTOREDCOMMANDS + 1) == commands.head ){
		commands.head ++;
		commands.head %= (MAXSTOREDCOMMANDS + 1);
	}
	
}

void showCommands(){
	int commandsSize = (commands.tail + MAXSTOREDCOMMANDS - commands.head + 2) 
									% (MAXSTOREDCOMMANDS + 1);
	printf("\nHistory Commands:\n" );
	if(commandsSize == 0){
		printf("none\n");
		return;	
	}
	for(int loop = commandsSize - 1 ; loop >= 0 ; loop--){
		printf("%d -> %s\n" , commandsSize - loop - 1 
					, commands.commands[ (loop + commands.head) % (MAXSTOREDCOMMANDS + 1)]);
	}
}

int getCommandIndex(char firstLetter){
	int commandsSize = (commands.tail + MAXSTOREDCOMMANDS - commands.head + 2) 
									% (MAXSTOREDCOMMANDS + 1);
	if(firstLetter == 0){
		return commands.commands[(commands.tail + MAXSTOREDCOMMANDS - 1) % (MAXSTOREDCOMMANDS + 1)];
	}

	for(int loop = commandsSize - 1 ; loop >= 0 ; loop--){
		if(commands.commands[ (loop + commands.head) % (MAXSTOREDCOMMANDS + 1)][0] == firstLetter){
			return (loop + commands.head) % (MAXSTOREDCOMMANDS + 1);
		}	
	}
	return -1;
}

int main(void)
{
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	sigaction(SIGINT , &handler , NULL);
	initialCommands();
	key_t shmid;
	shmid = shmget(IPC_PRIVATE, sizeof(int), PERM);
	ifError = shmat(shmid, 0, 0);
	char inputBuffer[MAX_LINE]; /* 这个缓存用来存放输入的命令*/
	int background;
	
	/* ==1时,表示在后台运行命令,即在命令后加上'&' */
	char *args[MAX_LINE/2+1];/* 命令最多40个参数 */
	while (1){
		/* 程序在setup中正常结束*/
		background = 0;
		printf("COMMAND->"); //输出提示符,没有换行,仅将字符串送入输出缓存
		fflush(stdout);
		//若要输出输出缓存内容用fflush(stdout);头文件stdio.h

		setup(inputBuffer,args,&background);
		/* 获取下一个输入的命令 */
		pid_t pid = fork();
		if(pid == 0){
			if(ifExecvp)
				*ifError = execvp(args[0] , args);
			if(*ifError == -1){
				*ifError = errno;
			}
			exit(1);		
		}
		else{
			if(background==0) wait( NULL );
			commands.commandsError[commands.tail] = *ifError;
			
		}
		
	}
}
/* 这一步要做:
(1) 用fork()产生一个子进程
(2) 子进程将调用execvp()执行命令,即 execvp(args[0],args);
(3) 如果 background == 0, 父进程将等待子进程结束, 即if(background==0) wait(0);
否则,将回到函数setup()中等待新命令输入.
*/
void setup(char inputBuffer[], char *args[],int *background)
{
	
	int length, /* 命令的字符数目 */
	ct; /* 下一个参数存入args[]的位置 */
	ct = 0;
	/* 读入命令行字符,存入inputBuffer */
	ifCatchControlC = false;
	length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

	if(ifCatchControlC){
		args[0] = NULL;
		return;
	}
	solve(ct , length , inputBuffer , args , background);
	
	
}

void solve(int ct , int length, char inputBuffer[], char *args[],int *background ){
	int i , start;
	start = -1;
	ifExecvp = true;
	if (length == 0) exit(0);
	/* 输入ctrl+d,结束shell程序 */
	if (length < 0){
		perror("error reading the command");
		exit(-1);
		/* 出错时用错误码-1结束shell */
	}/* 检查inputBuffer中的每一个字符 */
	if(inputBuffer[0] == '\n'){
		args[0] = NULL;
		return ;
	}
	if(  inputBuffer[0] == 'r' && inputBuffer[1] == ' ' ){
		bool isHistoryCommand = true;
		if(isHistoryCommand){
			char firstLetter , *tempBuffer;
			if(length > 3){
				firstLetter = inputBuffer[2];
			}
			else{
				firstLetter = '\0';
			}
			int index = getCommandIndex(firstLetter) ;
			if(index == -1){
				printf("\nno such command\n");
			}
			else if(commands.commandsError[index] != 0){
				printf("%s\n" , strerror(commands.commandsError[index]) );
			
				ifExecvp = false;
			}
			else{
				char *tempBuffer = commands.commands[index];
				solve(0 , strlen( tempBuffer ) + 1 , tempBuffer , args , 0);		
			}
			return ;
		}
	}
	putCommands(inputBuffer);
	for (i=0 ; i < length ; i++) {
		switch (inputBuffer[i]){
		case ' ':
		case '\t' :
		/* 字符为分割参数的空格或制表符(tab)'\t'*/
			if(start != -1){
				args[ct] = &inputBuffer[start];
				ct++;
			}
			inputBuffer[i] = '\0'; /* 设置 C string 的结束符 */
			start = -1;
		break;
		case '\n':
		case '\0':
			
			/* 命令行结束 */
			if (start != -1){
				args[ct] = &inputBuffer[start];
				ct++;
			}
			inputBuffer[i] = '\0';
			args[ct] = NULL; /* 命令及参数结束 */
			break;
		default:
			/* 其他字符 */
			
			if (start == -1)
				start = i;
			if (inputBuffer[i] == '&'){
				*background = 1;
				/*置命令在后台运行*/
				inputBuffer[i] = '\0';
			}
		}

	}
	args[ct] = NULL; /* 命令字符数 > 80 */
}
