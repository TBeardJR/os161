/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

typedef enum boolean { false, true} bool;

volatile bool all_dishes_available = true; 
static struct semaphore *done; 
static struct semaphore *mutex; 
static struct semaphore *dish_mutex;
volatile bool dish1_busy = false;
volatile bool dish2_busy = false;

static struct semaphore *cats_queue; 
volatile int cats_wait_count = 0; 
volatile bool no_cat_eat = true; /*first cat*/

static struct semaphore *mice_queue; 
volatile int mice_wait_count = 0; 
volatile bool no_mouse_eat = true;


/*
 * 
 * Function Definitions
 * 
 */


/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;

        bool first_cat_eat = false;
        bool another_cat_eat = false;
        int mydish = 0;


        P(mutex); 
        if (all_dishes_available == true) {
            all_dishes_available = false; 
            V(cats_queue); /* let first cat in */
        } 
        cats_wait_count++; 
        V(mutex);
        
        P(cats_queue); /* first cat in, other wait */ 
        kprintf("Cat %lu walking to the kitchen.\n", catnumber); /*cat name*/
        
        if (no_cat_eat == true) {
            no_cat_eat = false; 
            first_cat_eat = true;
        }
        else {
            first_cat_eat = false;
        } 

        if (first_cat_eat == true) { 
            P(mutex); 
            if (cats_wait_count > 1) {
                another_cat_eat = true;    
                kprintf("Cat %lu is signaling another cat. Cat %lu is the first cat.\n", catnumber, catnumber);
                V(cats_queue); /*let another cat in*/
                kprintf("Cat %lu finished signaling another cat.\n", catnumber); 
            } 
            V(mutex); 
        }
        kprintf("Cat %lu in the kitchen.\n", catnumber); /*cat name*/

        P(dish_mutex); /* protect shared variables */ 
        if (dish1_busy  == false) { 
            dish1_busy = true; 
            mydish = 1; 
        } else { 
            assert(dish2_busy == false); 
            dish2_busy = true; 
            mydish = 2; 
        } 
        V(dish_mutex); 

        kprintf("Cat %lu eating.\n", catnumber); /* cat name */ 
        clocksleep(1); /* enjoys food */ 
        kprintf("Cat %lu finished eating.\n", catnumber); /* done. */

        P(dish_mutex); /*protect shared variables*/ 
        if (mydish == 1) { /* release dish 1 */ 
            dish1_busy = false;
        } else { /* release dish 2 */
            dish2_busy = false; 
        }            
        V(dish_mutex);

        P(mutex); /*protect shared variables*/ 
        cats_wait_count--; /*reduced before leaving*/ 
        V(mutex);

        if (first_cat_eat == true) { /* first cat */ 
            if (another_cat_eat == true) {
                P(done); /* wait for another cat */
            }
            kprintf("Cat %lu is leaving (was the first cat)\n", catnumber); 
            no_cat_eat = true; /*let next cat control*/


            /* Switch to mice if any is waiting */ 
            /* (1) Wake up mice */ 
            /* (2) Wake up cat */ 
            /* (3) set all_dishes_available to true */

            /* Switch to mice if any is waiting */ 
            P(mutex); /* protect shared variables */ 
            if (mice_wait_count > 0) { /* mice waiting */ 
                V(mice_queue); /* let mice eat */ 
            } else if (cats_wait_count > 0) {
                V(cats_queue); /* let cat eat */ 
            } else {
                all_dishes_available = true; 
            }
            V(mutex);
        } else { /* non-first cat is leaving */ 
            kprintf("Cat %lu is leaving (was not first cat)\n", catnumber); 
            V(done); /* inform the first cat */ 
        }
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer, 
         unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) mousenumber;

        bool first_mouse_eat = false;
        int mydish = 1;

        P(mutex); 
        if (all_dishes_available == true) {
            all_dishes_available = false; 
            V(mice_queue); /* let first mouse in */
        } 
        mice_wait_count++; 
        V(mutex);

        P(mice_queue); /* first mouse in, other wait */       

        if (no_mouse_eat == true) {
            no_mouse_eat = false; 
            first_mouse_eat = true;
        }
        else {
            first_mouse_eat = false;
        } 

        if (first_mouse_eat == true) { 
            P(mutex); 
            if (mice_wait_count > 1) {
                V(mice_queue); /*let another mouse in*/ 
            } 
            V(mutex); 
        }

        kprintf("Mouse %lu in the kitchen.\n", mousenumber); /*mouse name*/

        P(dish_mutex); /* protect shared variables */ 
        if (dish1_busy  == false) { 
            dish1_busy = true; 
            mydish = 1;           
        } else { 
            assert(dish2_busy == false); 
            dish2_busy = true; 
            mydish = 2; 
        } 
        V(dish_mutex); 

        kprintf("Mouse %lu eating.\n", mousenumber); /* mouse name */ 
        clocksleep(1); /* enjoys food */ 
        kprintf("Mouse %lu finished eating.\n", mousenumber); /* done. */

        P(dish_mutex); /*protect shared variables*/ 
        if (mydish == 1) { /* release dish 1 */ 
            dish1_busy = false;
        } else { /* release dish 2 */
            dish2_busy = false; 
        }            
        V(dish_mutex);

        P(mutex); /*protect shared variables*/ 
        mice_wait_count--; /*reduced before leaving*/ 
        V(mutex);

        if(first_mouse_eat == true) {
            P(done); /* wait for another mouse */
            kprintf("Mouse %lu is leaving.\n", mousenumber);
            /* Switch to cats if any is waiting */ 
            /* (1) Wake up cats */ 
            /* (2) Wake up mice */ 
            /* (3) set all_dishes_available to true */
            no_mouse_eat = true; 

            /* Switch to cats if any is waiting */ 
            P(mutex); /* protect shared variables */ 
            if (cats_wait_count > 0) { /* cats waiting */ 
                V(cats_queue); /* let cat eat */
            } else if (mice_wait_count > 0) {
                V(mice_queue); /* let mice eat */ 
            } else {
                all_dishes_available = true; 
            }
            V(mutex);
        } else {
            V(done);
            kprintf("Mouse %lu is leaving.\n", mousenumber);
        }

        
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;

        done = sem_create("done", 0); 
        if (done == NULL) { 
            panic("done: Out of memory.\n"); 
        }

        mutex = sem_create("mutex", 1); 
        if (mutex == NULL) { 
            panic("mutex: Out of memory.\n"); 
        }

        dish_mutex = sem_create("dish_mutex", 1); 
        if (dish_mutex == NULL) { 
            panic("dish_mutex: Out of memory.\n"); 
        }

        cats_queue = sem_create("cats_queue", 0); 
        if (cats_queue == NULL) { 
            panic("cats_queue: Out of memory.\n"); 
        }

        mice_queue = sem_create("mice_queue", 0); 
        if (mice_queue == NULL) { 
            panic("mice_queue: Out of memory.\n"); 
        }
   
        /*
         * Start NCATS catsem() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        return 0;
}


/*
 * End of catsem.c
 */
