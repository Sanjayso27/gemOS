#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char * argv[])
{
	if(strcmp(argv[1],"-c")==0){
		char *dir=(char*)malloc(1000);
		strcpy(dir,argv[2]);
		strcat(dir,"/");
		strcat(dir,argv[3]);
		char buffer[10000];
		int src_fd,dst_fd;
		long long int id,sz;
		dst_fd = open(dir, O_CREAT|O_WRONLY,0644);
		char* file_name_out=(char*)malloc(17+10);
		char* file_name_in=(char*)malloc(1000+17+10);
		char* len=(char*)malloc(11+10);
		DIR* inp;
		inp=opendir(argv[2]);
		struct dirent *ent,*ent1;
		long long int cnt_files=0;
		while ((ent1 = readdir (inp)) != NULL) {
			if(ent1->d_type!=DT_REG)
				continue;
			if(strcmp(ent1->d_name,argv[3])==0)
				continue;
			cnt_files++;
		}
		sprintf(len,"%lld",cnt_files);
		strncpy(buffer,len,11);
		id = write(dst_fd, buffer, 11);
		if (id == -1) {
			printf("Failed to complete creation operation\n");
			exit(1);
		}
		closedir (inp);
		inp=opendir(argv[2]);
		while ((ent = readdir (inp)) != NULL) {
			if(ent->d_type!=DT_REG)
				continue;
			if(strcmp(ent->d_name,argv[3])==0)
				continue;
			strcpy(file_name_in,argv[2]);
			strcat(file_name_in,"/");
			strcat(file_name_in,ent->d_name);
			src_fd = open(file_name_in, O_RDONLY);
			//printf("src:%d,dst:%d\n",src_fd,dst_fd);
			strncpy(file_name_out,ent->d_name,17);
			strncpy(buffer,file_name_out,17);
			id = write(dst_fd, buffer, 17);
			if (id == -1) {
				printf("Failed to complete creation operation\n");
				exit(1);
			}
			struct stat sbuf;
			stat(file_name_in, &sbuf);
			sz=sbuf.st_size;
			sprintf(len,"%lld",sz);
			strncpy(buffer,len,11);
			id = write(dst_fd, buffer, 11);
			if (id == -1) {
				printf("Failed to complete creation operation\n");
				exit(1);
			}
			while(1){
				if(sz==0)break;
				if(sz<=10000){
					id = read(src_fd, buffer, sz);
					sz=0;
				}
				else {
					id = read(src_fd, buffer, 10000);
					sz-=10000;
				}
				if(id == -1){
					printf("Failed to complete creation operation\n");
					exit(1);
				}
				id = write(dst_fd, buffer, id);
				if (id == -1) {
					printf("Failed to complete creation operation\n");
					exit(1);
				}
			}
			close(src_fd);
		}
		close(dst_fd);
		//free(dir); free(len); free(file_name_in); free(file_name_out);
	}
	else if(strcmp(argv[1],"-d")==0){
		int len=strlen(argv[2]);
		char dump[len];
		strncpy(dump,argv[2],len-4);
		dump[len-4]='\0';
		strcat(dump,"Dump");
		mkdir(dump,0777);
		int src_fd = open(argv[2], O_RDONLY),dst_fd,id;
		long long sz;
		char* file_name_out=(char*)malloc(17+10);
		char* num=(char*)malloc(11+10);
		char buffer[10000];
		char* file1=(char*)malloc(len+20+10);
		char *tempfile=(char*)malloc(len+20+10);
		strncpy(file1,dump,len);
		file1[len]='\0';
		strcat(file1,"/");
		read(src_fd,buffer,11);
		// printf("%s\n",file1);
		while(1){
			id = read(src_fd, buffer, 17);
			if(id==0)break;
			if(id == -1){
				printf("Failed to complete extraction operation\n");
				exit(1);
			}
			strncpy(file_name_out,buffer,17);
			id = read(src_fd, buffer, 11);
			if(id == -1){
				printf("Failed to complete extraction operation\n");
				exit(1);
			}
			strcpy(tempfile,file1);
			strcat(tempfile,file_name_out);
			strncpy(num,buffer,11);
			sz=atoll(num);
			dst_fd=open(tempfile,O_CREAT|O_WRONLY,0644);
			while(1){
				if(sz==0)break;
				if(sz<=10000){
					id = read(src_fd, buffer, sz);
					sz=0;
				}
				else {
					id = read(src_fd, buffer, 10000);
					sz-=10000;
				}
				if(id == -1){
					printf("Failed to complete extraction operation\n");
					exit(1);
				}
				id = write(dst_fd, buffer, id);
				if (id == -1) {
					printf("Failed to complete extraction operation\n");
					exit(1);
				}
			}
			close(dst_fd);
		}
		close(src_fd);
	}
	else if(strcmp(argv[1],"-e")==0){
		int len=strlen(argv[2]);
		char dump[len+20];
		int spl;
		for(int i=len-1;i>=0;i--){
			if (argv[2][i]=='/'){
				spl=i;break;
			}
		}
		strncpy(dump,argv[2],spl+1);
		dump[spl+1]='\0';
		strcat(dump,"IndividualDump");
		mkdir(dump,0777);
		int src_fd = open(argv[2], O_RDONLY),dst_fd,id;
		long long sz;
		char* file_name_out=(char*)malloc(17+10);
		char* num=(char*)malloc(11+10);
		char buffer[10000];
		char* file1=(char*)malloc(len+20+10);
		char *tempfile=(char*)malloc(len+20+10);
		strcpy(file1,dump);
		strcat(file1,"/");
		read(src_fd,buffer,11);
		while(1){
			id = read(src_fd, buffer, 17);
			if(id==0)break;
			if(id == -1){
				printf("Failed to complete extraction operation\n");
				exit(1);
			}
			strncpy(file_name_out,buffer,17);
			id = read(src_fd, buffer, 11);
			if(id == -1){
				printf("Failed to complete extraction operation\n");
				exit(1);
			}
			strncpy(num,buffer,11);
			sz=atoll(num);
			if(strcmp(file_name_out,argv[3])!=0){
				lseek(src_fd,sz,SEEK_CUR);
				continue;
			}
			strcpy(tempfile,file1);
			strcat(tempfile,file_name_out);
			dst_fd=open(tempfile,O_CREAT|O_WRONLY,0644);
			while(1){
				if(sz==0)break;
				if(sz<=10000){
					id = read(src_fd, buffer, sz);
					sz=0;
				}
				else {
					id = read(src_fd, buffer, 10000);
					sz-=10000;
				}
				if(id == -1){
					printf("Failed to complete extraction operation\n");
					exit(1);
				}
				id = write(dst_fd, buffer, id);
				if (id == -1) {
					printf("Failed to complete extraction operation\n");
					exit(1);
				}
			}
			close(dst_fd);
			close(src_fd);
			break;
		}
	}
	else if(strcmp(argv[1],"-l")==0){
		int len=strlen(argv[2]);
		char dump[len];
		strncpy(dump,argv[2],len-4);
		dump[len-4]='\0';
		int spl;
		for(int i=len-1;i>=0;i--){
			if(dump[i]=='/'){
				spl=i;break;
			}
		}
		char file_name[len+20];
		strncpy(file_name,argv[2],spl+1);
		file_name[spl+1]='\0';
		strcat(file_name,"tarStructure");
		int src_fd = open(argv[2], O_RDONLY),dst_fd=open(file_name,O_CREAT|O_WRONLY,0644),id;
		long long sz;
		char* file_name_out=(char*)malloc(17+10);
		char* num=(char*)malloc(11+10);
		char buffer[10000];
		struct stat sbuf;
		stat(argv[2], &sbuf);
		sz=sbuf.st_size;
		sprintf(num,"%lld",sz);
		strncpy(buffer,num,11);
		strcat(buffer,"\n");
		id = write(dst_fd, buffer, strlen(buffer));
		if(id == -1){
			printf("Failed to complete list operation\n");
			exit(1);
		}
		id=read(src_fd,buffer,11);
		if(id == -1){
			printf("Failed to complete list operation\n");
			exit(1);
		}
		strcat(buffer,"\n");
		id=write(dst_fd,buffer,strlen(buffer));
		if(id == -1){
			printf("Failed to complete list operation\n");
			exit(1);
		}
		while(1){
			id = read(src_fd, buffer, 17);
			if(id==0)break;
			if(id == -1){
				printf("Failed to complete list operation\n");
				exit(1);
			}
			strcat(buffer," ");
			id = write(dst_fd, buffer, strlen(buffer));
			if(id == -1){
				printf("Failed to complete list operation\n");
				exit(1);
			}
			id = read(src_fd, buffer, 11);
			if(id == -1){
				printf("Failed to complete list operation\n");
				exit(1);
			}
			strcat(buffer,"\n");
			id = write(dst_fd, buffer, strlen(buffer));
			if(id == -1){
				printf("Failed to complete list operation\n");
				exit(1);
			}
			strncpy(num,buffer,11);
			sz=atoll(num);
			lseek(src_fd,sz,SEEK_CUR);
		}
		close(src_fd);
		close(dst_fd);
		// free(num); free(file_name_out);
	}
	
	return 0;
}
