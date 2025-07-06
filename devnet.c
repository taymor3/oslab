/* devnet.c: Example char device module.
 *
 */
/* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/kernel.h>  	
#include <linux/module.h>
#include <linux/fs.h>       		
#include <asm/uaccess.h>
#include <linux/errno.h>  
#include <asm/segment.h>
#include <asm/current.h>
#include <linux/slab.h> // for kmalloc and kfree
#include <stdio.h>

#include "devnet.h"

#define MY_DEVICE "devnet"
#define MAX_CONNECTIONS 256 // MINOR is [0,255]


MODULE_AUTHOR("Anonymous");
MODULE_LICENSE("GPL");

/* globals */
int my_major = 0; /* will hold the major # of my device driver */
file_data_t **dev_files; // Pointer to file data structure

typedef struct{
    unsigned int id;//number of file [0,255] , this is an index for the driver
    int fd; // file descriptor - for kernel space
    //here we can save a pointer to a queue of messages written into the file...
    char ** messages; // Array of messages for each connection
    int *ports;
} file_data_t;


struct file_operations my_fops = {
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .ioctl = my_ioctl,
};

int init_module(void)
{
    // This function is called when inserting the module using insmod

    my_major = register_chrdev(my_major, MY_DEVICE, &my_fops);

    if (my_major < 0)
    {
	printk(KERN_WARNING "can't get dynamic major\n");
	return my_major;
    }

    //
    // do_init();
    //

    dev_files = kmalloc(MAX_CONNECTIONS * sizeof(*file_data_t), GFP_KERNEL);
    if (!dev_files) {
        printk(KERN_ERR "Failed to allocate memory for file data structures\n");
        kfree(ports); // Free previously allocated ports
        unregister_chrdev(my_major, MY_DEVICE);
        return -ENOMEM; // Memory allocation failed
    }

    // initialize all slots to NULL:
    for (i = 0; i < MAX_CONNECTIONS; i++)
    dev_files[i] = NULL;


    return 0;
}


void cleanup_module(void)
{
    // This function is called when removing the module using rmmod

    unregister_chrdev(my_major, MY_DEVICE);

    //
    // do clean_up();
    //
    free(dev_files); // Free the allocated memory for file data structures
    return;
}


int my_open(struct inode *inode, struct file *filp)
{
    // handle open
    if(!inode) {
        printk(KERN_ERR "Invalid inode pointer\n");
        return -EINVAL; // Invalid argument
    }

    unsigned int minor_num = MINOR(inode->i_rdev);
    if (minor_num >= MAX_CONNECTIONS) {
        printk(KERN_ERR "Invalid minor number: %u\n", minor_num);
        return -EINVAL; // Invalid argument
    }

    if(dev_files[minor_num]) {
        printk(KERN_ERR "File already open for minor number: %u\n", minor_num);
        return -EBUSY; // Device or resource busy
    }
    // Initialize file data structure for this minor number
    dev_files[minor_num] = kmalloc(sizeof(file_data_t), GFP_KERNEL);
    if (!dev_files[minor_num]) {
        printk(KERN_ERR "Failed to allocate memory for file data structure\n");
        return -ENOMEM; // Memory allocation failed
    }

    dev_files[minor_num]->id = minor_num;
    dev_files[minor_num]->ports = kmalloc(QUEUE_SIZE * sizeof(int), GFP_KERNEL);
    if (!dev_files[minor_num]->ports) {
        printk(KERN_ERR "Failed to allocate memory for ports\n");
        kfree(dev_files[minor_num]); // Free previously allocated file data structure
        dev_files[minor_num] = NULL; // Set to NULL to indicate it's not in use
        return -ENOMEM; // Memory allocation failed
    }
    dev_files[minor_num]->fd = filp->f_flags; // Store file descriptor flags
    

    return 0;
}


int my_release(struct inode *inode, struct file *filp)
{
    // handle file closing

    return 0;
}

ssize_t my_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    //
    // Do read operation.
    // Return number of bytes read.
    return 0; 
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
    case LISTEN:
	//
	// handle 
	//
	break;
    case CONNECT:
	//
	// handle 
	//
	break;
    default:
	return -ENOTTY;
    }

    return 0;
}
