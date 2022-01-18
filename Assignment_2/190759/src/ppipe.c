#include<ppipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>


// Per process information for the ppipe.
struct ppipe_info_per_process {

    // TODO:: Add members as per your need...
    int open_r,open_w;
    int id;
    int r,empty,full;
};

// Global information for the ppipe.
struct ppipe_info_global {

    char *ppipe_buff;       // Persistent pipe buffer: DO NOT MODIFY THIS.

    // TODO:: Add members as per your need...
    int r,w,empty,full;
    int proc;
};

// Persistent pipe structure.
// NOTE: DO NOT MODIFY THIS STRUCTURE.
struct ppipe_info {

    struct ppipe_info_per_process ppipe_per_proc [MAX_PPIPE_PROC];
    struct ppipe_info_global ppipe_global;

};


// Function to allocate space for the ppipe and initialize its members.
struct ppipe_info* alloc_ppipe_info() {

    // Allocate space for ppipe structure and ppipe buffer.
    struct ppipe_info *ppipe = (struct ppipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);
    if(ppipe==NULL || buffer == NULL)
        return NULL;
    // Assign ppipe buffer.
    ppipe->ppipe_global.ppipe_buff = buffer;

    /**
     *  TODO:: Initializing pipe fields
     *
     *  Initialize per process fields for this ppipe.
     *  Initialize global fields for this ppipe.
     *
     */ 
    ppipe->ppipe_global.r=0;
    ppipe->ppipe_global.w=0;
    ppipe->ppipe_global.empty=1;
    ppipe->ppipe_global.full=0;
    ppipe->ppipe_global.proc=1;
    ppipe->ppipe_per_proc[0].open_r=1;
    ppipe->ppipe_per_proc[0].open_w=1;
    ppipe->ppipe_per_proc[0].empty=1;
    ppipe->ppipe_per_proc[0].full=0;
    ppipe->ppipe_per_proc[0].r=0;
    // Return the ppipe.
    return ppipe;

}

// Function to free ppipe buffer and ppipe info object.
// NOTE: DO NOT MODIFY THIS FUNCTION.
void free_ppipe (struct file *filep) {

    os_page_free(OS_DS_REG, filep->ppipe->ppipe_global.ppipe_buff);
    os_page_free(OS_DS_REG, filep->ppipe);

} 

// Fork handler for ppipe.
int do_ppipe_fork (struct exec_context *child, struct file *filep) {
    
    /**
     *  TODO:: Implementation for fork handler
     *
     *  You may need to update some per process or global info for the ppipe.
     *  This handler will be called twice since ppipe has 2 file objects.
     *  Also consider the limit on no of processes a ppipe can have.
     *  Return 0 on success.
     *  Incase of any error return -EOTHERS.
     *
     */
    //printk("Yo\n");
    if(filep->ppipe==NULL)
        return -EOTHERS;
    int cnt=(filep->ppipe)->ppipe_global.proc;
    int p1=child->pid,p2=child->ppid,i,ind;
    int chk=0;
    //printk("|--|\n cnt: %d|--|\n",cnt);
    for(i=0;i<cnt;i++){
        if(((filep->ppipe)->ppipe_per_proc)[i].id==p1){
            chk=1;
        }
        if(((filep->ppipe)->ppipe_per_proc)[i].id==p2){
            ind=i;
        }
    }
    if(!chk){
        if(cnt==MAX_PPIPE_PROC){
            return -EOTHERS;
        }
        ((filep->ppipe)->ppipe_per_proc)[cnt].open_r=((filep->ppipe)->ppipe_per_proc)[ind].open_r;
        ((filep->ppipe)->ppipe_per_proc)[cnt].open_w=((filep->ppipe)->ppipe_per_proc)[ind].open_w;
        ((filep->ppipe)->ppipe_per_proc)[cnt].empty=((filep->ppipe)->ppipe_per_proc)[ind].empty;
        ((filep->ppipe)->ppipe_per_proc)[cnt].full=((filep->ppipe)->ppipe_per_proc)[ind].full;
        ((filep->ppipe)->ppipe_per_proc)[cnt].r=((filep->ppipe)->ppipe_per_proc)[ind].r;
        ((filep->ppipe)->ppipe_per_proc)[cnt].id=p1;
        (filep->ppipe)->ppipe_global.proc++;
    }
    //printk("|--|\n cnt: %d|--|\n",cnt);
    // Return successfully.
    return 0;

}


// Function to close the ppipe ends and free the ppipe when necessary.
long ppipe_close (struct file *filep) {

    /**
     *  TODO:: Implementation of Pipe Close
     *
     *  Close the read or write end of the ppipe depending upon the file
     *      object's mode.
     *  You may need to update some per process or global info for the ppipe.
     *  Use free_pipe() function to free ppipe buffer and ppipe object,
     *      whenever applicable.
     *  After successful close, it return 0.
     *  Incase of any error return -EOTHERS.
     *                                                                          
     */
    if(filep->ppipe==NULL)
        return -EOTHERS;
    int ret_value;
    struct exec_context *cur=get_current_ctx();
    int cnt=(filep->ppipe)->ppipe_global.proc;
    int p1=cur->pid,ind=-1,i;
    for(i=0;i<cnt;i++){
        if(((filep->ppipe)->ppipe_per_proc)[i].id==p1){
            ind=i;
        }
    }
    if(ind==-1)
        return -EOTHERS;
    if(filep->mode == O_READ){
        (filep->ppipe)->ppipe_per_proc[ind].open_r=0;
    }
    else if(filep->mode == O_WRITE){
        (filep->ppipe)->ppipe_per_proc[ind].open_w=0;
    }
    if(((filep->ppipe)->ppipe_per_proc[ind].open_r==0) && 
        ((filep->ppipe)->ppipe_per_proc[ind].open_w==0)){
            for(i=ind;i<cnt-1;i++){
                (filep->ppipe)->ppipe_per_proc[i]=(filep->ppipe)->ppipe_per_proc[i+1];
            }
            cnt--;
    }
    (filep->ppipe)->ppipe_global.proc=cnt;
    if(cnt==0){
        free_ppipe(filep);
    }

    // Close the file.
    ret_value = file_close (filep);         // DO NOT MODIFY THIS LINE.

    // And return.
    return ret_value;

}

// Function to perform flush operation on ppipe.
int do_flush_ppipe (struct file *filep) {

    /**
     *  TODO:: Implementation of Flush system call
     *
     *  Reclaim the region of the persistent pipe which has been read by 
     *      all the processes.
     *  Return no of reclaimed bytes.
     *  In case of any error return -EOTHERS.
     *
     */
    if(filep->ppipe==NULL)
        return -EOTHERS;
    int reclaimed_bytes = 0;
    int p1=(filep->ppipe)->ppipe_global.r,p2=(filep->ppipe)->ppipe_global.w;
    int cnt=(filep->ppipe)->ppipe_global.proc;
    int all_close=1;
    for(int i=0;i<cnt;i++){
        if((filep->ppipe)->ppipe_per_proc[i].open_r==1)all_close=0;
    }
    if((filep->ppipe)->ppipe_global.empty || (all_close)){
        return reclaimed_bytes;
    }
    int r,chk,ok=0;
    //printk("|--|\n cnt: %d|--|\n",cnt);
    for(int i=0;i<MAX_PPIPE_SIZE;i++){
        ok=0;
        for(int j=0;j<cnt;j++){
            if((filep->ppipe)->ppipe_per_proc[j].open_r==0)continue;
            r=((filep->ppipe)->ppipe_per_proc[j]).r;
            chk=((filep->ppipe)->ppipe_per_proc[j]).empty;
            if((i==0) && (p1==r) && chk)continue;
            //printk("j: %d\n",j);
            if(p1==r){
                ok=1;break;
            }
        }
        if(ok)break;
        p1=(p1+1)%MAX_PPIPE_SIZE;
        reclaimed_bytes++;
    }
    (filep->ppipe)->ppipe_global.r=p1;
    // Return reclaimed bytes.
    if(reclaimed_bytes){
        (filep->ppipe)->ppipe_global.full=0;
        if(p1==p2)
            (filep->ppipe)->ppipe_global.empty=1;
    }
    return reclaimed_bytes;

}

// Read handler for the ppipe.
int ppipe_read (struct file *filep, char *buff, u32 count) {
    
    /**
     *  TODO:: Implementation of PPipe Read
     *
     *  Read the data from ppipe buffer and write to the provided buffer.
     *  If count is greater than the present data size in the ppipe then just read
     *      that much data.
     *  Validate file object's access right.
     *  On successful read, return no of bytes read.
     *  Incase of Error return valid error code.
     *      -EACCES: In case access is not valid.
     *      -EINVAL: If read end is already closed.
     *      -EOTHERS: For any other errors.
     *
     */
    if(filep==NULL || (filep->ppipe == NULL))
        return -EOTHERS;
    if(filep->mode != O_READ){
        return -EACCES;
    }
    struct exec_context *cur=get_current_ctx();
    int cnt=(filep->ppipe)->ppipe_global.proc;
    int pi=cur->pid,ind,i=0;
    for(i=0;i<cnt;i++){
        if(((filep->ppipe)->ppipe_per_proc)[i].id==pi){
            ind=i;
        }
    }
    if((filep->ppipe)->ppipe_per_proc[ind].open_r!=1){
        return -EINVAL;
    }
    int bytes_read = 0;
    if(count==0) return 0;
    int p1=((filep->ppipe)->ppipe_per_proc[ind]).r;
    int p2=(filep->ppipe)->ppipe_global.w;
    if(((filep->ppipe)->ppipe_per_proc[ind]).empty){
        return bytes_read;
    }
    i=0;
    //printk("|--|\n p1: %d,ind: %d\n |--|\n",p1);
    if(((filep->ppipe)->ppipe_per_proc[ind]).full){
        buff[0]=((filep->ppipe)->ppipe_global).ppipe_buff[p1];
        p1=(p1+1)%MAX_PPIPE_SIZE;
        bytes_read++;
        i++;
        ((filep->ppipe)->ppipe_per_proc[ind]).full=0;
    }
    for(;i<count;i++){
        if(p1==p2)break;
        bytes_read++;
        buff[i]=((filep->ppipe)->ppipe_global).ppipe_buff[p1];
        p1=(p1+1)%MAX_PPIPE_SIZE;
    }
    ((filep->ppipe)->ppipe_per_proc[ind]).r=p1;
    if(p1==p2){
        ((filep->ppipe)->ppipe_per_proc[ind]).empty=1;
    }
    // //printk("|--|\n cnt: %d|--|\n",cnt);
    // for(i=0;i<cnt;i++){
    //     p1=((filep->ppipe)->ppipe_per_proc[i]).r;
    //     //printk("|--|\n p1: %d,ind: %d\n |--|\n",p1,i);
    // }
    // Return no of bytes read.
    return bytes_read;
	
}

// Write handler for ppipe.
int ppipe_write (struct file *filep, char *buff, u32 count) {

    /**
     *  TODO:: Implementation of PPipe Write
     *
     *  Write the data from the provided buffer to the ppipe buffer.
     *  If count is greater than available space in the ppipe then just write
     *      data that fits in that space.
     *  Validate file object's access right.
     *  On successful write, return no of written bytes.
     *  Incase of Error return valid error code.
     *      -EACCES: In case access is not valid.
     *      -EINVAL: If write end is already closed.
     *      -EOTHERS: For any other errors.
     *
     */
    if(filep==NULL || (filep->ppipe == NULL))
        return -EOTHERS;
    if(filep->mode != O_WRITE){
        return -EACCES;
    }
    struct exec_context *cur=get_current_ctx();
    int cnt=(filep->ppipe)->ppipe_global.proc;
    int pi=cur->pid,ind,i;
    for(i=0;i<cnt;i++){
        if(((filep->ppipe)->ppipe_per_proc)[i].id==pi){
            ind=i;
        }
    }
    if((filep->ppipe)->ppipe_per_proc[ind].open_w!=1){
        return -EINVAL;
    }
    if(count==0) return 0;
    int bytes_written = 0;
    int p1=(filep->ppipe)->ppipe_global.r;
    int p2=(filep->ppipe)->ppipe_global.w;
    if((filep->ppipe)->ppipe_global.full){
        return bytes_written;
    }
    i=0;
    if((filep->ppipe)->ppipe_global.empty){
        ((filep->ppipe)->ppipe_global).ppipe_buff[p2]=buff[0];
        p2=(p2+1)%MAX_PPIPE_SIZE;
        i++;
        bytes_written++;
        (filep->ppipe)->ppipe_global.empty=0;
    }
    for(;i<count;i++){
        if(p2==p1)break;
        bytes_written++;
        ((filep->ppipe)->ppipe_global).ppipe_buff[p2]=buff[i];
        p2=(p2+1)%MAX_PPIPE_SIZE;
    }
    (filep->ppipe)->ppipe_global.w=p2;
    if(p1==p2){
        (filep->ppipe)->ppipe_global.full=1;
    }
    for(i=0;i<cnt;i++){
        p1=(filep->ppipe)->ppipe_per_proc[i].r;
        if(p1==p2){
           (filep->ppipe)->ppipe_per_proc[i].full=1;
        }
        (filep->ppipe)->ppipe_per_proc[i].empty=0;
    }
    // Return no of bytes written.
    return bytes_written;

}

// Function to create persistent pipe.
int create_persistent_pipe (struct exec_context *current, int *fd) {

    /**
     *  TODO:: Implementation of PPipe Create
     *
     *  Find two free file descriptors.
     *  Create two file objects for both ends by invoking the alloc_file() function.
     *  Create ppipe_info object by invoking the alloc_ppipe_info() function and
     *      fill per process and global info fields.
     *  Fill the fields for those file objects like type, fops, etc.
     *  Fill the valid file descriptor in *fd param.
     *  On success, return 0.
     *  Incase of Error return valid Error code.
     *      -ENOMEM: If memory is not enough.
     *      -EOTHERS: Some other errors.
     *
     */
    int fd_id,count=0;
    struct ppipe_info* cur_pipe = alloc_ppipe_info();
    if(cur_pipe==NULL)
        return -ENOMEM;
    int p1=current->pid;
    ((cur_pipe)->ppipe_per_proc)[0].id=p1;
    for (fd_id = 0; fd_id < MAX_OPEN_FILES; ++fd_id) {
        if(count>=2)break;
		if (current->files[fd_id] == NULL) {
            current->files[fd_id]=alloc_file();
            if(current->files[fd_id]==NULL)
                return -ENOMEM;
            (current->files[fd_id])->fops->read= ppipe_read;
            (current->files[fd_id])->fops->write= ppipe_write;
            (current->files[fd_id])->fops->close= ppipe_close;
            (current->files[fd_id])->ppipe=cur_pipe;
            if(count==0)(current->files[fd_id])->mode = O_READ;
            else (current->files[fd_id])->mode = O_WRITE;
            (current->files[fd_id])->type=PPIPE;
            fd[count]=fd_id;
            count++;
        }
    }
    if(count<2)return -EOTHERS;
    // Simple return.
    return 0;

}
