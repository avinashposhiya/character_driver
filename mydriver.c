/***************************************************************************//*
*  File       mydriver.c
*
*  Details    Simple Linux device driver (IOCTL)
*
*  Author     Avinash Poshiya
*
*  Tested with Linux Beagle Bone Black and Ubuntu22.04 5.15.0-105-generic
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>			
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/err.h>
 
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

/*Maximum size of the device memory*/
#define DEVICE_MEM_SIZE 2048

/*Device Memory Buffer*/
char device_buffer[DEVICE_MEM_SIZE];
 
int32_t value = 0;
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev my_cdev;

/*
* Function Prototypes
*/
static int      __init my_driver_init(void);
static void     __exit my_driver_exit(void);
static int      my_open(struct inode *inode, struct file *file);
static int      my_release(struct inode *inode, struct file *file);
static ssize_t  my_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  my_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/*
* File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = my_read,
        .write          = my_write,
        .open           = my_open,
        .unlocked_ioctl = my_ioctl,
        .release        = my_release,
};

/*
* This function will be called when application will open the device file.
*/
static int my_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened successfully.\n");
        return 0;
}

/*
* This function will be called when application will close the device file.
*/
static int my_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed successfully.\n");
        return 0;
}

/*
** This function will be called when application will read the device file.
*/
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	size_t length = 0;
        pr_info("Read Requested for %zu bytes\n",count);
        pr_info("Current File position - %lld\n",*f_pos);
        
        /*Adjust and validate the count*/
        if((*f_pos + count) > DEVICE_MEM_SIZE){
        	count = DEVICE_MEM_SIZE - *f_pos;
        }
        
        /*Update File position to read data from device buffer*/
        length = strlen(device_buffer);
        pr_info("device_buffer length- %zu\n",length);
  	*f_pos -= (length+1);
  	
        /*Application requested Read data from driver*/
        if(copy_to_user(buf,&device_buffer[*f_pos],count)){
        	return -EFAULT;
        } 
        
        /*After data read, update the file position*/
        *f_pos += count;
        
        pr_info("Number of Bytes read %zu bytes\n",count);
        pr_info("New File position - %lld\n",*f_pos);
        
        /*Return number of bytes successfully read*/
        return count;
}

/*
* This function will be called when application will write the device file.
*/
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
        pr_info("Write Requested for %zu bytes\n",count);
        pr_info("Current File position - %lld\n",*f_pos);
        
        /*Adjust and validate the count*/
        if((*f_pos + count) > DEVICE_MEM_SIZE){
        	count = DEVICE_MEM_SIZE - *f_pos;
        }
        
        if(!count){
        	return -ENOMEM;
        }
        
        /*Application requested write data to driver*/
        if(copy_from_user(&device_buffer[*f_pos],buf,count)){
        	return -EFAULT;
        } 
        
        /*After data read, update the file position*/
        *f_pos += count;
        
        pr_info("Number of Bytes written %zu bytes\n",count);
        pr_info("New File position - %lld\n",*f_pos);
        
        /*Return number of bytes successfully written to driver*/
        return count;
}

/*
* This function will be called when application will call IOCTL on the device file.
*/
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        if( copy_from_user(&value ,(int32_t*) arg, sizeof(value)) )
                        {
                                pr_err("Data Written : Error!\n");
                        }
                        pr_info("Value = %d\n", value);
                        break;
                case RD_VALUE:
                        if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
                        {
                                pr_err("Data Read : Error!\n");
                        }
                        break;
                default:
                        pr_info("Default case.\n");
                        break;
        }
        return 0;
}
 
/*
* Module Init function will call when driver is loaded.
*/
static int __init my_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "my_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&my_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&my_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"my_class"))){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"my_device"))){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("My Sample Device Driver Inserted successfully...\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

/*
* Module exit function
*/
static void __exit my_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("My Simple Device Driver Remove...\n");
}
 
module_init(my_driver_init);
module_exit(my_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Avinash Poshiya");
MODULE_DESCRIPTION("Simple Linux device driver (IOCTL)");
MODULE_VERSION("1.0");
