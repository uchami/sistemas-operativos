#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/random.h>

#include <linux/device.h>

static struct cdev device;
static dev_t major;
int ultimo_valor = -1;

ssize_t azar_read(struct file *filp, char __user *data, size_t s,loff_t *off) {
    if(ultimo_valor == -1){
        return -EPERM;
    }
    int random;
    get_random_bytes(&random, sizeof(int));
    random = random % ultimo_valor;
    //ssize_t bufferSize = snprintf(NULL, sizeof(int), "%s", random);
    
    char* buffer = kmalloc(bufferSize+1);
    snprintf(buffer,bufferSize, "%d",random);
    
    buffer[bufferSize] = '\n';

    copy_to_user(data,buffer,bufferSize+1);

    kfree(&buffer);

    return bufferSize+1;
}


ssize_t azar_write(struct file *filp, char __user *data, size_t s,loff_t *off) {
    
    char copy[s+1];
    copy_from_user(&copy[0],data,s);
    copy[s] = "\0";

    printk(KERN_ALERT "%s",copy);
    
    kstrtoint(copy,10,&ultimo_valor);

    return s;
}

static struct file_operations operaciones = {
    .owner = THIS_MODULE,
    .read = azar_read,
    .write = azar_write,
};

static struct class *mi_class;

static int __init azar_init(void) {
    //Primer param: struct cdev *cdev, "El primer parámetro es una estructura que representará al dispositivo."??
    cdev_init(&device, &operaciones);

    alloc_chrdev_region(major, 0, 1, "azar");

    cdev_add(&device, major, 1);

    mi_class = class_create(THIS_MODULE, "azar");
    device_create(mi_class, NULL, major, NULL, "azar");

	printk(KERN_ALERT "Azar\n");
	return 0;
}

static void __exit azar_exit(void) {

    unregister_chrdev_region(mi_class, major);

    cdev_del(&device);

    device_destroy(mi_class, major); 
    class_destroy(mi_class);

	printk(KERN_ALERT "Fin azar\n");
}

module_init(nulo_init);
module_exit(nulo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de 'Hola, mundo'");
