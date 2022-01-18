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
    // Expected result is 0.
    printf ("%d\n", ret_code);
    char buf1[4097],buf2[4097];
    ret_code=read (fd[0], buf1, 5);
    printf("%d\n",ret_code);
    // Create child.	
	int pid = fork();
    if (pid < 0) {
        printf ("Somehow not able ro create process!!!\n");
        exit (-1);
    }
	if (!pid) {               // Child code.
		
		// char *read_buffer[6];

        // // Expected result for the fds will be 3 and 4.
		// printf ("%d\n", fd[0]);
        // printf ("%d\n", fd[1]);
        printf ("%d\n", close (fd[1]));
		sleep (5);

        // Read from the pipe.
		ret_code = read (fd[0], buf1, 5);
		if (ret_code < 0) {
		
			printf ("Child: Reading from the pipe is failed!!!\n");
			return -1;

		}
        // Expected return will be 5 and buffer will be "hello".
        printf ("%d\n", ret_code);
		printf ("%s\n", buf1);

        // Close both read and write ends.
        // Expected results should be 0.
		printf ("%d\n", close (fd[0]));
        // Exit.
		exit (1);

	}

    // Write to the pipe.
    printf ("%d\n", close (fd[0]));
	ret_code = write (fd[1], "hello", 5);
	if (ret_code < 0) {
		
		printf ("Parent: Writing to the pipe is failed!!!\n");
		return -1;

	}
    // Expected return should be 5.
    printf ("%d\n", ret_code);
    
    // This is for order between parent and child.
	sleep (30);

	ret_code = write (fd[1], "world", 5);
    printf ("%d\n", ret_code);
	ret_code = read (fd[0], buf1, 5);
	if (ret_code < 0) {
		
		printf ("Parent: Reading from the pipe is failed!!!\n");
		return -1;

	}
    // Expected return will be 5 and buffer will be "world".
    buf1[5] = '\0';
    printf ("%d\n", ret_code);
	printf ("%s\n", buf1);

    // Close pipe ends.
    // expected return will be 0.
    printf ("%d\n", close (fd[1]));

    // Simple return.
	return 0;

}
