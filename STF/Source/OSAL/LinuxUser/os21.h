// Dummy os21.h for code that wants to include this for everything that is not WIN32

// Use OS21 -> STF Abstraction  
#define TIMEOUT_INFINITY  0
#define TIMEOUT_IMMEDIATE 1

typedef int    mutex_t;
typedef int    event_group_t;

// Quick implementation of message queue.
typedef struct message_queue_s
{
	int             ElementSize;
	int             ElementNb;
	char          * ElementBuffer;
	char          **ElementAccessTab;
	int             ElementClaim;
	int             ElementRelease;
	int             ElementAvailable;
	
	mutex_t       * Mutex;
	event_group_t * Event; // not implemented : to be implemented if usage of message_claim_timeout(TIMEOUT_INFINITE)	
} message_queue_t;



// prototypes

_EXTERN_C_ mutex_t         * mutex_create_fifo(void);
_EXTERN_C_ void              mutex_lock   (mutex_t * os21_mutex);
_EXTERN_C_ int               mutex_release(mutex_t * os21_mutex);
_EXTERN_C_ int               mutex_delete (mutex_t * os21_mutex);

_EXTERN_C_ message_queue_t * message_create_queue(int element_size,int list_length);
_EXTERN_C_ int               message_release      (message_queue_t * msg_queue, char * msg);
_EXTERN_C_ char            * message_claim        (message_queue_t * msg_queue);
_EXTERN_C_ char            * message_claim_timeout(message_queue_t * msg_queue, int timeout);
_EXTERN_C_ void              message_delete_queue (message_queue_t * msg_queue);

_EXTERN_C_ event_group_t   * event_group_create(int option);
_EXTERN_C_ void              event_wait_any(event_group_t * event, int value, int * ptr, int timeout);
_EXTERN_C_ void              event_post(event_group_t * event, int value);
_EXTERN_C_ void              event_clear(event_group_t * event, int value);
_EXTERN_C_ void              event_group_delete(event_group_t * os21_event);

_EXTERN_C_ void              accmme_initlog(void);
_EXTERN_C_ void              accmme_loglock(const char * szFormat, ...);
_EXTERN_C_ void              accmme_logunlock(const char * szFormat, ...);
_EXTERN_C_ void              accmme_log(const char * szFormat, ...);

#ifdef _ACC_AMP_SYSTEM_
_EXTERN_C_ void              accamp_loglock(const char * szFormat, ...);
_EXTERN_C_ void              accamp_logunlock(const char * szFormat, ...);
_EXTERN_C_ void              accamp_log(const char * szFormat, ...);
#endif /* _ACC_AMP_SYSTEM_ */

