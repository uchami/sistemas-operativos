#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include <linux/device.h>

ssize_t nulo_read(struct file *filp, char __user *data, size_t s,loff_t *off) {
    return 0;
}

ssize_t nulo_write(struct file *filp, const char __user *data, size_t s,loff_t *off) {
    return s;
}

static struct cdev device;
static struct file_operations operaciones = {
    .owner = THIS_MODULE,
    .read = nulo_read,
    .write = nulo_write,
};
static dev_t major;
static struct class *mi_class;

static int __init nulo_init(void) {
    //Primer param: struct cdev *cdev, "El primer parámetro es una estructura que representará al dispositivo."??
    cdev_init(&device, &operaciones);

    alloc_chrdev_region(&major, 0, 1, "nulo");

    cdev_add(&device, major, 1);

    mi_class = class_create(THIS_MODULE, "nulo");
    device_create(mi_class, NULL, major, NULL, "nulo");

	printk(KERN_ALERT "Nulo\n");
	return 0;
}

static void __exit nulo_exit(void) {

    unregister_chrdev_region(major, 1);

    cdev_del(&device);

    device_destroy(mi_class, major); 
    class_destroy(mi_class);

	printk(KERN_ALERT "Fin nulo\n");
}

module_init(nulo_init);
module_exit(nulo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de 'Hola, mundo'");
