#include<ulib.h>


int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the pipe.
	int fd[2];
	
	// Create pipe.
	int ret = ppipe(fd);
	
    // Expected result is 0.
    printf ("%d\n", r);
    int pid = fork();
    if (!pid) {
        sleep(10);
        char b1[4097];
        ret=read(fd[0],b1,500);
        b1[ret]='\0';
        printf("p2 : %d ,%s\n",ret,b1);
        exit(1);
    }
    int p1=fork();
    if(!p1){
        sleep(10);
        char b1[4097];
        ret=read(fd[0],b1,200);
        b1[ret]='\0';
        printf("p3 : %d ,%s\n",ret,b1);
        exit(1);
    }
    char buf1[4097];
    int i;
    for(i=0;i<1000;i++){
        buf1[i]='a'+ i%10;
    }
    ret=write (fd[1], buf1, 1000);
    printf("p1 : %d\n",ret);
    return 0;

}
