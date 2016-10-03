/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
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

static struct lock *mutex;
/* wait for the cat or mouse turn */ 
static struct cv *turn_cv;
/* wait here for process done */ 
static struct cv *done_cv;
/* Initialize mutex and two condition variables */

/* 1. NOCATMOUSE, 2. CATS 3. MICE */ 
static volatile int turn_type = 1;

int NOCATMOUSE = 1;
int CATS = 2;
int MICE = 3;

/*Variables to track dish usage*/
volatile bool dish_1_busy = false;
volatile bool dish_2_busy = false;

/*Keep track of the number of turns remaining before turn swap*/
volatile int mouse_turns_left = 0;
volatile int cat_turns_left = 0;

/*Keep track of the number of Cats and Mice waiting*/
volatile int cats_waiting = 0;
volatile int mice_waiting = 0;

/*Keep track of the number of Cats and Mice fed and remaining to know when to end*/
volatile int cats_fed = 0;
volatile int mice_fed = 0;
volatile int animals_left = 8;



/*
 * 
 * Function Definitions
 * 
 */

/*
*
* change_turn()
*
* Arguements:
*		None
*
* Returns:
*		Nothing
*
* Notes:
*		Changes the current animal turn according to the current turn and 
*		whether or not a type of animal is waiting
*
*
*/

void change_turn()
{
	/*Case 1: there are waiting mice*/ 
	if (mice_waiting > 0) {
		turn_type = MICE; 
		mouse_turns_left = 2; 
		kprintf("It is the Mice's turn now.\n");
	} /*Case 2: there are waiting cats*/
	else if (cats_waiting > 0) {
		turn_type = CATS;
		cat_turns_left = 2;
		kprintf("It is the Cats' turn now.\n"); 
	}
	else {
		turn_type = NOCATMOUSE;
	} 
		/*Wake up those waiting for turn change*/ 
		cv_broadcast(turn_cv, mutex);
}


/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
	kprintf("Cat Entered Program.\n");
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
    int mydish = 0;

    if(lock_do_i_hold(mutex) == true) {
		kprintf("Cat %lu has the lock!\n", catnumber);
	} 
	lock_acquire(mutex);	
	cats_waiting++; /* Initial value = 0 */
	if (turn_type == NOCATMOUSE) {
		turn_type = CATS; 
		cat_turns_left = 2; /*Two cats per turn*/
		kprintf("Cat went first.\n");
	}
	
	while (turn_type == MICE || cat_turns_left == 0) {
		cv_wait(turn_cv, mutex);
	} 
	
	cat_turns_left--; /* a cat enters the kitchen */ 
	
	kprintf("Cat %lu enters the kitchen.\n", catnumber);
	
	if (dish_1_busy  == false) { 
		dish_1_busy = true;
		mydish = 1;
	} 
	else {
		dish_2_busy = true; 
		mydish = 2;
	} 
	kprintf("Cat %lu is eating.\n", catnumber);  
	lock_release(mutex);

	clocksleep(1); /* enjoys food */
	
	lock_acquire(mutex); 
	kprintf("Cat %lu finishes eating at dish %d\n", catnumber, mydish);
	
	if (mydish == 1){ /* release dish 1 */
		dish_1_busy = false; 
	}
	else{ /* release dish 2 */ 
		dish_2_busy = false;
	}
	
	cats_fed++;
	cats_waiting--; /*update the number of waiting cats*/ 
	
	/* Wake up one waiting cat to enter if there are no mice waiting*/
	if (mice_waiting == 0 && cats_waiting > 0) { 
		cat_turns_left++;
		cv_broadcast(turn_cv, mutex);
	}
	
	/*Last cat and timeout. Then switch to mice.*/ 
	if (dish_1_busy == false && dish_2_busy == false && cat_turns_left == 0 ||
	    dish_1_busy == false && dish_2_busy == false && cats_waiting == 0){ 
		change_turn();
		}
	kprintf("Cat %lu leaves kitchen.\n", catnumber); 

	while(cats_fed < 6 || mice_fed < 2)
	{
		cv_wait(done_cv, mutex);
	}
	animals_left--;
	
		
	kprintf("Cat %lu has exited the program\n", catnumber);
	if(animals_left == 0)
	{
		kprintf("Cat %lu is destroying the lock\n", catnumber);
		lock_release(mutex);
		lock_destroy(mutex);
		return;
	}
	else
	{
		cv_signal(done_cv, mutex);
	}
	lock_release(mutex);

	
	
}
	

/*
 * mouselock()
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
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
	kprintf("Mouse Entered Program.\n");
        /*
         * Avoid unused variable warnings.
         */
        
        (void) unusedpointer;

	int mydish = 0;
	
	if(lock_do_i_hold(mutex) == true) {
		kprintf("Mouse %lu has the lock!\n", mousenumber);
	} 
	lock_acquire(mutex);

	mice_waiting++; /* Initial value = 0 */
	if (turn_type == NOCATMOUSE) {
		turn_type = MICE; 
		mouse_turns_left = 2; /*Two mice per turn*/
		kprintf("Mouse went first.\n");
	}
	
	while (turn_type == CATS || mouse_turns_left == 0) {
		cv_wait(turn_cv, mutex);
	} 
	mouse_turns_left--; /* a cat enters the kitchen */ 
	
	kprintf("Mouse %lu enters the kitchen.\n", mousenumber);
	
	if (dish_1_busy  == false) { 
		dish_1_busy = true;
		mydish = 1;
	} 
	else {
		dish_2_busy = true; 
		mydish = 2;
	} 
	kprintf("Mouse %lu is eating.\n", mousenumber); 
	lock_release(mutex); 
	
	clocksleep(1); /* enjoys food */
	
	lock_acquire(mutex); 
	kprintf("Mouse %lu finished eating at dish %d\n", mousenumber, mydish);
	if (mydish == 1){ /* release dish 1 */
		dish_1_busy = false; 
	}
	else{ /* release dish 2 */ 
		dish_2_busy = false;
	}
	
	mice_fed++;
	mice_waiting--; /*update the number of waiting mice*/ 
	
	/* Wake up one waiting mouse to enter */
	if (cats_waiting == 0 && mice_waiting > 0) { 
		mouse_turns_left++;
		cv_broadcast(turn_cv, mutex);
	}
	
	/*Last mouse and timeout. Then switch to cats.*/ 
	if (dish_1_busy == false && dish_2_busy == false && mouse_turns_left == 0 ||
	    dish_1_busy == false && dish_2_busy == false && mice_waiting == 0){ 
		change_turn();
		}
	kprintf("Mouse %lu leaves kitchen.\n", mousenumber); 
	
	while(cats_fed < 6 || mice_fed < 2)
	{
		cv_wait(done_cv, mutex);
	}
	animals_left--;

	kprintf("Mouse %lu has exited the program\n", mousenumber);
	if(animals_left == 0)
	{
		kprintf("Mouse %lu is destroying the lock\n", mousenumber);
		lock_release(mutex);
		lock_destroy(mutex);
		return;
	}
	else
	{
		cv_signal(done_cv, mutex);
	}
	lock_release(mutex);

	
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
		 
		mutex = lock_create("catlock mutex"); 
		turn_cv = cv_create("catlock turn cv"); 
		done_cv = cv_create("catlock done cv"); 

		

   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
   
        /*
         * Start NCATS catlock() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        return 0;
}

/*
 * End of catlock.c
 */
