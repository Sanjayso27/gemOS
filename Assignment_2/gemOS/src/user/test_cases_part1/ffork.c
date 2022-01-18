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

    // Create child.	
	int pid = fork();
    if (pid < 0) {
        printf ("Somehow not able ro create process!!!\n");
        exit (-1);
    }

	if (!pid) {               // Child code.
		
		char *read_buffer[6];

        // Expected result for the fds will be 3 and 4.
		printf ("%d\n", fd[0]);
        printf ("%d\n", fd[1]);
		sleep (5);

        // Read from the pipe.
		ret_code = read (fd[0], read_buffer, 5);
        
		if (ret_code < 0) {
		
			printf ("Child: Reading from the pipe is failed!!!\n");
			return -1;

		}
        // Expected return will be 5 and buffer will be "hello".
        printf ("%d\n", ret_code);
		printf ("%s\n", read_buffer);

        int p1=fork();
        if(!p1){
            char A[4097];
            ret_code = read (fd[0], A, 5);
            A[ret_code]='\0';
            printf("cchild read %d, %s\n",ret_code,A);
            int r1=close(fd[0]),r2=close(fd[1]);
            printf("cchild close %d %d\n",r1,r2);
            exit(1);
        }
        char B[4097];
        ret_code = read (fd[0], B, 5);
        B[ret_code]='\0';
        printf("child read %d, %s\n",ret_code,B);
        // Close both read and write ends.
        // Expected results should be 0.
		printf ("%d\n", close (fd[0]));
        printf ("%d\n", close (fd[1]));
        
        // Exit.
		exit (1);

	}

    // Write to the pipe.
	ret_code = write (fd[1], "hellooooooooooo", 15);
	if (ret_code < 0) {
		
		printf ("Parent: Writing to the pipe is failed!!!\n");
		return -1;

	}
    // Expected return should be 5.
    printf ("%d\n", ret_code);
    
    // This is for order between parent and child.
	sleep (30);

    // Expected value of fds in parent will be 3 and 4.
    printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);
	
	// Even if child has closed its end, parent can still write and read from
    // the pipe.
	ret_code = write (fd[1], "world", 5);
	if (ret_code < 0) {
		
		printf ("Parent: Writing again to the pipe is failed!!!\n");
		return -1;

	}
    // Expected return will be 5.
    printf ("%d\n", ret_code);

	char *read_buffer[6];
	ret_code = read (fd[0], read_buffer, 5);
	if (ret_code < 0) {
		
		printf ("Parent: Reading from the pipe is failed!!!\n");
		return -1;

	}
    // Expected return will be 5 and buffer will be "world".
    read_buffer[5] = '\0';
    printf ("%d\n", ret_code);
	printf ("%s\n", read_buffer);

    // Close pipe ends.
    // expected return will be 0.
    printf ("%d\n", close (fd[0]));
    printf ("%d\n", close (fd[1]));

    // Simple return.
	return 0;

}
