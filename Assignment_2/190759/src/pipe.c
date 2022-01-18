#include<pipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>


// Per process info for the pipe.
struct pipe_info_per_process {

    // TODO:: Add members as per your need...
    int open_r,open_w;
    int id;
};

// Global information for the pipe.
struct pipe_info_global {

    char *pipe_buff;    // Pipe buffer: DO NOT MODIFY THIS.

    // TODO:: Add members as per your need...
    int r,w,empty,full;
    int proc;
};

// Pipe information structure.
// NOTE: DO NOT MODIFY THIS STRUCTURE.
struct pipe_info {

    struct pipe_info_per_process pipe_per_proc [MAX_PIPE_PROC];
    struct pipe_info_global pipe_global;

};


// Function to allocate space for the pipe and initialize its members.
struct pipe_info* alloc_pipe_info () {
	
    // Allocate space for pipe structure and pipe buffer.
    struct pipe_info *pipe = (struct pipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);
    if(pipe==NULL || buffer == NULL)
        return NULL;
    // Assign pipe buffer.
    pipe->pipe_global.pipe_buff = buffer;

    /**
     *  TODO:: Initializing pipe fields
     *  
     *  Initialize per process fields for this pipe.
     *  Initialize global fields for this pipe.
     *
     */
    pipe->pipe_global.r=0;
    pipe->pipe_global.w=0;
    pipe->pipe_global.empty=1;
    pipe->pipe_global.full=0;
    pipe->pipe_global.proc=1;
    pipe->pipe_per_proc[0].open_r=1;
    pipe->pipe_per_proc[0].open_w=1;
    // Return the pipe.
    return pipe;

}

// Function to free pipe buffer and pipe info object.
// NOTE: DO NOT MODIFY THIS FUNCTION.
void free_pipe (struct file *filep) {

    os_page_free(OS_DS_REG, filep->pipe->pipe_global.pipe_buff);
    os_page_free(OS_DS_REG, filep->pipe);

}

// Fork handler for the pipe.
int do_pipe_fork (struct exec_context *child, struct file *filep) {

    /**
     *  TODO:: Implementation for fork handler
     *
     *  You may need to update some per process or global info for the pipe.
     *  This handler will be called twice since pipe has 2 file objects.
     *  Also consider the limit on no of processes a pipe can have.
     *  Return 0 on success.
     *  Incase of any error return -EOTHERS.
     *
     */
    if(filep->pipe==NULL)
        return -EOTHERS;
    int cnt=(filep->pipe)->pipe_global.proc;
    int p1=child->pid,p2=child->ppid,i,ind;
    int chk=0;
    for(i=0;i<cnt;i++){
        if(((filep->pipe)->pipe_per_proc)[i].id==p1){
            chk=1;
        }
        if(((filep->pipe)->pipe_per_proc)[i].id==p2){
            ind=i;
        }
    }
    //printk("|****|\nfork call %d\n|****|\n ",cnt);
    if(!chk){
        if(cnt==MAX_PIPE_PROC){
            return -EOTHERS;
        }
        ((filep->pipe)->pipe_per_proc)[cnt].open_r=((filep->pipe)->pipe_per_proc)[ind].open_r;
        ((filep->pipe)->pipe_per_proc)[cnt].open_w=((filep->pipe)->pipe_per_proc)[ind].open_w;
        ((filep->pipe)->pipe_per_proc)[cnt].id=p1;
        (filep->pipe)->pipe_global.proc++;
    }
    ////printk("|****|\nfork call %d\n|****|\n ",cnt);
    // Return successfully.
    return 0;

}

// Function to close the pipe ends and free the pipe when necessary.
long pipe_close (struct file *filep) {

    /**
     *  TODO:: Implementation of Pipe Close
     *
     *  Close the read or write end of the pipe depending upon the file
     *      object's mode.
     *  You may need to update some per process or global info for the pipe.
     *  Use free_pipe() function to free pipe buffer and pipe object,
     *      whenever applicable.
     *  After successful close, it return 0.
     *  Incase of any error return -EOTHERS.
     *
     */
    if(filep->pipe==NULL)
        return -EOTHERS;
    int ret_value;
    struct exec_context *cur=get_current_ctx();
    int cnt=(filep->pipe)->pipe_global.proc;
    int p1=cur->pid,ind=-1,i;
    for(i=0;i<cnt;i++){
        if(((filep->pipe)->pipe_per_proc)[i].id==p1){
            ind=i;
        }
    }
    if(ind==-1){
        //printk("|****|\npipe_close\n|****|\n");
        return -EOTHERS;
    }
    if(filep->mode == O_READ){
        (filep->pipe)->pipe_per_proc[ind].open_r=0;
    }
    else if(filep->mode == O_WRITE){
        (filep->pipe)->pipe_per_proc[ind].open_w=0;
    }
    if(((filep->pipe)->pipe_per_proc[ind].open_r==0) && 
        ((filep->pipe)->pipe_per_proc[ind].open_w==0)){
            for(i=ind;i<cnt-1;i++){
                (filep->pipe)->pipe_per_proc[i]=(filep->pipe)->pipe_per_proc[i+1];
            }
            cnt--;
    }
    (filep->pipe)->pipe_global.proc=cnt;
    if(cnt==0){
        free_pipe(filep);
    }
    // Close the file and return.
    ret_value = file_close (filep);         // DO NOT MODIFY THIS LINE.

    // And return.
    return ret_value;

}

// Check whether passed buffer is valid memory location for read or write.
int is_valid_mem_range (unsigned long buff, u32 count, int access_bit) {

    /**
     *  TODO:: Implementation for buffer memory range checking
     *
     *  Check whether passed memory range is suitable for read or write.
     *  If access_bit == 1, then it is asking to check read permission.
     *  If access_bit == 2, then it is asking to check write permission.
     *  If range is valid then return 1.
     *  Incase range is not valid or have some permission issue return -EBADMEM.
     *
     */
    int ret_value = -EBADMEM;

    struct exec_context *cur=get_current_ctx();
    struct vm_area *vm=cur->vm_area;
    while(vm!=NULL){
        if(buff>=vm->vm_start && buff<=vm->vm_end){
            if(buff+count-1<=vm->vm_end){
                if((vm->access_flags)&(1<<(access_bit-1))){
                    ret_value=1;
                }
            }
        }
    }
    int i;
    for(i=0;i<MAX_MM_SEGS-1;i++){
        if(buff>=(cur->mms[i]).start && buff<=(cur->mms[i]).end){
            if(buff+count-1<=(cur->mms[i]).next_free){
                if(((cur->mms[i]).access_flags) & (1<<(access_bit-1))){
                    ret_value=1;
                }
            }
        }
    }
    if(buff>=(cur->mms[i]).start && buff+count-1<=(cur->mms[i]).end){
        if((cur->mms[i]).access_flags & (1<<(access_bit-1))){
            ret_value=1;
        }
    }
    // Return the finding.
    return ret_value;

}

// Function to read given no of bytes from the pipe.
int pipe_read (struct file *filep, char *buff, u32 count) {

    /**
     *  TODO:: Implementation of Pipe Read
     *
     *  Read the data from pipe buffer and write to the provided buffer.
     *  If count is greater than the present data size in the pipe then just read
     *       that much data.
     *  Validate file object's access right.
     *  On successful read, return no of bytes read.
     *  Incase of Error return valid error code.
     *       -EACCES: In case access is not valid.
     *       -EINVAL: If read end is already closed.
     *       -EOTHERS: For any other errors.
     *
     */
    if(filep==NULL || (filep->pipe == NULL)){
        //printk("|****|\npipe_read\n|****|\n");
        return -EOTHERS;   
    }
    if(filep->mode != O_READ){
        return -EACCES;
    }
    struct exec_context *cur=get_current_ctx();
    int cnt=(filep->pipe)->pipe_global.proc;
    int pi=cur->pid,ind,i;
    for(i=0;i<cnt;i++){
        if(((filep->pipe)->pipe_per_proc)[i].id==pi){
            ind=i;
        }
    }
    if((filep->pipe)->pipe_per_proc[ind].open_r!=1){
        return -EINVAL;
    }
    unsigned long temp=(unsigned long)buff;
    if(count==0) return 0;
    if(is_valid_mem_range(temp,count,2)!=1){
        return -EBADMEM;
    }
    int bytes_read = 0;
    int p1=(filep->pipe)->pipe_global.r;
    int p2=(filep->pipe)->pipe_global.w;
    if((filep->pipe)->pipe_global.empty){
        return bytes_read;
    }
    i=0;
    if((filep->pipe)->pipe_global.full){
        buff[0]=((filep->pipe)->pipe_global).pipe_buff[p1];
        p1=(p1+1)%MAX_PIPE_SIZE;
        i++;
        bytes_read++;
        (filep->pipe)->pipe_global.full=0;
    }
    for(;i<count;i++){
        if(p1==p2)break;
        bytes_read++;
        buff[i]=((filep->pipe)->pipe_global).pipe_buff[p1];
        p1=(p1+1)%MAX_PIPE_SIZE;
    }
    (filep->pipe)->pipe_global.r=p1;
    if(p1==p2){
        (filep->pipe)->pipe_global.empty=1;
    }
    // Return no of bytes read.
    return bytes_read;

}

// Function to write given no of bytes to the pipe.
int pipe_write (struct file *filep, char *buff, u32 count) {

    /**
     *  TODO:: Implementation of Pipe Write
     *
     *  Write the data from the provided buffer to the pipe buffer.
     *  If count is greater than available space in the pipe then just write data
     *       that fits in that space.
     *  Validate file object's access right.
     *  On successful write, return no of written bytes.
     *  Incase of Error return valid error code.
     *       -EACCES: In case access is not valid.
     *       -EINVAL: If write end is already closed.
     *       -EOTHERS: For any other errors.
     *
     */
    if(filep==NULL || (filep->pipe == NULL)){
        //printk("|****|\npipe_write\n|****|\n");
        return -EOTHERS;
    }
    if(filep->mode != O_WRITE){
        return -EACCES;
    }
    struct exec_context *cur=get_current_ctx();
    int cnt=(filep->pipe)->pipe_global.proc;
    int pi=cur->pid,ind,i;
    for(i=0;i<cnt;i++){
        if(((filep->pipe)->pipe_per_proc)[i].id==pi){
            ind=i;
        }
    }
    if((filep->pipe)->pipe_per_proc[ind].open_w!=1){
        return -EINVAL;
    }
    unsigned long temp=(unsigned long)buff;
    if(count==0)return 0;
    if(is_valid_mem_range(temp,count,1)!=1){
        return -EBADMEM;
    }
    int bytes_written = 0;
    int p1=(filep->pipe)->pipe_global.r;
    int p2=(filep->pipe)->pipe_global.w;
    if((filep->pipe)->pipe_global.full){
        return bytes_written;
    }
    i=0;
    if((filep->pipe)->pipe_global.empty){
        ((filep->pipe)->pipe_global).pipe_buff[p2]=buff[0];
        p2=(p2+1)%MAX_PIPE_SIZE;
        i++;
        bytes_written++;
        (filep->pipe)->pipe_global.empty=0;
    }
    for(;i<count;i++){
        if(p2==p1)break;
        bytes_written++;
        ((filep->pipe)->pipe_global).pipe_buff[p2]=buff[i];
        p2=(p2+1)%MAX_PIPE_SIZE;
    }
    (filep->pipe)->pipe_global.w=p2;
    if(p1==p2){
        (filep->pipe)->pipe_global.full=1;
    }
    // Return no of bytes written.
    return bytes_written;

}

// Function to create pipe.
int create_pipe (struct exec_context *current, int *fd) {

    /**
     *  TODO:: Implementation of Pipe Create
     *
     *  Find two free file descriptors.
     *  Create two file objects for both ends by invoking the alloc_file() function. 
     *  Create pipe_info object by invoking the alloc_pipe_info() function and
     *       fill per process and global info fields.
     *  Fill the fields for those file objects like type, fops, etc.
     *  Fill the valid file descriptor in *fd param.
     *  On success, return 0.
     *  Incase of Error return valid Error code.
     *       -ENOMEM: If memory is not enough.
     *       -EOTHERS: Some other errors.
     *
     */
    int fd_id,count=0;
    struct pipe_info* cur_pipe = alloc_pipe_info();
    if(cur_pipe==NULL)
        return -ENOMEM;
    int p1=current->pid;
    ((cur_pipe)->pipe_per_proc)[0].id=p1;
    for (fd_id = 0; fd_id < MAX_OPEN_FILES; ++fd_id) {
        if(count>=2)break;
		if (current->files[fd_id] == NULL) {
            current->files[fd_id]=alloc_file();
            if(current->files[fd_id]==NULL)
                return -ENOMEM;
            (current->files[fd_id])->fops->read= pipe_read;
            (current->files[fd_id])->fops->write= pipe_write;
            (current->files[fd_id])->fops->close= pipe_close;
            (current->files[fd_id])->pipe=cur_pipe;
            if(count==0)(current->files[fd_id])->mode = O_READ;
            else (current->files[fd_id])->mode = O_WRITE;
            (current->files[fd_id])->type=PIPE;
            fd[count]=fd_id;
            count++;
        }
    }
    if(count<2){
        //printk("|****|\ncreate_pipe\n|****|\n");
        return -EOTHERS;
    }
    // Simple return.
    return 0;

}
