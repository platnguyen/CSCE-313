#include <threading.h>
#include <ucontext.h>
#include <stdio.h>
//struct worker_context contexts[NUM_CTX];
//uint8_t current_context_idx;

//To manage the stack for each context
static char* context_stacks[NUM_CTX];

void t_init()
{
	// Initialize all context entries to be invalid
    	for (int i = 0; i < NUM_CTX; ++i) {
        	contexts[i].state = INVALID;
        	context_stacks[i] = NULL; //set our stack storage to null
    	}
    	// The main execution flow is our first context (context 0)
	current_context_idx = 0;
	contexts[0].state = VALID;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
	volatile int new_index = -1;

	for (volatile int i = 1; i < NUM_CTX; i++) {
		if (contexts[i].state == INVALID || contexts[i].state == DONE) {
			new_index = i;
			break;
		}
	}
	if (new_index == -1) {
		fprintf(stderr, "Max contexts reached!\n");
		return 1;
	}
	//If the slot we want to access in our stack storage isn't null, we have to clear it
	if (context_stacks[new_index] != NULL) {
		free(context_stacks[new_index]);
		context_stacks[new_index] = NULL;
	}
	context_stacks[new_index] = (char*) malloc(STK_SZ);
	//if this allocation didn't work, throw an error!
	if (context_stacks[new_index] == NULL) {
		fprintf(stderr, "Can't allocate space for new context!\n");
		return 1;
	}
	//initialize the context
	getcontext(&contexts[new_index].context);
	//set up the stack
	contexts[new_index].context.uc_stack.ss_sp = context_stacks[new_index];
	contexts[new_index].context.uc_stack.ss_size = STK_SZ;
	//set up link to null. function will always call finish
	contexts[new_index].context.uc_link = NULL;
	//create the context with the passed parameters
	makecontext(&contexts[new_index].context, (void (*)(void))foo, 2, arg1, arg2);
	contexts[new_index].state = VALID; 
	return 0;
}

int32_t t_yield()
{
	//find the next available context
        int next_index = -1;
	int search_start_pos = (current_context_idx + 1) % NUM_CTX; //go to the next context and loop back to start if we get to the end
	for (int i = 0; i < NUM_CTX; i++) {
		int index = (search_start_pos + i) % NUM_CTX; //do this to be able to loop to start
		if (contexts[index].state == VALID) {
			next_index = (uint8_t)index;
			break;
		}
	}
	if (next_index == -1) {//if the index never changes throw error
		return -1;
	}
	//do the switch
	uint8_t old_context_index = current_context_idx;
	current_context_idx = (uint8_t)next_index;
	swapcontext(&contexts[old_context_index].context, &contexts[current_context_idx].context);
	//figure out how many other valid contexts are there left
	int32_t valid_count = 0;
	for (int i = 0; i < NUM_CTX; i++) {
		if (i != current_context_idx && contexts[i].state == VALID) {
		valid_count++;
		}
	}
	return valid_count;

}

void t_finish()
{
       	contexts[current_context_idx].state = DONE; //mark the current context as done
	if (context_stacks[current_context_idx] != NULL) { // free the stack from this context
		free(context_stacks[current_context_idx]);
		context_stacks[current_context_idx] = NULL;
	}
	int next_index = -1; //do the same index finding as in yield
	int search_start_pos = (current_context_idx + 1) % NUM_CTX;
	for (int i = 0; i < NUM_CTX; i++) {
		int index = (search_start_pos + i) % NUM_CTX;
		if (contexts[index].state == VALID) {
			next_index = index;
			break;
		}
	}
	if (next_index == -1) {
		exit(0);
	}
	//switch to the current context. we are done here.
	current_context_idx = (uint8_t)next_index;
	setcontext(&contexts[current_context_idx].context);

}
