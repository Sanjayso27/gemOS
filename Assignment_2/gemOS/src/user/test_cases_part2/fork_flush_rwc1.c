#include<ulib.h>


int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the pipe.
	int fd[2];
	
	// Create pipe.
	int ret = ppipe(fd);
	
    // Expected result is 0.
    printf ("%d\n", ret);
    int pid = fork();
    if (!pid) {
        sleep(10);
        char b1[4097];
        ret=read(fd[0],b1,500);
        printf("p2 : %d\n",ret);
        sleep(15);
        printf("p2 : %d\n",write(fd[1],b1,2196));
        sleep(10);
        printf("p2 : %d\n",write(fd[1],b1,200));
        printf("p2 : %d\n",flush_ppipe(fd));
        printf("p2 : %d\n",read(fd[0],b1,3000));
        sleep(10);
        printf("p2 : %d\n",write(fd[1],b1,400));
        printf("p2 : %d\n",read(fd[0],b1,700));
        printf("p2 : %d\n",flush_ppipe(fd));
        printf("p2: %d\n",close(fd[0]));
        printf("p2 : %d\n",flush_ppipe(fd));
        // b1[ret]='\0';
        // printf("p2 : %d ,%s\n",ret,b1);
        exit(1);
    }
    int p1=fork();
    if(!p1){
        sleep(10);
        char b1[4097];
        ret=read(fd[0],b1,200);
        printf("p3 : %d\n",ret);
        int i;
        for(i=0;i<1000;i++){
            b1[i]='b'+ i%10;
        }
        ret=write(fd[1],b1,1000);
        printf("p3 : %d\n",ret);
        printf("p3: %d\n",flush_ppipe(fd));
        sleep(10);
        printf("p3: %d\n",read(fd[0],b1,3000));
        sleep(10);
        printf("p3: %d\n",read(fd[0],b1,2196));
        printf("p3: %d\n",close(fd[0]));
        sleep(10);
        // b1[ret]='\0';
        // printf("p3 : %d ,%s\n",ret,b1);
        exit(1);
    }
    char buf1[4097];
    int i;
    for(i=0;i<1000;i++){
        buf1[i]='a'+ i%10;
    }
    ret=write (fd[1], buf1, 1000);
    printf("p1 : %d\n",ret);
    sleep(20);
    printf("p1 : %d\n",close(fd[0]));
    printf("p1: %d\n",flush_ppipe(fd));
    sleep(30);
    return 0;

}
