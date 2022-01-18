#include <debug.h>
#include <context.h>
#include <entry.h>
#include <lib.h>
#include <memory.h>


/*****************************HELPERS******************************************/

/*
 * allocate the struct which contains information about debugger
 *
 */
struct debug_info *alloc_debug_info()
{
	struct debug_info *info = (struct debug_info *) os_alloc(sizeof(struct debug_info));
	if(info)
		bzero((char *)info, sizeof(struct debug_info));
	return info;
}
/*
 * frees a debug_info struct
 */
void free_debug_info(struct debug_info *ptr)
{
	if(ptr)
		os_free((void *)ptr, sizeof(struct debug_info));
}



/*
 * allocates a page to store registers structure
 */
struct registers *alloc_regs()
{
	struct registers *info = (struct registers*) os_alloc(sizeof(struct registers));
	if(info)
		bzero((char *)info, sizeof(struct registers));
	return info;
}

/*
 * frees an allocated registers struct
 */
void free_regs(struct registers *ptr)
{
	if(ptr)
		os_free((void *)ptr, sizeof(struct registers));
}

/*
 * allocate a node for breakpoint list
 * which contains information about breakpoint
 */
struct breakpoint_info *alloc_breakpoint_info()
{
	struct breakpoint_info *info = (struct breakpoint_info *)os_alloc(
		sizeof(struct breakpoint_info));
	if(info)
		bzero((char *)info, sizeof(struct breakpoint_info));
	return info;
}

/*
 * frees a node of breakpoint list
 */
void free_breakpoint_info(struct breakpoint_info *ptr)
{
	if(ptr)
		os_free((void *)ptr, sizeof(struct breakpoint_info));
}

/*
 * Fork handler.
 * The child context doesnt need the debug info
 * Set it to NULL
 * The child must go to sleep( ie move to WAIT state)
 * It will be made ready when the debugger calls wait
 */
void debugger_on_fork(struct exec_context *child_ctx)
{
	//printk("DEBUGGER FORK HANDLER CALLED\n");
	child_ctx->dbg = NULL;
	child_ctx->state = WAITING;
}


/******************************************************************************/


/* This is the int 0x3 handler
 * Hit from the childs context
 */

long int3_handler(struct exec_context *ctx)
{
	
	//Your code
	//printk("Yo2 \n");
	if(ctx==NULL)return -1;
	//printk("Yo\n");
	struct exec_context *par=get_ctx_by_pid(ctx->ppid);
	u64 pc=ctx->regs.entry_rip-1;
	struct breakpoint_info *cur=par->dbg->head;
	par->regs.rax=pc;
	int chk=1;
	int id=0;
	//  
	//printk("pc:%x\n",pc);
	// save the registers in debug field
	par->dbg->regs.entry_rip=ctx->regs.entry_rip;
	par->dbg->regs.entry_rsp=ctx->regs.entry_rsp;
	par->dbg->regs.rbp=ctx->regs.rbp;
	par->dbg->regs.rax=ctx->regs.rax;
	par->dbg->regs.rdi=ctx->regs.rdi;
	par->dbg->regs.rsi=ctx->regs.rsi;
	par->dbg->regs.rdx=ctx->regs.rdx;
	par->dbg->regs.rcx=ctx->regs.rcx;
	par->dbg->regs.r8=ctx->regs.r8;
	par->dbg->regs.r9=ctx->regs.r9;
	while(cur!=NULL){
		//printk("addr: %x\n",cur->addr);
		if(cur->addr==(u64)pc){
			chk=0;
			break;
		}
		cur=cur->next;
	}
	// save cur addr
	if(chk==0) {
		//printk("Yo\n")
		par->dbg->back[id]=pc;
		id++;
	}
	// save the back trace in debug field
	// // use backtracking via rbp
	u64 back=ctx->regs.entry_rsp,n=ctx->regs.rbp;
	while(*((u64*)back)!=END_ADDR){
		//printk("%x\n",*((u64*)back));
		par->dbg->back[id]=*((u64*)back);
		back=(n+8);
		n=*(u64*)n;
		id++;
	}
	// printk("Stack\n");
	// printk("%x\n",pc);
	// back=ctx->regs.entry_rsp;
	// while(*((u64*)back)!=END_ADDR){
	// 	printk("%x\n",*((u64*)back));
	// 	back=(back+8);
	// }
	// printk("Stack ends\n");
	par->dbg->back_cnt=id;
	if(chk==0 ){
		// push par->dbg->end_handler into stack
		// push dummy rbp 
		//ctx->regs.rbp=ctx->regs.entry_rsp;
		// ctx->regs.entry_rsp-=8;
		// *((u64 *) ctx->regs.entry_rsp)=ctx->regs.rbp;
		if(cur->end_breakpoint_enable==1){
			u64 temp=ctx->regs.entry_rsp;
			ctx->regs.entry_rsp-=8;
			*((u64 *) ctx->regs.entry_rsp)=(u64)par->dbg->end_handler;
			//printk("Yo 1\n ,addr: %x\n",(u64)(par->dbg->end_handler));
			// ctx->regs.entry_rsp-=8;
			// *((u64 *) ctx->regs.entry_rsp)=temp;
			//ctx->regs.rbp=ctx->regs.entry_rsp;
		}
		//printk("base: %x\n",ctx->regs.entry_rsp);
	}
	else {
		//printk("base: %x\n",*(u64 *)ctx->regs.rbp);
	}
	// printk("Stack\n");
	// printk("%x\n",pc);
	// back=ctx->regs.entry_rsp;
	// while(*((u64*)back)!=END_ADDR){
	// 	printk("%x\n",*((u64*)back));
	// 	back=(back+8);
	// }
	// printk("Stack ends\n");
	//
	//
	ctx->state=WAITING;
	par->state=READY;
	schedule(par);
	return -1;
}

/*
 * Exit handler.
 * Deallocate the debug_info struct if its a debugger.
 * Wake up the debugger if its a child
 */
void debugger_on_exit(struct exec_context *ctx)
{
	// Your code
	if(ctx->dbg!=NULL){
		struct breakpoint_info *temp=ctx->dbg->head,*next;
		while(temp!=NULL){
			next=temp->next;
			free_breakpoint_info(temp);
			temp=next;
		}
		free_debug_info(ctx->dbg);
	}
	else {
		struct exec_context *par=get_ctx_by_pid(ctx->ppid);
		par->regs.rax=CHILD_EXIT;
		par->state=READY;
	}
}


/*
 * called from debuggers context
 * initializes debugger state
 */
int do_become_debugger(struct exec_context *ctx, void *addr)
{
	// Your code
	if(ctx==NULL)return -1;
	ctx->dbg=alloc_debug_info();
	if(ctx->dbg==NULL) return -1;
	*((u8* )addr)=INT3_OPCODE;
	ctx->dbg->end_handler=addr;
	ctx->dbg->breakpoint_count=0;
	ctx->dbg->back_cnt=0;
	ctx->dbg->id=0;
	ctx->dbg->head=NULL;
	return 0;
}

/*
 * called from debuggers context
 */
int do_set_breakpoint(struct exec_context *ctx, void *addr, int flag)
{

	// Your code
	if(ctx==NULL || ctx->dbg==NULL ||ctx->dbg->breakpoint_count==MAX_BREAKPOINTS)
		return -1;
	if(ctx->dbg->head==NULL){
		ctx->dbg->head=alloc_breakpoint_info();
		if(ctx->dbg->head==NULL) return -1;
		ctx->dbg->head->num=(++ctx->dbg->id);
		ctx->dbg->breakpoint_count++;
		ctx->dbg->head->addr=(u64)addr;
		ctx->dbg->head->end_breakpoint_enable=flag;
		ctx->dbg->head->next=NULL;
	}
	else {
		struct breakpoint_info *cur=ctx->dbg->head,*prev;
		int chk=1;
		while(cur!=NULL){
			if(cur->addr==(u64)addr){
				chk=0;
				cur->end_breakpoint_enable=flag;
				break;
			}
			prev=cur,cur=cur->next;
		}
		if(chk){
			prev->next=alloc_breakpoint_info();
			if(prev->next==NULL) return -1;
			prev=prev->next;
			prev->num=(++ctx->dbg->id);
			ctx->dbg->breakpoint_count++;
			prev->addr=(u64)addr;
			prev->end_breakpoint_enable=flag;
			prev->next=NULL;
		}
	}
	*(u8*)addr =INT3_OPCODE;
	return 0;
	// printk("\nHi,%u breakpoint\n",ctx->dbg->id);
	// printk("\n%x\n",*((u64*)addr));
}

/*
 * called from debuggers context
 */
int do_remove_breakpoint(struct exec_context *ctx, void *addr)
{
	//Your code
	if(ctx==NULL || ctx->dbg==NULL)return -1;
	struct breakpoint_info *cur=ctx->dbg->head,*prev;
	int chk=1;
	while(cur!=NULL){
		if(cur->addr==(u64)addr){
			// go through backtrace and check
			int ok=1;
			if(cur->end_breakpoint_enable==1){
				for(int i=0;i<ctx->dbg->back_cnt;i++){
					if(ctx->dbg->back[i]==(u64)addr)ok=0;
				}
				if(ok==0)return -1;
			}
			*(u8* )addr=PUSHRBP_OPCODE;
			if(cur==ctx->dbg->head){
				ctx->dbg->head=cur->next;
			}
			else {
				prev->next=cur->next;
			}
			free_breakpoint_info(cur);
			chk=0;
			ctx->dbg->breakpoint_count--;
			break;
		}
		prev=cur;cur=cur->next;
	}
	if(chk)return -1;
	return 0;
}


/*
 * called from debuggers context
 */

int do_info_breakpoints(struct exec_context *ctx, struct breakpoint *ubp)
{
	
	// Your code
	if(ctx==NULL)return -1;
	struct breakpoint_info *cur=ctx->dbg->head;
	if(cur==NULL)return -1;
	struct breakpoint temp;
	int id=0;
	while(cur!=NULL){
		temp.num=cur->num;
		temp.end_breakpoint_enable=cur->end_breakpoint_enable;
		temp.addr=cur->addr;
		ubp[id]=temp;
		cur=cur->next;
		id++;
	}
	return id;
}


/*
 * called from debuggers context
 */
int do_info_registers(struct exec_context *ctx, struct registers *regs)
{
	// Your code
	if(ctx==NULL || ctx->dbg==NULL )
		return -1;
	regs->entry_rip=ctx->dbg->regs.entry_rip;
	regs->entry_rsp=ctx->dbg->regs.entry_rsp;
	regs->rbp=ctx->dbg->regs.rbp;
	regs->rax=ctx->dbg->regs.rax;
	regs->rdi=ctx->dbg->regs.rdi;
	regs->rsi=ctx->dbg->regs.rsi;
	regs->rdx=ctx->dbg->regs.rdx;
	regs->rcx=ctx->dbg->regs.rcx;
	regs->r8=ctx->dbg->regs.r8;
	regs->r9=ctx->dbg->regs.r9;
	return 0;
}

/*
 * Called from debuggers context
 */
int do_backtrace(struct exec_context *ctx, u64 bt_buf)
{

	// Your code

	if(ctx==NULL ||ctx->dbg==NULL)
		return -1;
	struct debug_info * cur=ctx->dbg;
	u64 back_cnt=ctx->dbg->back_cnt;
	for(int i=0;i<back_cnt;i++){
		((u64*)bt_buf)[i]=cur->back[i];
	}
	return back_cnt;
}

/*
 * When the debugger calls wait
 * it must move to WAITING state
 * and its child must move to READY state
 */

s64 do_wait_and_continue(struct exec_context *ctx)
{
	// Your code
	if(ctx->dbg==NULL)return -1;
	struct exec_context * child;
	int chk=0;
	for(int i=0;i<=MAX_PROCESSES;i++){
		child=get_ctx_by_pid(i);
		if(ctx->pid==child->ppid){
			chk=1;
			break;
		}
	}
	if(!chk)return CHILD_EXIT;
	child->state=READY;
	ctx->state=WAITING;
	schedule(child);
	return -1;
}






