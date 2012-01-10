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

#define I2C_IO_MODES 0xF0;


typedef enum i2c_commands_t {
	INPUT,
	OUTPUT,
	POLARITY,
	CONFIG
} i2c_commands_t;

struct file_operations fops = {
		// FIXME bizarre ici, il veut pas .close
		//.close 	= pca9554_close,
		.open 	= pca9554_open,
		//pca9554_ioctl
		.read 	= pca9554_read,
		.write	= pca9554_write,
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

	/* A compléter ... */

	return 0;
}

ssize_t pca9554_read(struct file *file, char __user *buff, size_t len, loff_t *off) {
	int err;
	int i;
	char localBuff[1];

	if( (err = xeno_i2c_read(localBuff, len)) < 0) {
		printk("I2C read error : %d", err);
		return err;
	}

	cpoy_to_user(localBuff, buff);


	return 0;
}

ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *off) {
	int err;
	if( (err = xeno_i2c_write(buff, len)) < 0) {
		printk("I2C write error : %d", err);
		return err;
	}

	return 0;
}

int __init init_module(void) {
	int err;
	if( (err = xeno_i2c_init()) < 0) {
		printk("I2C init error : %d", err);
		return err;
	}

	/* IO pins configuration */
	xeno_i2c_ioctl(I2C_SLAVE, 0x0);

	size_t len;
	len = 2;

	char buff[len];

	buff[0] = CONFIG;
	buff[1] = I2C_IO_MODES;

	if( (err = xeno_i2c_write(buff, len)) < 0) {
		printk("I2C io modes config error : %d", err);
		return err;
	}

	return 0;
}

void __exit cleanup_module(void) {
	int err;
	if( (err = xeno_i2c_exit()) < 0) {
		printk("I2C cleanup error : %d", err);
	}

}

MODULE_LICENSE("GPL");
