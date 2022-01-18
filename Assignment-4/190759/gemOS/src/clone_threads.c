#include<clone_threads.h>
#include<entry.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<mmap.h>

/*
  system call handler for clone, create thread like 
  execution contexts. Returns pid of the new context to the caller. 
  The new context starts execution from the 'th_func' and 
  use 'user_stack' for its stack
*/
// void return_exit(){

// }
long do_clone(void *th_func, void *user_stack, void *user_arg) 
{
  


  struct exec_context *new_ctx = get_new_ctx();
  struct exec_context *ctx = get_current_ctx();

  u32 pid = new_ctx->pid;
  
  if(!ctx->ctx_threads){  // This is the first thread
          ctx->ctx_threads = os_alloc(sizeof(struct ctx_thread_info));
          bzero((char *)ctx->ctx_threads, sizeof(struct ctx_thread_info));
          ctx->ctx_threads->pid = ctx->pid;
  }
     
 /* XXX Do not change anything above. Your implementation goes here*/
  
  
  // allocate page for os stack in kernel part of process's VAS
  // The following two lines should be there. The order can be 
  // decided depending on your logic.
  struct thread *th;
  struct ctx_thread_info *tinfo = ctx->ctx_threads;
   int ok=0,i;
   for(i=0; i<MAX_THREADS; ++i){
        th = &tinfo->threads[i];
        if(th->status == TH_UNUSED){
          tinfo->threads[i].status=TH_USED;
          tinfo->threads[i].pid=pid;
          tinfo->threads[i].parent_ctx=ctx;
          ok=1;
          break;
        }
   }
   if(!ok){
     new_ctx->state = UNUSED;
     return -1;
    }
   setup_child_context(new_ctx);
   new_ctx->type = EXEC_CTX_USER_TH;    // Make sure the context type is thread
   new_ctx->ppid= ctx->pid;
   new_ctx->used_mem = ctx->used_mem;
   new_ctx->pgd = ctx->pgd;
   for (i=0;i< MAX_MM_SEGS;i++){
     new_ctx->mms[i] = ctx->mms[i];
   }
   new_ctx->vm_area = ctx -> vm_area;
   for (i=0;i<CNAME_MAX;i++){
     new_ctx->name[i]=ctx->name[i];
   }
   new_ctx->pending_signal_bitmap = ctx->pending_signal_bitmap;
   for (i=0;i<MAX_SIGNALS;i++){
     new_ctx->sighandlers[i] = ctx ->sighandlers[i];
   }
   new_ctx->ticks_to_sleep = ctx->ticks_to_sleep;
   new_ctx->alarm_config_time = ctx->alarm_config_time;
   for (i=0;i<MAX_OPEN_FILES;i++){
     new_ctx->files[i]=ctx->files[i];
   }
   new_ctx->regs=ctx->regs;
   new_ctx->regs.rdi=(u64)user_arg;
   new_ctx->regs.entry_rsp=((u64) user_stack)-8;
   new_ctx->regs.rbp=(u64) user_stack;
   new_ctx->regs.entry_rip =(u64)th_func;
   //printk("func: %x\n",(u64)th_func);
   // add things in ctx_thread_info *ctx_threads
   // handle case when thread_exit is not called from thread in that case return should be handled
  //   new_ctx->regs.entry_rsp-=8;
  //  *((u64*)new_ctx->regs.rsp)=return_exit;
   
   new_ctx->state = WAITING;            // For the time being. Remove it as per your need.
   return pid;

}

/*This is the page fault handler for thread private memory area (allocated using 
 * gmalloc from user space). This should fix the fault as per the rules. If the the 
 * access is legal, the fault handler should fix it and return 1. Otherwise it should
 * invoke segfault_exit and return -1*/

int handle_thread_private_fault(struct exec_context *current, u64 addr, int error_code)
{
  
  /* your implementation goes here*/
  int i,ctr;
  struct exec_context* par;
  if(current->type==EXEC_CTX_USER_TH)
    par=get_ctx_by_pid(current->ppid);
  else 
    par=current;
  // printk("pid: %d,error_code: %d \n",current->pid,error_code);
  u32 owner_pid;
  for(i=0;i<MAX_THREADS;i++){
    struct thread_private_map *thmap = &((par->ctx_threads->threads[i]).private_mappings[0]);
    for(ctr = 0; ctr < MAX_PRIVATE_AREAS; ++ctr, thmap++){
      if(!thmap->owner)continue;
      if(addr >=thmap->start_addr && addr<(thmap->start_addr+thmap->length)){
        owner_pid=par->ctx_threads->threads[i].pid;
        u32 flags=thmap->flags;
        if(current->type==EXEC_CTX_USER_TH && (current->pid!=owner_pid) && 
          ((flags&(0x10)) || ((flags&(0x20)) && (error_code&(0x2)))) )
          {
            segfault_exit(current->pid, current->regs.entry_rip, addr);
            return -1;
          }
        else {
          if(!(error_code & 0x1)){
            u64 * base=(u64 *)osmap(current->pgd),*value;
            u64 mask[]={PGD_MASK,PUD_MASK,PMD_MASK};
            u64 shift[]={PGD_SHIFT,PUD_SHIFT,PMD_SHIFT};
            u64 pfn;
            for(i=0;i<3;i++){
              value=base + ((mask[i] & addr) >> shift[i]);
              if(!(*value & 0x1)){
                pfn=os_pfn_alloc(OS_PT_REG);
                *value= (pfn <<PTE_SHIFT);
                *value = ((*value >> 3)<<3) | 0x7;
              }
              else {
                *value = ((*value >> 3)<<3)| 0x7;
                // pfn= (*value >> PTE_SHIFT);
                pfn= (*value >> PTE_SHIFT)& 0xFFFFFFFF;
              }
              base = (u64 *)osmap(pfn);
            }
            value = base + ((PTE_MASK & addr) >> PTE_SHIFT);
            pfn = os_pfn_alloc(USER_REG);
            *value = (pfn <<PTE_SHIFT);
            if((current->type==EXEC_CTX_USER) || (current->pid==owner_pid)
              || (flags & (0x40))) *value = ((*value >> 3)<<3) | 0x7;
            else if(flags & (0x20)) *value = ((*value >> 3)<<3) | 0x5;
          }
        }
        return 1;
      }
    }
  }
  return 1;
}

/*This is a handler called from scheduler. The 'current' refers to the outgoing context and the 'next' 
 * is the incoming context. Both of them can be either the parent process or one of the threads, but only
 * one of them can be the process (as we are having a system with a single user process). This handler
 * should apply the mapping rules passed in the gmalloc calls. */

int handle_private_ctxswitch(struct exec_context *current, struct exec_context *next)
{
  
  /* your implementation goes here*/
  // update the last three bits based on os meta data by page table walk;
  int i,ctr;
  // printk("Yo\n");
  if(current==NULL || next==NULL)return -1;
  struct exec_context* par;
  if(current->type==EXEC_CTX_USER_TH)
    par=get_ctx_by_pid(current->ppid);
  else 
    par=current;
  // printk("par pid: %d\n",par->pid);
  u64 addr;
  for(i=0;i<MAX_THREADS;i++){
    // printk("i: %d,status: %d\n",i,par->ctx_threads->threads[i].status);
    if(par->ctx_threads->threads[i].status==TH_UNUSED)continue;
    struct thread_private_map *thmap = &(par->ctx_threads->threads[i]).private_mappings[0];
    u32 owner_pid=par->ctx_threads->threads[i].pid,flags;
    // printk("owner: %d\n",owner_pid);
    for(ctr = 0; ctr < MAX_PRIVATE_AREAS; ++ctr, thmap++){
      if(!thmap->owner)continue;
      if(isProcess(next)|| owner_pid == next->pid){
        flags=0x7;
      }
      else {
        if(thmap->flags & (0x10))flags=0x4;
        else if((thmap->flags & (0x20)))flags=0x5;
        else if((thmap->flags & (0x40)))flags=0x7;
      }
      u64 * base=(u64 *)osmap(current->pgd),*value;
      u64 mask[]={PGD_MASK,PUD_MASK,PMD_MASK,PTE_MASK};
      u64 shift[]={PGD_SHIFT,PUD_SHIFT,PMD_SHIFT,PTE_SHIFT};
      u64 pfn;
      // printk("owner_pid: %d,start addr: %x,length: %d,flags: %d\n",owner_pid,thmap->start_addr,thmap->length,flags);
      for(addr=thmap->start_addr; addr <thmap->start_addr+thmap->length;addr++){
        // printk("thread :%d , ctr: %d ,addr: %x\n",i,ctr,addr);
        asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
        for(i=0;i<4;i++){
          value=base + ((mask[i] & addr) >> shift[i]);
          if(!(*value & 0x1)){
            break;
          }
          if(i==3){*value = ((*value >> 3)<<3)| flags;}
          // printk("addr: %x,pid: %d",addr,next->pid);}
          else *value = ((*value >> 3)<<3) | 0x7;
          if(i<3){
            pfn= (*value >> PTE_SHIFT) & 0xFFFFFFFF;
            // pfn= (*value >> PTE_SHIFT);
            base = (u64 *)osmap(pfn);
          }
        }
        // printk("i: %d\n",i);
      }
    }
  }
  // printk("Yo\n");
  // printk("pid: %d,pc: %x\n",next->pid,next->regs.entry_rip);
  printk("");
  return 0;	

}
