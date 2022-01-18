#include<ulib.h>


int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the pipe.
	int fd[2];
	
	// Create pipe.
	int ret_code = pipe(fd);
	
	// Check for any error in pipe creation.
	if (ret_code < 0) {

		printf("Pipe allocation failed!!!\n");
		return -1;
	
	}
    printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);
	
	char buf1[4097],buf2[4097];
    for(int i=0;i<4096;i++){
        buf1[i]='a'+i%10;
    }
	ret_code = write (fd[1], buf1, 4095);
	if (ret_code < 0) {
		
		printf ("Parent: Writing again to the pipe is failed!!!\n");
		return -1;

	}
    // Expected return will be 4096.
    printf ("%d\n", ret_code);
    ret_code = write (fd[1], "Sanjay", 3);
    printf ("%d\n", ret_code);
	ret_code = read (fd[0], buf2, 4090);
	if (ret_code < 0) {
		
		printf ("Parent: Reading from the pipe is failed!!!\n");
		return -1;

	}
    // Expected return will be 5 and buffer will be "world".
    printf ("%d\n", ret_code);
    ret_code = read (fd[0], buf2, 2);
    buf2[ret_code]='\0';
	printf("%s\n",buf2);
    int pid = fork();
    if (!pid) {
        ret_code = read (fd[0], buf2, 2);
        if (ret_code < 0) {
            printf ("Parent: Reading from the pipe is failed!!!\n");
            return -1;
        }
        printf ("%d\n", ret_code);
        buf2[ret_code]='\0';
        printf("%s\n",buf2);
        buf1[0]='h';buf1[1]='i';
        ret_code = write(fd[1], buf1, 10);
        if (ret_code < 0) {
            printf ("Parent: Reading from the pipe is failed!!!\n");
            return -1;
        }
        printf ("%d\n", ret_code);
        exit(1);
    }
    sleep(10);
    ret_code = read (fd[1], buf2, 12); //12
    if (ret_code < 0) {
        printf ("Parent: Reading from the pipe is failed!!!\n");
        return -1;
    }
    printf ("%d\n", ret_code);
    buf2[ret_code]='\0';
    printf("%s\n",buf2);
    // Close pipe ends.
    // expected return will be 0.
    printf ("%d\n", close (fd[0]));
    printf ("%d\n", close (fd[1]));

    // Simple return.
	return 0;

}