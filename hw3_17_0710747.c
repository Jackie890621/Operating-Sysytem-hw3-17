#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_SEQUENCE 10
typedef struct{
	long fib[MAX_SEQUENCE];
	int arg;
} share_data;

int main(int argc, char *argv[])
{
	//判斷使用者是否只輸入一個數字
	if (argc == 1) {
                printf("You need to input a number\n");
		return 1;
        } else if (argc > 2) {
                printf("You only need to input one number\n");
		return 1;
        }

	share_data *data;
	int n = atoi(argv[1]);
	int status;
	int id;
	int key = 1;
	pid_t parent_pid, child_pid;

	if (n <= 0) { //確認使用者輸入數字大於零
		printf("You need to input a number > 0\n");
	} else {
		//不超過10
		if (n > MAX_SEQUENCE) {
			printf("That's out of range.\n");
			return 1;
		}

		parent_pid = fork();
		if (parent_pid == 0) { //child process
			//create share memory id
			id = shmget(key, sizeof(share_data), 0666|IPC_CREAT);
			//check id
                        if (id == -1) {
                                printf("id error\n");
                                return 1;
                        }
			// use the id to create share memory
                        data = shmat(id, NULL, 0);
			//check share memory
                        if (data == (void *) -1) {
                                printf("shm address error\n");
                                return 1;
                        }
			//initiallize
                        data->fib[0] = 0;
                        data->fib[1] = 1;
                        data->arg = n;
			printf("Child process-start, pid = %d\n", child_pid);
			//產生費式數列
			if (data->arg > 2) {
				for (int i = 2; i < data->arg; i++) {
					data->fib[i] = data->fib[i - 1] + data->fib[i - 2];
				}
			}
			printf("Child process-end, pid = %d\n", child_pid);
			exit(0);
		} else if (parent_pid > 0) { //parent process
			printf("Parent process-start, pid = %d\n", parent_pid);
			//create share memory id
			id = shmget(key, sizeof(share_data), 0666|IPC_CREAT);
			//check id
			if (id == -1) {
				printf("id error\n");
				return 1;
			} else {
				printf("Create a shared-memory segment, segment_id = %d\n", id);
			}
			// use the id to create share memory
                        data = shmat(id, NULL, 0);
                        //check share memory
                        if (data == (void *) -1) {
                                printf("shm address error\n");
                                return 1;
                        }
                        //initiallize
                        data->fib[0] = 0;
                        data->fib[1] = 1;
                        data->arg = n;
			
			child_pid = wait(&status);
			for (int i = 0; i < data->arg; i++) {
				printf("%ld ", data->fib[i]);
			}
			printf("\n");
			printf("Parent process-end, pid = %d\n", parent_pid);
			//detach the share memory
			if (shmdt(data) == -1) {
				printf("detach error\n");
			}
		} else {
			printf("fork error\n");
		}
	}
	return 0;
}
