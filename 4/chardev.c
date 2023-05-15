#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/random.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "chardev"
#define BUF_LEN 100

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static char buffer[BUF_LEN];
static int buffer_index = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);

struct task_struct *ts;
int thread(void *data) {
    int i, lessthan100;
    // while (1) {
        get_random_bytes(&i, sizeof(i));
        lessthan100 = i % 100;
        sprintf(buffer, "%d\n", lessthan100);
        printk("Hello from randomizer: %d\n", lessthan100);
        msleep(2000);
        wake_up_interruptible(&wq);
        // }
    return 0;
}

static ssize_t chardev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int bytes_read = 0;
    int i;

    wait_event_interruptible(wq, buffer_index > 0);

    for (i = 0; i < buffer_index && i < count; i++) {
        if (put_user(buffer[i], &buf[i]) != 0)
            return -EFAULT;
        bytes_read++;
    }

    buffer_index = 0;

    return bytes_read;
}

static int chardev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "symboldev: Device opened\n");
    return 0;
}

static int chardev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "symboldev: Device closed\n");
    return 0;
}

struct file_operations fops = {
    .read = chardev_read,
    .open = chardev_open,
    .release = chardev_release,
};

static int __init chardev_init(void)
{
    int ret;

    if ((ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) < 0) {
        printk(KERN_ERR "reserving the major and minor numbers of the device Failed\n");
        return ret;
    }

    cdev_init(&c_dev, &fops);

    if ((ret = cdev_add(&c_dev, dev, 1)) < 0) {
        printk(KERN_ERR "adding character device Failed\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    cl = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(cl)) {
        printk(KERN_ERR "Failed to create device class\n");
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(cl);
    }

    if (device_create(cl, NULL, dev, NULL, DEVICE_NAME) == NULL) {
        printk(KERN_ERR "Failed to create device\n");
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    printk(KERN_INFO "Character device driver loaded\n");
    ts = kthread_run(thread,NULL,"foo kthread");
    return 0;
}

static void __exit chardev_exit(void)
{
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, 1);
    kthread_stop(ts);
    printk(KERN_INFO "Character device driver unloaded\n");
}

module_init(chardev_init);
module_exit(chardev_exit);