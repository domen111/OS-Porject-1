// syscall #337


#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage void sys_my_print(int pid, long start, long end)
{
	printk(KERN_INFO "[Project1] %d %ld.%ld %ld.%ld", pid, start / 1000000000l, start % 1000000000l, end / 1000000000l, end % 1000000000l);
}

