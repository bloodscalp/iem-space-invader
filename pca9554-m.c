#include "pca9554-m.h"

/******************************************************************************
 * Fichier : 		pca9554-m.c
 *
 * Auteurs : 		Daniel Rossier (DRE)			-	Version decembre 08
 *
 * Modifications : 	Joel Golinucci et Daniel Molla	-	v. 22.01.2010
 *
 * Description :	Driver de type caractère permettant les interactions avec
 * 					le driver I2C fourni. Il peut etre utilise par l'espace
 * 					utilisateur via les appels systeme ou directement pas
 * 					l'espace noyau par appels des callbacks.
 ******************************************************************************/

static int buf_size = 64;
static char *buffer;


/******************************************************************************
 * Fonction  : 		pca9554_open
 *
 * But  : 			Implementation du callback open du driver
 *
 * Description : 	Permet d'initialiser la communication avec la
 * 					ligne I2C et configure le driver de maniere a pouvoir
 * 					par la suite lire l'etat des switch et ecrire
 * 					les etats des leds.
 *
 * Parametres :   	struct inode * noeud  - l'inode (renseigne par le VFS)
 * 					struct file * fichier - le fichier (renseigne par le VFS)
 *
 * Retour : 		code d'erreur : 0 en cas de succes, -1 pour une erreur
******************************************************************************/
int pca9554_open(struct inode * noeud, struct file * fichier)
{
	int err;			// Code d'erreur local
	char buffer [2];  	// Buffer utilise pour la configuration des registres I/O

	// Initialisation du buffer avec les commandes de configuration souhaitees
	buffer [0] = CMD_CONFIG;
	buffer [1] = 0xf0;

	// Initialisation de la couche i2c
	if ((err = xeno_i2c_init()) >= 0)
	{
		// Configuration de la couche i2c afin de renseigner l'adresse du peripherique
		// i2c que l'on souhaite piloter
		if ((err = xeno_i2c_ioctl((unsigned int)I2C_SLAVE, (unsigned long)I2C_ADDRESS)) >= 0)
		{
			// Permet d'eviter que l'on utilise au niveau du noyeau les autres
			// callbacks (read / write) sans etre passe par le open d'abord
			connectionOpen = TRUE;

			// Configuration des registres I/O
			xeno_i2c_write (buffer, 2);

			return 0;
		}
		else
		{
			switch (-err)
			{
				case EINVAL:
					printk ("Error ioctl : wrong value !\n");
				case EBUSY:
					printk ("Error ioctl : device busy !\n");
				default:
					printk ("Error in xeno_i2c_ioctl() !\n");
			}
			return -1;
		}
	}
	else
	{
		switch (-err)
		{
			case ENODEV:
				printk ("Error init : no device !\n");
			case ENOMEM:
				printk ("Error init : not enough memory !\n");
			default:
				printk ("Error in xeno_i2c_init() !\n");
		}
		return -1;
	}

}


/******************************************************************************
 * Fonction  : 		pca9554_ioctl
 *
 * But  : 			Implementation du callback ioctl du driver
 *
 * Description : 	Permet de transmettre une comande de configuration
 * 					directement a la couche xeno i2c.
 * 					Aucune verification n'est effectuee au niveau des
 * 					parametres recus !
 *
 * Parametres :     struct inode * noeud	- l'inode (renseigne par le VFS)
 * 					struct file * fichier	- le fichier (renseigne par le VFS)
 * 					unsigned int cmd 		- commande ioctl
 * 					unsigned long arg 		- valeur a transmettre correspondant a la commande cmd.
 *
 * Retour : 		code d'erreur : 0 en cas de succes, <0 pour une erreur
 ******************************************************************************/
int pca9554_ioctl(struct inode * noeud, struct file * fichier, unsigned int cmd, unsigned long arg)
{
	if(connectionOpen)
		return xeno_i2c_ioctl((unsigned)cmd, (unsigned long)arg);
	else
		return -1;
}


/******************************************************************************
 * Fonction  : 		pca9554_close
 *
 * But  : 			Implementation du callback close du driver
 *
 * Description : 	Permet de fermer la communication avec le driver xeno i2c.
 *
 * Parametres :   	-
 *
 * Retour : 		code d'erreur : 0 en cas de succes, -1 pour une erreur
 ******************************************************************************/
int pca9554_close(void)
{
	int err;

	if ((err = xeno_i2c_exit()) >= 0)
	{
		printk ("pca9554_close () : successfull !\n");
		connectionOpen = FALSE;
		return 0;
	}
	else
	{
		printk ("Error in exit function !\n");
		return -1;
	}
}


/******************************************************************************
 * Fonction  : 		pca9554_read
 *
 * But  : 			Implementation du callback read du driver
 *
 * Description : 	Permet de lire la valeur des switchs via le bus i2c
 * 					et de la retourer par le biais d'un buffer a l'appelant.
 *
 * Parametres :   	struct file * fichier - le fichier (renseigne par le VFS)
 *					char *buff 			  - un pointeur sur le buffer a remplir
 *											avec a valeur des switch
 *					size_t length 		  - quantite (nb d'octets) a lire (non utilise)
 *					loff_t *offset 		  - decalage (non utilise)
 *
 * Retour : 		retourne le nombre de bytes lus ou -1 en cas d'erreur
******************************************************************************/
ssize_t pca9554_read(struct file *file, char *buff, size_t length, loff_t *offset)
{
	char command [1] = {CMD_READ};	// Utilise pour l'envoie de la commande read
	char bufferLocal[1];			// Utilise pour la reception des donnees
	int  bytesLus;					// Nombre de byte lu lors du read

	if (connectionOpen)
	{
		xeno_i2c_write (command, 1);
		if ((bytesLus = xeno_i2c_read (bufferLocal, 1)) < 0)
			return -1;

		// On utilise le parametre file afin de savoir si l'appel vient de l'espace
		// utilisateur ou de l'espace noyeau.
		if ( ((file != NULL) && (copy_to_user(buff, bufferLocal, 1) != -1) ) ||
			 ((file == NULL) && (      memcpy(buff, bufferLocal, 1) != NULL) ) )
			return bytesLus;
		else
			return -1;
	}
	else
	{
		printk ("Error in read function!\n");
		return -1;
	}
}


/******************************************************************************
 * Fonction  : 		pca9554_write
 *
 * But  : 			Implementation du callback write du driver
 *
 * Description : 	Permet de d'ecrire l'etat des leds fournis en parametre via le bus i2c.
 *
 * Parametres :   	struct file * fichier   - le fichier (renseigne par le VFS)
 *					const char __user *buff - un pointeur sur le buffer contenant
 *								              l'etat des leds a ecrire.
 *					size_t len              - quantite (nb d'octets) a lire (non utilise)
 *				  	loff_t *offset          - decalage (non utilise)
 *
 * Retour : 		retourne le nombre de bytes ecris ou -1 en cas d'erreur
******************************************************************************/
ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *offset)
{
	char buffer [2];		// Buffer utilise pour l'envoie des donnee
	buffer[0] = CMD_WRITE;	// Utilise pour l'envoie de la commande write

	if (connectionOpen)
	{
		if ( ((file != NULL) && (copy_from_user(&buffer[1], buff, 1) != -1) ) ||
			 ((file == NULL) && (        memcpy(&buffer[1], buff, 1) != NULL) ) )
			return xeno_i2c_write (buffer, 2);
		else
			return -1;
	}
	else
	{
		printk ("Error in write function!\n");
		return -1;
	}
}


/******************************************************************************
 * Affectation des callbacks du driver avec les syscall
 ******************************************************************************/
struct file_operations fops =
{
		.open  = pca9554_open,
		.read  = pca9554_read,
		.write = pca9554_write,
		.ioctl = pca9554_ioctl,
};


/******************************************************************************
 * Fonction  : 	init_module
 * But  : 		Point d'entree du driver. Permet l'initialisation et
 * 				l'enregistrement du driver aupres du noyau
 * Parametres : -
 * Retour : 	code d'erreur pour l'os (toujours 0 dans notre cas)
******************************************************************************/
int __init init_module(void)
{
	dev_t dev;
	buffer = kmalloc(buf_size, GFP_KERNEL);
	dev = MKDEV (126, 0);
	my_dev = cdev_alloc ();
	cdev_init (my_dev, &fops);
	my_dev->owner = THIS_MODULE;
	cdev_add (my_dev, dev, 1);

	return 0;
}


/******************************************************************************
 * Fonction  : 	cleanup_module
 * But  : 		Point de sortie du driver. Cette fonction est appellee
 *				lors du dechargement du module du noyeau.
 * Parametres : -
******************************************************************************/
void __exit cleanup_module(void)
{
	printk ("Bye bye !\n");
	pca9554_close ();
	cdev_del (my_dev);
}


// Exportation des fonctions du driver pour une utilisation directe au sein du noyau
EXPORT_SYMBOL (pca9554_open);
EXPORT_SYMBOL (pca9554_read);
EXPORT_SYMBOL (pca9554_write);
EXPORT_SYMBOL (pca9554_ioctl);


MODULE_LICENSE("GPL");
