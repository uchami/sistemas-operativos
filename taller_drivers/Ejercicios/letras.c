#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include <linux/device.h>



ssize_t letras_read(struct file *filp, char __user *data, size_t s,loff_t *off) {
    return 0;
}

ssize_t letras_write(struct file *filp, const char __user *data, size_t s,loff_t *off) {
    return s;
}

static struct cdev device;
static struct file_operations operaciones = {
    .owner = THIS_MODULE,
    .read = letras_read,
    .write = letras_write,
    .open = letras_open,
    .close = letras_close 
};
static dev_t major;
static struct class *mi_class;

static int __init letras_init(void) {
    //Primer param: struct cdev *cdev, "El primer parámetro es una estructura que representará al dispositivo."??
    cdev_init(&device, &operaciones);

    alloc_chrdev_region(&major, 0, 1, "letras");

    cdev_add(&device, major, 1);

    mi_class = class_create(THIS_MODULE, "letras");
    device_create(mi_class, NULL, major, NULL, "letras");

	printk(KERN_ALERT "Letras\n");
	return 0;
}

static void __exit nulo_exit(void) {

    unregister_chrdev_region(major, 1);

    cdev_del(&device);

    device_destroy(mi_class, major); 
    class_destroy(mi_class);

	printk(KERN_ALERT "Fin nulo\n");
}

module_init(letras_init);
module_exit(letras_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de 'Hola, mundo'");
