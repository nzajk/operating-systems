/*
 * seconds.c
 * 
 * Kernel module that communicates with /proc file system.
 *
 * Nicholas Zajkeskovic 
 * September 20, 2024
 * COMP SCI 3SH3
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/seq_file.h>

#define PROC_NAME "seconds"

// Function prototypes
int get_seconds(struct seq_file *m, void *v);
int proc_open(struct inode *inode, struct file *file);
int proc_init(void);
void proc_exit(void);

// Global variable to store the start time
unsigned long start_time;

// Calculates the elapsed time, print to /proc using seq_file.h
int get_seconds(struct seq_file *m, void *v) {
	unsigned long elapsed_time = (jiffies - start_time) / HZ;
	seq_printf(m, "%lu seconds have elapsed.\n", elapsed_time);
	return 0;
};

// File operations struct for the proc file
const struct proc_ops my_proc_ops = {
	.proc_open = proc_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

int proc_open(struct inode *inode, struct file *file) {
	return single_open(file, get_seconds, NULL);
};

// This function is called when the module is loaded
int proc_init(void) {
	start_time = jiffies;
	// Creates the /proc/seconds entry
	proc_create(PROC_NAME, 0, NULL, &my_proc_ops);

	printk(KERN_INFO "/proc/%s loaded\n", PROC_NAME);

	return 0;
};

// This function is called when the module is removed 
void proc_exit(void) {
	// Removes the /proc/seconds entry
	remove_proc_entry(PROC_NAME, NULL);

	printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
};

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Time Elapsed Module");
MODULE_AUTHOR("Nicholas Zajkeskovic");

/*
 * 'lab_release -a' output
 * Distributor ID:	Ubuntu
 * Description:	Ubuntu 24.04.1 LTS
 * Release:	24.04
 * Codename:	noble
 *
 * 'uname -r' output
 * 6.8.0-44-generic
 */
