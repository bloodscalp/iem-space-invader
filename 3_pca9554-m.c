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
#define BUF_SIZE 64 		/*Size of the buffer*/
#define DEVICE_NAME "pca9554"
#define MAJOR_NUMBER 126
#define MINOR_NUMBER 0


#define CONNECT			1		// Périphérique connecté avec le driver.
#define NO_CONNECT		0		// Périphérique pas connecté avec le driver.

// Permet d'indiquer si l'utilisateur a ouvert le driver du
// périphéàrique.
unsigned int peripherique = NO_CONNECT;

struct cdev *my_dev;



// Callbacks avec le driver à partir des appels systèmes.
static struct file_operations fops =
{
		.owner 		= THIS_MODULE,
		.open 		= pca9554_open,
		.ioctl 		= pca9554_ioctl,
		.release 	= pca9554_close,
		.read 		= pca9554_read,
		.write		= pca9554_write,
};

// Permet de réaliser l'initialisation de l'I2C.
int pca9554_open (struct inode *inode, struct file *file)
{
	char buf[2];
	int code_erreur = 0;

	printk("fonction open\n");

	// Initialisation de l'I2C
	code_erreur = xeno_i2c_init(); 
	if(code_erreur !=0)
	{
		printk("Erreur initialisation\n");
		return code_erreur;
	}

	// Configure l'I2C en mode write
	code_erreur = xeno_i2c_ioctl(I2C_SLAVE, 0x20); 
	if(code_erreur != 0)
	{
		printk("Erreur configuration I2C\n");
		return code_erreur;
	}

	buf[0] = 0x03;	// Ecriture => configuration registre.
	buf[1] = 0xF0;	// Bits 7..4 => entrées / Bits 3..0 => sorties..

	/* Ecriture sur le bus I2C.
	 * Nous n'avons pas récupérer le paramètre de retour,
	 * qui indique le nombre de byte transmit avec succès.*/
	xeno_i2c_write (buf, 2);
	
	peripherique = CONNECT;

	return 0;
}

// Permet de modifier la configuration de l'I2C
int pca9554_ioctl(struct inode * inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	// Configuration du périphérique.
	if(peripherique)
		return xeno_i2c_ioctl(cmd,arg);
	
	return -1;
}

// Permet de fermer le driver qui gère l'I2C.
int pca9554_close(struct inode *inode, struct file *file)
{
	int code_erreur = 0;

	printk("fonction close\n");
	
	
	code_erreur = xeno_i2c_exit();
	
	if(!code_erreur)	// Fermeture de l'I2C
	{
		peripherique = NO_CONNECT;
	}

	return code_erreur;
}

// Permet de lire la valeur des boutons par le biais du bus I2C.
ssize_t pca9554_read(struct file *file, char __user *buff, size_t len, loff_t *off)
{
	char buf[2];
	unsigned int taille = 0;
	
	// Si le périphérique est connecté
	printk("before peripherique \n");
	if(peripherique)
	{
		// Mode lecture
		buf[0] = 0x00;
		printk("before i2c write \n");
		xeno_i2c_write(&buf[0],1);
		

		// Lecture du message
		taille = xeno_i2c_read(&buf[1],1);
		
		printk("Taille = %d \n", taille);

		// Tous les bytes ont été transmit
		if(!taille)
		{
			printk("before copy_to_user \n");
			if (((file != NULL) && (copy_to_user(buff,&buf[1],1) != -1)) ||
				((file == NULL ) && (memcpy(buff, &buf[1], 1) != NULL)))
			{
				printk("before return\n");
				return taille;
			}
		}
	}
	printk("Une erreur est survenu lors de la lecture \n");
	return -1;
}

// Permet d'écrire sur les leds par le biais du bus de l'I2C.
ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
	char buf[2];

	// Si le périphérique est connecter?
	if(peripherique)	
	{
		buf[0] = 0x01;		// Ecriture sur leds => output port register.
		
		if (((file != NULL) && copy_from_user(&buf[1],buff,1) != -1) ||
			((file == NULL) && (memcpy(&buf[1], buff, 1) != NULL)))
		{
			return xeno_i2c_write (buf, 2);
		}
	}
	printk("Une erreur est survenu lors de l'écriture \n");
	return -1;
}

int __init init_module(void)
{
	dev_t dev;

	printk("\n\n Initialisation du module \n\n");

	dev = MKDEV(MAJOR_NUMBER, MINOR_NUMBER);
	my_dev = cdev_alloc();
	cdev_init(my_dev, &fops);
	my_dev->owner = THIS_MODULE;
	cdev_add(my_dev, dev, 1);

	return 0;
}

void __exit cleanup_module(void)
{
	printk("\n\n Exit module \n\n");
	
	if(peripherique)
	{
		xeno_i2c_exit();
		peripherique = NO_CONNECT;
	}
	
	cdev_del(my_dev);
}


MODULE_LICENSE("GPL");
EXPORT_SYMBOL(pca9554_read);
EXPORT_SYMBOL(pca9554_open);
EXPORT_SYMBOL(pca9554_close);
