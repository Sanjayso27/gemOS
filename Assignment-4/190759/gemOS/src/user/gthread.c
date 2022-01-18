#include <gthread.h>
#include <ulib.h>

static struct process_thread_info tinfo __attribute__((section(".user_data"))) = {};
/*XXX 
      Do not modifiy anything above this line. The global variable tinfo maintains user
      level accounting of threads. Refer gthread.h for definition of related structs.
 */

static void return_exit(){
	// struct exec_context *ctx = get_current_ctx();
	// void *retval=ctx->regs.rax;
	// printf("Yo return\n");
	void* retval;
	asm volatile("mov %%rax, %0;"
				: "=r" (retval)
				:: );
	// printf("Yo return\n");
	// printf("exited %d",getpid());
	gthread_exit(retval);
}

/* Returns 0 on success and -1 on failure */
/* Here you can use helper system call "make_thread_ready" for your implementation */
int gthread_create(int *tid, void *(*fc)(void *), void *arg) {
        
	/* You need to fill in your implementation here*/
	if(tinfo.num_threads>=MAX_THREADS || tid == NULL || fc == NULL)return -1;
	void *stackp;
	stackp = mmap(NULL, TH_STACK_SIZE, PROT_READ|PROT_WRITE, 0);
	if(!stackp || stackp == MAP_ERR){
        return -1;
  	}
	int thpid;
	thpid=clone(fc, ((u64)stackp)+TH_STACK_SIZE, arg);
	if(thpid <0){
		munmap(stackp, TH_STACK_SIZE);
		return -1;
	}
	make_thread_ready(thpid);
	int i;
	for(i=0; i<MAX_THREADS; ++i){
		// printf("i: %d ",tinfo.threads[i].status);
		if(tinfo.threads[i].status==TH_UNUSED){
			*tid=i;
			// printf("i: %d\n",i);
			tinfo.threads[i].tid=*tid;
			tinfo.threads[i].pid=thpid;
			tinfo.threads[i].status=TH_STATUS_USED;
			tinfo.threads[i].stack_addr=stackp;
			for(int j=0;j<MAX_GALLOC_AREAS;j++){
				tinfo.threads[i].priv_areas[j].owner=NULL;
			}
			tinfo.num_threads++;
			break;
		}
	}
	// struct exec_context *ctx=get_ctx_by_pid(thpid);
	// ctx->entry_rsp-=8;
	// *((u64*)ctx->regs.entry_rsp)=return_exit;
	// ctx->entry_rsp-=8;
	// *((u64*)ctx->regs.entry_rsp)=ctx->regs.rbp;
	// ctx->regs.rbp=ctx->regs.entry_rsp;
	u64 rsp=(u64)(((u64)stackp)+TH_STACK_SIZE);
	rsp-=8;
	*((u64*)rsp)=(u64)(&return_exit);
	return 0;
}

int gthread_exit(void *retval) {

	/* You need to fill in your implementation here*/
	
	u32 pid = getpid();
	// go through par and match the pid and delete info from both structures.
	int i;
	for(i=0; i<MAX_THREADS; ++i){
		if(tinfo.threads[i].pid == pid && tinfo.threads[i].status ==TH_STATUS_USED){
			tinfo.threads[i].ret_addr=retval;
			tinfo.threads[i].status=TH_EXITED_JOIN;
			for(int j=0;j<MAX_GALLOC_AREAS;j++){
				tinfo.threads[i].priv_areas[j].owner=NULL;
			}
		}
	}
	// printf("thread %d exited \n",pid);
	//call exit
	exit(0);
}

void* gthread_join(int tid) {
        
     /* Here you can use helper system call "wait_for_thread" for your implementation */
       
     /* You need to fill in your implementation here*/
	 if(tid<0)return NULL;
	u32 pid;
	int i;
	// printf("Yo join\n");
	for(i=0; i<MAX_THREADS; ++i){
		if(tinfo.threads[i].tid == tid){
			pid=tinfo.threads[i].pid;
			break;
		}
	}
	// printf("thread %d, join\n",tid);
	while(1){
		if(tinfo.threads[tid].status==TH_EXITED_JOIN || wait_for_thread(pid)<0)break;
	}
	void *ret;
	if(tinfo.threads[tid].status!=TH_EXITED_JOIN)ret=NULL;
	else ret=tinfo.threads[i].ret_addr;
	tinfo.threads[tid].status=TH_UNUSED;
	tinfo.num_threads--;
	if(munmap(tinfo.threads[tid].stack_addr, TH_STACK_SIZE)<0)return NULL;
	return ret;

	// return NULL;
}


/*Only threads will invoke this. No need to check if its a process
 * The allocation size is always < GALLOC_MAX and flags can be one
 * of the alloc flags (GALLOC_*) defined in gthread.h. Need to 
 * invoke mmap using the proper protection flags (for prot param to mmap)
 * and MAP_TH_PRIVATE as the flag param of mmap. The mmap call will be 
 * handled by handle_thread_private_map in the OS.
 * */

void* gmalloc(u32 size, u8 alloc_flag)
{
   
	/* You need to fill in your implementation here*/
	u32 prot=PROT_READ |PROT_WRITE ;
	if(alloc_flag == GALLOC_OWNONLY){
		prot =prot | TP_SIBLINGS_NOACCESS;
	}
	else if(alloc_flag == GALLOC_OTRDONLY){
		prot =prot | TP_SIBLINGS_RDONLY;
	}
	else if(alloc_flag == GALLOC_OTRDWR){
		prot =prot | TP_SIBLINGS_RDWR;
	}
	else return NULL;
	int i,chk;
	u32 pid;
	pid=getpid();
	for(i=0;i<MAX_THREADS;i++){
		if(tinfo.threads[i].pid==pid){
			chk=0;
			for(int j=0;j<MAX_GALLOC_AREAS;j++){
				if(tinfo.threads[i].priv_areas[j].owner==NULL)chk=1;
			}
		}
	}
	if(!chk)return NULL;
	void *ret=mmap(NULL, size, prot,MAP_TH_PRIVATE);
	if(ret==NULL || ret == MAP_ERR) return NULL;
	// printf("allocated \n");
	for(i=0; i<MAX_THREADS; ++i){
		if(tinfo.threads[i].pid==pid){
			chk=0;
			for(int j=0;j<MAX_GALLOC_AREAS;j++){
				if(tinfo.threads[i].priv_areas[j].owner==NULL){
					chk=1;
					tinfo.threads[i].priv_areas[j].owner=&tinfo.threads[i];
					tinfo.threads[i].priv_areas[j].start=(u64)ret;
					tinfo.threads[i].priv_areas[j].length=size;
					tinfo.threads[i].priv_areas[j].flags=prot;
					break;
				}
			}
			// printf("chk: %d\n",chk);
			// printf("pid: %d,chk: %d\n",pid,chk);
			if(chk==0)return NULL;
			break;
		}
	}
	// printf("updated\n");
	return ret;
}
/*
   Only threads will invoke this. No need to check if the caller is a process.
*/
int gfree(void *ptr)
{
   
    /* You need to fill in your implementation here*/
	u32 pid;
	int i;
	pid=getpid();
	for(i=0; i<MAX_THREADS; ++i){
		for(int j=0;j<MAX_GALLOC_AREAS;j++){
			if((tinfo.threads[i].priv_areas[j].owner!=NULL)
				&& (tinfo.threads[i].priv_areas[j].start==(u64)ptr) && (pid==tinfo.threads[i].pid)){
				munmap(ptr,tinfo.threads[i].priv_areas[j].length);
				tinfo.threads[i].priv_areas[j].owner=NULL;
				return 0;
			}
		}
	}
    return -1;
}
