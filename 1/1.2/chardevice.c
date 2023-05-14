#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#define SUCCESS 0
#define DEVICE_NAME "chardev" 
#define BUF_LEN 100 
 
// Prototypes
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);
 
 // global
static int major;  
static char msg[BUF_LEN]; 
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};
 
static int __init chardev_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
 
    if (major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major;
    }
 
    printk(KERN_INFO "I was assigned major number %d.\n", major); 
    printk(KERN_INFO "Device created on /dev/%s\n", DEVICE_NAME);
 
    return SUCCESS;
}
 
static void __exit chardev_exit(void)
{ 
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Exit\n" 
}
 
 // Methodss 

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device opened by process: %s (pid=%d)\n", current->comm, current->pid);
    return SUCCESS;
}
 
static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device released\n");
    return SUCCESS;
}
 

static ssize_t device_read(struct file *filp, 
                           char __user *buffer,
                           size_t length,
                           loff_t *offset)
{
    printk(KERN_INFO "Device read\n");
    return SUCCESS;

}
 
static ssize_t device_write(struct file *filp, const char __user *buff,
                            size_t len, loff_t *off)
{
    printk(KERN_INFO "Device write\n");
    return len;
}
 
module_init(chardev_init);
module_exit(chardev_exit);
 
MODULE_LICENSE("GPL");