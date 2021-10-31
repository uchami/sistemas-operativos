#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>

static struct cdev device;
static dev_t major;
static int ultimo_valor = 0;

// int min(size_t a, size_t b) {
//     if(a < b) {
//         return a;
//     } else {
//         return b;
//     }
// }

ssize_t azar_read(struct file *filp, char __user *data, size_t s,loff_t *off) {
    if(ultimo_valor == 0){
        return -EPERM;
    }
    int random;
    get_random_bytes(&random, sizeof(int));
    random = random % ultimo_valor;
    
    char* buffer = kmalloc(s+2,NULL);
    size_t longitud = (size_t) snprintf(buffer,s,"%d\n",random);
    

    copy_to_user(data,buffer, min(longitud, s));

    kfree(buffer);
    return min(longitud, s);
}


ssize_t azar_write(struct file *filp, char __user *data, size_t s,loff_t *off) {
    
    char* copy = kmalloc(s+1, NULL);
    copy_from_user(copy, data, s);
    copy[s] = 0;

    if( kstrtoint(copy,10,&ultimo_valor) == 0 && ultimo_valor >= 0) {
        kfree(copy);
        return s;
    } else {
        kfree(copy);
        return -EPERM;
    }

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

    alloc_chrdev_region(&major, 0, 1, "azar");

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

module_init(azar_init);
module_exit(azar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de 'Hola, mundo'");
