#include<ulib.h>


int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the pipe.
	int fd[2];
	
	// Create pipe.
	int ret = pipe(fd);
	
    // Expected result is 0.
    printf ("%d\n", ret);
    char b1[4097];
    int pid = fork();
    if (!pid) {
        sleep(10);
        ret=read(fd[0],b1,2);
        b1[ret]='\0';
        printf("p2 : %d, %s\n",ret,b1);
        printf("p2 : %d\n",write(fd[1],"Yo",2));
        sleep(15);
        ret=read(fd[0],b1,17);
        b1[ret]='\0';
        printf("p2 : %d, %s\n",ret,b1);
        printf("p2 : %d\n",write(fd[1],"Last",4));
        printf("p2: %d\n",close(fd[0]));
        printf("p2: %d\n",close(fd[1]));
        sleep(10);
        // b1[ret]='\0';
        // printf("p2 : %d ,%s\n",ret,b1);
        exit(1);
    }
    int p1=fork();
    if(!p1){
        sleep(10);
        ret=read(fd[0],b1,12);
        b1[ret]='\0';
        printf("p3 : %d, %s\n",ret,b1);
        printf("p3: %d\n",write(fd[1],"Sanjay",6));
        sleep(15);
        ret=read(fd[0],b1,10);
        b1[ret]='\0';
        printf("p3 : %d, %s\n",ret,b1);
        printf("p3: %d\n",write(fd[1],"Sanjay",6));
        ret=read(fd[0],b1,10);
        b1[ret]='\0';
        printf("p3 : %d, %s\n",ret,b1);
        printf("p3: %d\n",close(fd[0]));
        printf("p3: %d\n",close(fd[1]));
        // b1[ret]='\0';
        // printf("p3 : %d ,%s\n",ret,b1);
        exit(1);
    }
    int i;
    for(i=0;i<5;i++){
        b1[i]='a'+ i;
        b1[i+5]='b'+i; 
    }
    ret=write (fd[1], b1, 10);
    printf("p1 : %d\n",ret);
    printf("p1 : %d\n",close(fd[0]));
    printf("p1 : %d\n",close(fd[1]));
    exit(1);
    return 0;

}
