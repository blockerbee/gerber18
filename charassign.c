#include <linux/init.h> 
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "charassign"
#define CLASS_NAME "assign"

MODULE_DESCRIPTION("Character device driver");
MODULE_LICENSE("GPL");

static int majorNumber;
static char message[1024]={0};//rethink on how to assign without 0
static short size_of_message;
static int numberOpens = 0;
static struct class* charassignClass = NULL;
static struct device* charassignDevice = NULL;

//prototype functions for character driver
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

//callback functions for device calls(open,read,write,release)

static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write= dev_write,
	.release=dev_release,
};

static int __init charassign_init(void){
	printk(KERN_INFO "Initializing the charassign device\n");
	//dynamically allocate major number for the device
	majorNumber = register_chrdev(0,DEVICE_NAME, &fops);
	if(majorNumber<0){
		printk(KERN_ALERT "Failed to register a Major Number for device charassign\n");
		return majorNumber;
	}
	printk(KERN_INFO "charassign registered correctly with major number %d \n", majorNumber);

	//register device class
	charassignClass=class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charassignClass)){
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class charassign\n");
		return PTR_ERR(charassignClass);
	}
	printk(KERN_INFO "charassign Device class created correctly \n");

	//register device driver
	if(IS_ERR(charassignDevice)){
		class_destroy(charassignClass);
		unregister_chrdev(majorNumber,DEVICE_NAME);
		printk(KERN_ALERT "Failed to create device\n");
		return PTR_ERR(charassignDevice);
	}
	printk(KERN_INFO "charassignDevice Class created correctly\n");
	return 0;
}

//cleanup function
static void __exit charassign_exit(void){
	device_destroy(charassignClass,MKDEV(majorNumber,0));
	class_unregister(charassignClass);
	class_destroy(charassignClass);
	unregister_chrdev(majorNumber,DEVICE_NAME);
	printk(KERN_INFO "Exiting from the charassign device\n");
}

//open function
static int dev_open(struct inode *inodep, struct file *filep){
	numberOpens++;
	printk(KERN_INFO "charassign Device has been opened %d time(s) \n",numberOpens);
	return 0;
}

//read function
static ssize_t dev_read(struct file *filep,char *buffer, size_t len, loff_t *offset){
	int error_count = 0;

	error_count	= copy_to_user(buffer,message,size_of_message);
	if(error_count==0){
		printk(KERN_INFO "charassign Sent %d characters to the user\n",size_of_message);
		//consider keeping track of the location of the message
		return (size_of_message=0);
	}
	else{
		printk(KERN_INFO "charassign Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

//write function
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len,loff_t *offset ){
	sprintf(message, "%s", buffer);
	size_of_message=strlen(message);
	printk(KERN_INFO "charassignDevice received %zu characters from the user\n", len);
	return len;
}

//device release function
static int dev_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO"charassignDevice successfully closed");
	return 0;
}

module_init(charassign_init);
module_exit(charassign_exit);