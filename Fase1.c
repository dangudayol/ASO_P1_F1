/**
 * @file   Fase1.c
 * @author Dan Gudayol
 * @date   Nov 2021
 * @brief Fase1
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 
#include <linux/interrupt.h>            
#include <linux/moduleparam.h>  
#include <linux/sched.h>
#include <linux/kmod.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dan Gudayol");
MODULE_DESCRIPTION("Fase1");

//estructura para los botones
typedef struct{
	int gpio;
	int handler;
   int numPresses;
}BUTTONLED;

BUTTONLED BUTTON[4];


// estructura para los LED
typedef struct{
	int gpio;
}LEDBUTTON;

LEDBUTTON LED[2];

void runScript (int button){
   char *argv[] = { "/home/pi/button1.sh", NULL };
   static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

   
   if (button == 1){
      argv[0] = kasprintf(GFP_KERNEL, "/home/pi/button2.sh");
   }
   
   if (button == 2){
      argv[0] = kasprintf(GFP_KERNEL, "/home/pi/button3.sh");
   }
   
   if (button == 3){
      argv[0] = kasprintf(GFP_KERNEL, "/home/pi/button4.sh");
   }

   call_usermodehelper( argv[0], argv, envp, UMH_NO_WAIT ); 
}


static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   
   if (irq == BUTTON[0].handler){
      gpio_set_value(LED[0].gpio, 1);          // poner LED como toca
      printk(KERN_INFO "Fase1: Button 0 created an interrupt!");
      BUTTON[0].numPresses++;                         // contador de pulsaciones de boton
      runScript(0);
      
   }
   
   if (irq == BUTTON[1].handler){
      gpio_set_value(LED[0].gpio, 0);          // poner LED como toca
      printk(KERN_INFO "Fase1: Button 1 created an interrupt!");
      BUTTON[1].numPresses++;                         // contador de pulsaciones de boton
      runScript(1);
   }
   
   if (irq == BUTTON[2].handler){
      gpio_set_value(LED[1].gpio, 1);          // poner LED como toca
      printk(KERN_INFO "Fase1: Button 2 created an interrupt!");
      BUTTON[2].numPresses++;                         // contador de pulsaciones de boton
      runScript(2);
   }
   
   if (irq == BUTTON[3].handler){
      gpio_set_value(LED[1].gpio, 0);          // poner LED como toca
      printk(KERN_INFO "Fase1: Button 3 created an interrupt!");
      BUTTON[3].numPresses++;                         // contador de pulsaciones de boton
      runScript(3);
   }
   return (irq_handler_t) IRQ_HANDLED;      //anunciar que la interrupcion se ha dado a cabo con exito
}

//funcion de inicializacion del driver
static int __init ebbgpio_init(void){
   int result = 0;
   int i=0;
   
   printk(KERN_INFO "Installing Fase1.c Driver\n");


   //iniciamos los botones con los pines que nos interesan
	BUTTON[0].gpio=26;
	BUTTON[1].gpio=19;
	BUTTON[2].gpio=13;
	BUTTON[3].gpio=21;
    
   LED[0].gpio=20;
   LED[1].gpio=16;

   // decraramos todos los leds 
   for(i=0; i<2; i++){
      gpio_request(LED[i].gpio, "sysfs");          // pedimos cada GPIO de los leds
      gpio_direction_output(LED[i].gpio, 1);   // ponemos el GPIO como salida y en estado ON
      gpio_export(LED[i].gpio, false);             // Causes gpio49 to appear in /sys/class/gpio
         // Miramos que el pin este isponible para salida
      if (!gpio_is_valid(LED[i].gpio)){
         printk(KERN_INFO "Fase1: invalid GPIO for LED\n");
         return -ENODEV;
      }
    }
    
    // bucle para el setup de los 4 botones
   for(i=0; i<4; i++){
      gpio_request(BUTTON[i].gpio, "sysfs");       // Pedimos cada GPIO de los 4 botones
      gpio_direction_input(BUTTON[i].gpio);        // declaramos GPIO como input
      gpio_set_debounce(BUTTON[i].gpio, 200);      // controlamos los rebotes a 200ms
      gpio_export(BUTTON[i].gpio, false);          
      printk(KERN_INFO "Fase1: The button state is currently: %d\n", gpio_get_value(BUTTON[i].gpio));

      //Declaramos los handler para saber que boton nos ha creado la interrupcion
      BUTTON[i].handler = gpio_to_irq(BUTTON[i].gpio);
      printk(KERN_INFO "Fase1: The button is mapped to handler: %d\n", BUTTON[i].handler);

      // creamos los interrupt
      result = request_irq(BUTTON[i].handler,             // el handler de la interrupt que nos interesa
                           (irq_handler_t) ebbgpio_irq_handler, 
                           IRQF_TRIGGER_RISING,   
                           "ebb_gpio_handler",    
                           NULL);                
    }
   
   return result;
}

// funcion de salida del driver
static void __exit ebbgpio_exit(void){
   int i = 0;
   for(i=0; i<4; i++){
      printk(KERN_INFO "Fase1: The button %d was pressed %d times\n",i , BUTTON[i].numPresses);
      free_irq(BUTTON[i].handler, NULL);           // liberar IQR
      gpio_unexport(BUTTON[i].gpio);               
      gpio_free(BUTTON[i].gpio);                   // liberar boton
   }
   
   for(i=0; i<2; i++){
      gpio_set_value(LED[i].gpio, 0);              // apagamos todos los LED
      gpio_unexport(LED[i].gpio);                  
      gpio_free(LED[i].gpio);                      // liberar los leds
   }
   
   printk(KERN_INFO "Fase1: Removing Fase1.c Driver\n");
}

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);
