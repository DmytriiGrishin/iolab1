#include<linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


static dev_t first;
static struct cdev c_dev; 
static struct class *cl;

struct file* dump_file = NULL;

static int my_open(struct inode *i, struct file *f) {
	printk(KERN_INFO "Driver: open()\n");
	return 0;
}
static int my_close(struct inode *i, struct file *f) {
	printk(KERN_INFO "Driver: close()\n");
	return 0;
}
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	printk(KERN_INFO "Driver: read()\n");
	return 0;
}

static bool is_command_open(const char __user *buf,  size_t len) {
	if (len >4) {
		if (buf[0] == 'o' 
			&& buf[1] == 'p'
			&& buf[2] == 'e'
			&& buf[3] == 'n')
			return true;
	}
	return false;
}

static bool is_command_close(const char __user *buf,  size_t len) {
	if (len >= 5) {
		if (buf[0] == 'c' 
			&& buf[1] == 'l'
			&& buf[2] == 'o'
			&& buf[3] == 's'
			&& buf[4] == 'e')
			return true;
	}
	return false;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off) {
	printk(KERN_INFO "Driver: write(%s) with length %ld\n", buf, len);
	if (dump_file == NULL) {
		if (is_command_open(buf, len)) {
			char* dump_name = buf + 5;
			dump_file = filp_open(dump_name, O_CREAT, 0644);
		} else {
			printk(KERN_INFO "Error: No open file");
		}
	} else {
		if (is_command_close(buf, len)) {
			filp_close(dump_file, NULL);
			dump_file = NULL;
		} else {
			mm_segment_t fs;
			fs = get_fs();
        	set_fs(get_ds());
			size_t write = dump_file->f_op->write(f, buf, len, &f->f_pos);
			while (write < len && write >= 0) {
				write += dump_file->f_op->write(f, buf + write, len - write, &f->f_pos);
			}
			set_fs(fs);
			printk(KERN_INFO "%ld", write);
		}
	}
	return len;
}

static struct file_operations mychdev_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static int __init ch_drv_init(void) {
	printk(KERN_INFO "Hello!\n");
	if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0) {
		printk(KERN_INFO "ch_dev not allocated\n");
		return -1;
	}
	printk(KERN_INFO "ch_dev allocated\n");
	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
		printk(KERN_INFO "chardrv not created\n");
		unregister_chrdev_region(first, 1);
		return -1;
	}
	printk(KERN_INFO "chardrv created\n");
	if (device_create(cl, NULL, first, NULL, "var1") == NULL) {
		printk(KERN_INFO "mychdev not created\n");
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	printk(KERN_INFO "mychdev created\n");

	cdev_init(&c_dev, &mychdev_fops);
	if (cdev_add(&c_dev, first, 1) == -1) {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
return 0;
}
static void __exit ch_drv_exit(void) {
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "Bye!!!\n");
}
module_init(ch_drv_init);
module_exit(ch_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("The first kernel module");