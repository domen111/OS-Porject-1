// systemcall #334


#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/timer.h>

asmlinkage long sys_my_time(void)
{
	struct timespec now;
	getnstimeofday(&now);
	printk(KERN_INFO "time: %ld, ", now.tv_sec * 1000000000l + now.tv_nsec);
	printk(KERN_INFO "tv_sec: %ld, tv_nsec: %ld\n", now.tv_sec, now.tv_nsec);
	return now.tv_sec * 1000000000l + now.tv_nsec;
}
