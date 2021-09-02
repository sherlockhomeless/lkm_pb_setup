#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include "sched.h"


   // Required for the copy to user function
#define  DEVICE_NAME "init_pbs"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "device_class"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("ml");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("base module");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  dev_class  = NULL; ///< The device-driver class struct pointer
static struct device* dev_device = NULL; ///< The device-driver device struct pointer

extern void print_something_wild(void);
static void init_pbs();

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t plan_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = plan_write,
   .release = dev_release,
};

static void init_pbs(){
    struct task_struct *proxy_task;
    struct rq *rq;

    proxy_task = kthread_create(loop_thread_func, NULL, "PB proxy thread");
    proxy_task->sched_class = &pb_sched_class;

    // this_rq returns the runqueue of the local CPU
    rq = this_rq();
    init_rq(&rq->pb);


}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init init_global_module(void){

    init_pbs();

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "[PBS_init_module] failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "[PBS_init_module]: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   dev_class = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(dev_class)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(dev_class);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "[PBS_init_module]: device class registered correctly\n");

   // Register the device driver
   dev_device = device_create(dev_class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(dev_device)){               // Clean up if there is an error
      class_destroy(dev_class);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(dev_device);
   }
    return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit exit_global_module(void){
   device_destroy(dev_class, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(dev_class);                          // unregister the device class
   class_destroy(dev_class);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "[PBS_exit_module]: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
  //printk(KERN_ALERT "dev_read called, global_task_lateness = %ld", global_pbs_task.lateness);

  return 0;
}

static ssize_t plan_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO ": Device successfully closed\n");
   return 0;
}


module_init(init_global_module);
module_exit(exit_global_module);
