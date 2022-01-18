#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc,char * argv[])
{
	unsigned long long x=atoll(argv[argc-1]),y,po;
	//printf("Yo, argc: %d",argc);
	if(argc==2){
		printf("%lld",2*x);
	}
	else {
		char *new_arg[argc];
		int i;
		for(i=1;i<argc-2;i++){
			new_arg[i]=(char*)malloc(sizeof(argv[i+1]));
			strcpy(new_arg[i],argv[i+1]);
		}
		new_arg[0]=(char*)malloc(sizeof(argv[1]));
		char *path=(char*)malloc(sizeof(argv[1])+2);
		path[0]='.';path[1]='/';
		i=0;
		while(1){
			new_arg[0][i]=argv[1][i];
			path[i+2]=argv[1][i];
			if(new_arg[0][i]=='\0')break;
			i++;
		}
		//printf("argv: %s , new_arg[0]: %s",argv[1],new_arg[0]);
		x*=2;
		int len=0;
		y=x;po=1;
		while(y){
			y/=10;len++;
			if(y!=0)po*=10;
		}
		new_arg[argc-2]=(char*)malloc(sizeof(char)*(len+1));
		for(i=0;i<len;i++){
			new_arg[argc-2][i]='0'+(x/po);
			x%=po;
			po/=10;
		}
		new_arg[argc-2][len]='\0';
		new_arg[argc-1]=NULL;
		int pid=fork();
		if(pid < 0){
			perror("UNABLE TO EXECUTE");
			exit(-1);
		}
		if(pid==0){ // child
			if(execv(path,new_arg)==-1){
				perror("UNABLE TO EXECUTE");
				exit(-1);
			}
		}
		else {
			wait(NULL);
			free(path);
			for(i=0;i<argc-1;i++){
				free(new_arg[i]);
			}
		}
	}
	return 0;
}
