#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */
    
    /* this tick value indicates when the thread will stop sleeping and wake up*/
    int64_t ticks;

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  	
  	//extra features added for priority scheduling.
  	/* To keep the initial priority, before considering the priority donations.*/
  	int initial_priority;
  	
  	/*Lock that the thread is waiting to acquire i.e only when the thread that has captured the lock is donated the priority can it release the lock and only then can this thread waiting on lock acquire it.This is NULL if no lock for a thread i.e if the thread is waiting for no lock.*/
  	struct lock *waiting_on;
  	
  	/*each of the thread on which other high priority threads depends on has this list.This list contains those threads that are ready to be priority donars and it is present in a sorted order (high priority to low priority) that are ready to donate priority to this current thread that has the lock captured.*/
  	struct list priority_donation;
  	
  	/*list element required to donate the priority to the thread.donation_elem is there for every thread in the donationlist and this helps in the donation of the priority to the thread on which this depends upon.infact this is what gets inserted into the donation list by every thread.*/
  	struct list_elem donation_element;
		
		//extra features added for advanced scheduling.
		int niceness;/*determines how nice it should be to the other threads. This helps in deciding how much CPU time should be alloted to a thread in comparision to other threads.*/
		
		int recent_cpu_ticks;/* number of clock ticks recently used by this thread. This determines which thread to be run when there are multiple threads of same priority; in which case the thread with the minimum recent_cpu_ticks is taken. */
		  	
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

//modified these functions for priority scheduling.
int thread_get_priority (void);
void thread_set_priority (int);

//modified these functions for advanced scheduling.
int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

//added functions for alarm clock
bool cmp_ticks(const struct list_elem *a,const struct list_elem *b,void *aux UNUSED);
void priority_check (void);

//added functions for priority scheduling
bool cmp_priority (const struct list_elem *a,const struct list_elem *b, void *aux UNUSED);
void thread_recall_donation(struct thread *t);
static void thread_reinsert_ready_list(struct thread *t );
static int thread_get_donated_priority(struct thread *t);
void thread_calculate_priority(struct thread *t);
static bool thread_donation_cmp(const struct list_elem *a,const struct list_elem *b,void *aux UNUSED);
void thread_donate_priority(struct thread *t);
void thread_yield_to_max(void);
static int thread_max_priority(void);

//added functions for advanced scheduling
void thread_calculate_priority_bsd(struct thread *t, void *aux UNUSED);
static void thread_update_bsd_status(void);
static void thread_update_recent_cpu(struct thread *t, void *aux UNUSED);
static int thread_get_ready_threads(void);
static void schedule_update_sleeping_threads(void);
static void schedule_update_thread_priorities(void);


#endif /* threads/thread.h */
