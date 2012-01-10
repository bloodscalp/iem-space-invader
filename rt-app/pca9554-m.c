/*******************************************************************
 * pca9554.c - HEIG-VD 2008, Cours IEM
 *
 * Author: DRE
 * Date: December 2008
 *******************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#include "pca9554-m.h"
#include "xeno-i2c.h"

#define I2C_SLAVE 0x0703 	/* IOCTL CMD value to be passed to xeno_i2c_ioctl */

#define I2C_IO_MODES 0xF0

#define BUFF_SIZE 2

struct cdev *i2c_dev;

typedef enum i2c_commands_t {
	INPUT,
	OUTPUT,
	POLARITY,
	CONFIG
} i2c_commands_t;

struct file_operations fops = {
	//.close 	= pca9554_close,
	//.open 	= pca9554_open,
	.read 	= pca9554_read,
	.write	= pca9554_write,
	.ioctl 	= pca9554_ioctl
};


int pca9554_open(struct inode *inode, struct file *file) {

	return 0;
}

int pca9554_ioctl(struct inode * inode, struct file *file, unsigned int cmd, unsigned long arg) {
	int err;
	if( (err = xeno_i2c_ioctl(cmd, arg)) < 0) {
		printk("I2C ioctl error : %d", err);
		return err;
	}

	return 0;
}

int pca9554_close(struct inode *inode, struct file *file) {


	return 0;
}

ssize_t pca9554_read(struct file *file, char __user *buff, size_t len, loff_t *off) {
	int err;
	char kbuf[len];

	kbuf[0] = INPUT;

	if( (err = xeno_i2c_write(kbuf, 1)) < 0) {
		printk("I2C read config error : %d", err);
		return err;
	}

	if ( (err = xeno_i2c_read(kbuf, len)) < 0) {
		printk("I2C read error : %d", err);
		return err;
	} else {
		if( (err = copy_to_user(buff, kbuf, len)) > 0 ) {
			printk("copy to user error : %d B not copied", err);
			return -1;
		}
	}

	return 0;
}

ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *off) {
	int err;
	char kbuf[len];

	kbuf[0] = OUTPUT;

	if( (err = xeno_i2c_write(kbuf, 1)) < 0) {
		printk("I2C write config error : %d", err);
		return err;
	}

	if( (err = copy_from_user(kbuf, buff, len)) > 0 ) {
		printk("copy from user error : %d B not copied", err);
		return -1;
	}

	if( (err = xeno_i2c_write(kbuf, len)) < 0) {
		printk("I2C write error : %d", err);
		return err;
	}

	return 0;
}

/**
 *  major 89  I2C bus interface
 *		  0 = /dev/i2c-0	First I2C adapter
 *		  1 = /dev/i2c-1	Second I2C adapter
 *
 */
int __init init_module(void) {

	int err;

	char buff[BUFF_SIZE];

	/* Enregistrement du driver */
	dev_t dev;

	dev = MKDEV(89, 0);
	i2c_dev = cdev_alloc();
	cdev_init(i2c_dev, &fops);

	i2c_dev->owner = THIS_MODULE;
	cdev_add(i2c_dev, dev, 1);



	/* Initialisation du périphérique i2c */
	if( (err = xeno_i2c_init()) < 0) {
		printk("I2C init error : %d", err);
		return err;
	}


	/* IO pins configuration */
	xeno_i2c_ioctl(I2C_SLAVE, 0x0);

	buff[0] = CONFIG;
	buff[1] = I2C_IO_MODES;

	if( (err = xeno_i2c_write(buff, BUFF_SIZE)) < 0) {
		printk("I2C io modes config error : %d", err);
		return err;
	}

	return 0;
}

void __exit cleanup_module(void) {
	int err;


	/* Delete device file */
	cdev_del(i2c_dev);

	/* Exit i2c module */
	if( (err = xeno_i2c_exit()) < 0) {
		printk("I2C cleanup error : %d", err);
	}
}


MODULE_LICENSE("GPL");
