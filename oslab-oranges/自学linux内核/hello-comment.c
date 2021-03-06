#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/unistd.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include<linux/string.h>
#include<linux/fs.h>
#include<linux/fdtable.h>
#include<linux/uaccess.h>

#include<linux/rtc.h>

MODULE_LICENSE("Dual BSD/GPL");
 
#define _DEBUG
#ifdef _DEBUG
#define kprintk(fmt,args...) printk(KERN_ALERT fmt,##args)
#define kprintf(fmt,args...) printf(fmt,##args)
#define kperror(str) perror(str)
#else
#define kprintk
#define kprintf
#define kperror
#endif

/*Function declaration*/
long * get_sys_call_table(void);
unsigned int close_cr(void);
void open_cr(unsigned int oldval);
void start_hook(void);
asmlinkage long (*orig_open)(char __user *filename, int flags, int mode);

long * g_sys_call_table = NULL; //save address of sys_call_table
long g_old_sys_open = 0; //save old address of sys_open
long g_oldcr0 = 0; //save address of cr0


struct _idtr{  //中断描述符表寄存器
    unsigned short limit;
    unsigned int base;
}__attribute__((packed));

struct _idt_descriptor{
    unsigned short offset_low;
    unsigned short sel;
    unsigned char none,flags;
    unsigned short offset_high;
}__attribute__((packed));

unsigned int close_cr(void){
    unsigned int cr0 = 0;
    unsigned int ret;
    asm volatile("movl %%cr0,%%eax":"=a"(cr0));
    ret = cr0;
    cr0 &= 0xfffeffff;
    asm volatile("movl %%eax,%%cr0"::"a"(cr0));
    return ret;
}

void open_cr(unsigned int oldval){
    asm volatile("movl %%eax,%%cr0"::"a"(oldval));
}

/*Get the address of sys_call_table*/
long * get_sys_call_table(void){ //在idtr寄存器
    

    struct _idt_descriptor * idt;
    struct _idtr idtr;
    unsigned int sys_call_off;
    int sys_call_table=0;
    unsigned char* p;
    int i;
    asm("sidt %0":"=m"(idtr));  //汇编，sidt指令获得中断描述符表基地址
    kprintk("   address of idtr: 0x%x\n",(unsigned int)&idtr);
    idt=(struct _idt_descriptor *)(idtr.base+8*0x80);  //0x80中断为系统调用中断 这是一个描述符，下面的操作得到描述符指向的具体地址
    sys_call_off=((unsigned int)(idt->offset_high<<16)|(unsigned int)idt->offset_low);
    kprintk("   address of idt 0x80: 0x%x\n",sys_call_off);   //0x80位
    p=(unsigned char *)sys_call_off;
    for(i=0;i<100;i++){
        if(p[i]==0xff&&p[i+1]==0x14&&p[i+2]==0x85){
            sys_call_table=*(int*)((int)p+i+3);
            kprintk("   address of sys_call_table: 0x%x\n",sys_call_table);

            return (long*)sys_call_table;
        }
    }
    
    return 0;
}

//My own sys_open
asmlinkage long my_sys_open(char * filename, int flags, int mode){ //打印当前使用sys-open函数的进程信息和文件名
    kprintk("The process is \"%s\"(pid is %i)\n",current->comm,current->pid);
    kprintk("The file is being accessed is \"%s\"\n",filename);
    return orig_open(filename,flags,mode);
}

void start_hook(void){ //得到系统调用表地址，寻找sys-open项，替换为my_sys-open,cr0寄存器16位置0，可以写只读项
    g_sys_call_table = get_sys_call_table();
    if(!g_sys_call_table){
        kprintk("Get sys_call_table error!\n");
        return;
    }
    if(g_sys_call_table[__NR_close] != (unsigned long)sys_close){
        kprintk("Incorrect sys_call_table address!\n");
        return;
    }
    
    g_old_sys_open = g_sys_call_table[__NR_open];
    orig_open = (long(*)(char *, int, int))g_sys_call_table[__NR_open];
    
    g_oldcr0=close_cr();
    g_sys_call_table[__NR_open] = my_sys_open;
    open_cr(g_oldcr0);
}

int monitor_init(void){   //启动模块
    kprintk("Monitor init\n");
    start_hook();
    return 0;
}

void monitor_exit(void){ //退出模块 恢复系统调用表sys_open函数所在项地址为原sys_open，cr0寄存器16位置1，禁止写只读文件
    if(g_sys_call_table && g_old_sys_open){
        g_oldcr0 = close_cr();
        g_sys_call_table[__NR_open] = g_old_sys_open;
        open_cr(g_oldcr0);
    }
    kprintk("Monitor exit\n");
}

module_init(monitor_init);
module_exit(monitor_exit);