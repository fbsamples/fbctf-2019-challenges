#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>   
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/random.h>
#include "pets.h"

#define  DEVICE_NAME "kpets"
#define  CLASS_NAME  "kpets" 

//#define DEBUG
#define DISTRIBUTE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bob Dylan");
MODULE_DESCRIPTION("Kernel Pets Simulator");
MODULE_VERSION("0.1");
 
static int    majorNumber;
static struct class*  class  = NULL;
static struct device* kpetsDevice = NULL;
 
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int check_for_dragon(void);
static void clear_pets(void);
static char *choose_owner(void);
static int get_random(void);
 
size_t max_pets = 10;
static void* chunk;
static size_t chunk_size;
static struct pet* first_slot;
//static char supported_pets[] = {0xc0, 0xc1, 0xc2, 0xc3};
#ifndef DISTRIBUTE
static char flag[] = "fb{double_the_fetch_for_double_the_fun}\n";
#else
static char flag[] = "fb{***********************************}\n";
#endif
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};
 
static int __init kpets_init(void){
    printk("kpets: initializing stuff\n");

    chunk_size = max_pets * sizeof(struct pet);
    chunk = (struct pet* )kmalloc(chunk_size + 0x1000, GFP_KERNEL);
    memset(chunk, 0, chunk_size);
    first_slot = (struct pet*)chunk;
    first_slot += max_pets - 1;

#ifdef DEBUG    
    printk("[*] chunk: %p\n", chunk);
    printk("[*] first_slot: %p\n", first_slot);
#endif

    // Try to dynamically allocate a major number for the device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
      printk("kpets: failed to register a major number\n");
      return majorNumber;
    }

    // Register the device class
    class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(class)){
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk("kpets: failed to register device class\n");
      return PTR_ERR(class);
    }

    // Register the device driver
    kpetsDevice = device_create(class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(kpetsDevice)){
      class_destroy(class);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk("kpets: failed to create the device\n");
      return PTR_ERR(kpetsDevice);
    }

    printk("kpets: welcome to Kernel Pets Simulator!\n");
    return 0;
}
 
static void __exit kpets_exit(void){
   device_destroy(class, MKDEV(majorNumber, 0));
   class_unregister(class);
   class_destroy(class);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk("kpets: goodbye, your pets will miss you!\n");
}
 
static int dev_open(struct inode *inodep, struct file *filep){
#ifdef DEBUG
   printk("kpets: device opened\n");
#endif
   return 0;
}
 
static int dev_release(struct inode *inodep, struct file *filep){
#ifdef DEBUG
   printk("kpets: device closed\n");
#endif
   return 0;
}

//static int bytes_to_copy  = -1;
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    if(check_for_dragon()) {
        int total = strlen(flag);
        ssize_t bytes = len < (total-(*offset)) ? len : (total-(*offset));
        if(copy_to_user(buffer, flag, bytes)){
            return -EFAULT;
        }
        (*offset) += bytes;
        return bytes;
    }

    int i;
    struct pet* p;
    for(i = max_pets - 1, p = first_slot; i > -1; i--, p--) {
        if(p->type != 0) {
            printk("Next pet\n");

            switch(p->type) {
                case '\xc0':
                    printk("Type: dog\n");
                    break;
                case '\xc1':
                    printk("Type: cat\n");
                    break;
                case '\xc2':
                    printk("Type: sheep\n");
                    break;
                default:
                    printk("Type: unknown\n");
                    break;
            }
            printk("Name: %s\n", &p->name);
            printk("Description: %s\n", &p->description);
        }
    } 

    return 0;
}
 
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
#ifdef DEBUG
    printk("kpets: received pet...\n");
#endif
    struct pet new_pet;
    struct pet* user_pet = (struct pet *)buffer;
    
    unsigned int pet_name_len;
    copy_from_user(&pet_name_len, &user_pet->name_len, sizeof(unsigned int));
    // Validate name len
    if(pet_name_len > sizeof(new_pet.name)) {
        printk("kpets: invalid pet name len: 0x%02x\n", pet_name_len);
        return len;
    }
#ifdef DEBUG
    printk("kpets: first len: %u\n", pet_name_len);
#endif
    unsigned int pet_desc_len;
    copy_from_user(&pet_desc_len, &user_pet->description_len, sizeof(unsigned int));
    // Validate description length
    if(pet_desc_len > sizeof(new_pet.description)) {
        printk("kpets: invalid pet description len: 0x%02x\n", pet_desc_len);
        return len;
    } 

    // Find next free slot
    int i;
    struct pet* p;
    for(i = max_pets - 1, p=first_slot; i > -1 && p->type; i--,p--) {}
    if(i == -1) {
#ifdef DEBUG
        printk("kpets: pet store is full! Time to clear out!\n");
#endif
        clear_pets();
        p = first_slot;
        i = max_pets - 1;
    }
#ifdef DEBUG
    printk("kpets: adding at slot %d\n", i);
#endif

    // Give time for someone to pwn us
    printk("kpets: your new pet owner is %s!", choose_owner());

    char type;
    copy_from_user(&type, &user_pet->type, sizeof(char));
    if(type != '\xc0' && type != '\xc1' && type != '\xc2') {
        printk("kpets: invalid pet type: 0x%02hhx\n", type);
        return len;
    }

    // This is the vulnerability
    unsigned int vuln_name_len;
    copy_from_user(&vuln_name_len, &user_pet->name_len, sizeof(unsigned int));
     
    p->type = type;
    copy_from_user(&p->name, &user_pet->name, vuln_name_len);
    copy_from_user(&p->description, &user_pet->description, pet_desc_len);

#ifdef DEBUG
    printk("New pet type: 0x%02hhx\n", type);
    printk("New pet name: %s\n", &p->name);
    printk("New pet description: %s\n", &p->description);
    if (pet_name_len != vuln_name_len) {
        printk("PWNEDPWNEDPWNED\n");
    }
    printk("Second len: %u\n", vuln_name_len);
#endif
    
    return len;
}

static void clear_pets() {
    memset(chunk, 0, chunk_size);
}

static int check_for_dragon() {
    int i;
    struct pet* p;
    for(i = max_pets - 1, p = first_slot; i > -1; i--, p--) {
        if(p->type == '\xaa') {
            return 1;
        }
    }
    return 0;
}

char *names[] = {"Archie", "Jet", "Elton", "Babe", "Big Foot", "Elsa"};

static int get_random() {
    int out = 0, i = 0;
    for(i = 0; i < sizeof(out); i++) {
        get_random_bytes((char *)&out + i, 1);
        msleep(1);
    }
    return out;
}
static char *choose_owner() {
    int idx = get_random();
    return names[idx % 6];
}


module_init(kpets_init);
module_exit(kpets_exit);

