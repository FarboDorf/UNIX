#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#define SUCCESS 0
#define DEVICE_NAME "chardev" 
#define BUF_LEN 512 
#define IOCTL_RESET 666
 
// Prototypes
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
 
 // global
static int major;  
static char msg[BUF_LEN]; 
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
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
    printk(KERN_INFO "Exit\n");
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
    int bytes_read = 0;
    size_t msg_len = strlen(msg);

    if (*offset >= msg_len)
        return 0;

    if (*offset + length > msg_len)
        length = msg_len - *offset;

    if (copy_to_user(buffer, msg + *offset, length))
        return -EFAULT;

    *offset += length;
    bytes_read = length;

    return bytes_read;
}
 
static ssize_t device_write(struct file *filp, 
                            const char __user *buffer,
                            size_t length, loff_t *off)
{
    int bytes_written = 0;
    size_t msg_len = strlen(msg);

    if (length >= BUF_LEN)
        return -ENOSPC;

    if (copy_from_user(msg, buffer, length))
        return -EFAULT;

    msg_len = length;
    bytes_written = length;

    return bytes_written;
}

static long device_ioctl(struct file *filp, 
                         unsigned int cmd, 
                         unsigned long arg)
{
    switch (cmd) {
        case IOCTL_RESET:
            memset(msg, 0, BUF_LEN); 
            break;
        default:
            return -ENOTTY; 
    }

    return SUCCESS;
}


 
module_init(chardev_init);
module_exit(chardev_exit);
 
MODULE_LICENSE("GPL");