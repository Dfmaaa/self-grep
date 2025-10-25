/* An attempt at making something similar to grep. Will use multithreading
   to look for words even as blocks are being filled.
   Locks will be used to make sure there are no race conditions.
   i.e finder thread reaches a block that hasn't been filled completely.
   For that purpose, 1 byte int has been added as field to struct 
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdatomic.h>

#define BLOCK_SIZE 1001 // 1 KB, 1 extra byte for \o

// unidirectional ordered block linked list
struct block{

    int8_t block_complete;

    char *buffer;

    struct block *next;

};

struct find_arg{

    struct block *head;
    char *target;
    atomic_int *eflag;

};

struct rect_arg{

    struct block *head;
    atomic_int *eflag; // for telling finder thread that linked list is complete.

};


struct block* _init_block(){

    struct block *b_ptr = (struct block *)malloc(sizeof(struct block));

    b_ptr->block_complete = 0;
    b_ptr->next = NULL;
    // buffer to be filled by function caller

    return b_ptr;

}



void _add_block(struct block *head, char *buffer){

    struct block *curr = head;

    while (curr->next!=NULL){

        curr = curr->next;

    }

    curr->next = _init_block();
    curr->next->buffer = buffer;

    curr->next->next = NULL;

    curr->next->block_complete = 1;


}

void _cleanup(struct block *head){

    struct block *curr = head;

    while( curr->next != NULL){

        struct block *save = curr;

        free(curr->buffer);
        curr = curr->next;
        free(save);



    }

    free(curr->buffer);
    free(curr);


}

void *__rect(void *arg){

    struct rect_arg *args = (struct rect_arg*)arg;

    struct block *head = args->head;

    fgets(head->buffer,BLOCK_SIZE,stdin); // first read, filling head.

    char *_fbuff = (char*)malloc(sizeof(char)*BLOCK_SIZE);

    while(fgets(_fbuff,BLOCK_SIZE,stdin)!=NULL){

        _add_block(head,_fbuff);

    }

    return NULL;
    
}


/*goes 'forward' characters ahead in linked list, takes care of race conditions 
 will change value of flag to 1, if end is reached, then it'll return a random character.
*/
char _trv_blocks(struct block *curr, int c, int forward, int *flag, atomic_int *eflag){

    struct block *trv=curr;
    int curr_cnter=c; // curr->buffer[curr_cnter] is current character
    int left = forward; 
    int _nbuff = strlen(curr->buffer); // this will change for final block.

    while(left>=0){

        // best case, final ending case.
        if(curr_cnter+left<_nbuff){

                return curr->buffer[curr_cnter+left];

        }

        // most likely case, block change.
        if(curr_cnter+left>=_nbuff){

            left -= (_nbuff - (curr_cnter + 1)) -1;

            if(curr->next == NULL){

                if(*eflag == 1){

                    *flag = 1; // end reached, cant go any further.

                    return '\t'; // returning random character that wont be processed
                }
                
                else{

                    while(curr->next==NULL || *eflag == 0){} // waiting

                    continue;


                }
                

            }
            else{

                while(curr->next->block_complete!=1){} // waiting
                curr=curr->next;
                curr_cnter=0;
                _nbuff =strlen(curr->buffer); // will only be useful in final block
                continue;



            }

        }

    }

    /* If this section of code is being accessed, end of block list has been 
        reached, this shouldn't be executed but still, returning random character,
        setting flag to 1.*/

    *flag=1;
    return '\t';




}

void *__find(void *arg){

    struct find_arg *args = (struct find_arg*)arg;
    char *targ = args->target;
    int inst_cnt = 0;
    int targ_len = strlen(targ);
    int flag = 0;
    struct block *curr  = args->head;
    int block_no=0;

    // can put 1 here but, just to be sure
    while(!flag) {

       while(curr->block_complete == 0){} // waiting
       int _bufl=strlen(curr->buffer); 

       for(int i = 0; i < _bufl ; i++){

            if(curr->buffer[i] == targ[0]){

                if(_trv_blocks(curr,i,targ_len-1,&flag,args->eflag) == targ[targ_len-1]){

                    for(int j = 1; i<targ_len-1;j++){

                        if(_trv_blocks(curr,i,j,&flag,args->eflag) != targ[j]){

                            continue; // not a match

                        }

                    }

                    // if this section is reached, word is a match.
                    printf("Total matches: %d, found in block: %d.", inst_cnt+1, block_no+1);
                    inst_cnt++;




                }

                if(flag == 1){

                    return NULL; 


                }

            }


       }

       if(curr->next == NULL && *(args->eflag) == 1){
            
            return NULL;

       }

       else{

            curr = curr->next;
            block_no++;

       }





    }




}

int main(int argc, char **argv){

    if( argc <= 1 ){

        printf("Usage: [command that prints output] | %s [text to find]\n",argv[0]);
        return -1;

    }

    else{

        atomic_int end_f = 0;
        struct block *dhead=_init_block();
        struct find_arg *farg=(struct find_arg*)malloc(sizeof(struct find_arg));
        struct rect_arg *rarg=(struct rect_arg*)malloc(sizeof(struct rect_arg));

        farg->head = dhead;
        farg->target = argv[1];
        farg->eflag = &end_f;

        rarg->head=dhead;
        rarg->eflag=&end_f;

        pthread_t rec_t,find_t;

        pthread_create(&rec_t, NULL, __rect, (void*)rarg);
        pthread_create(&find_t,NULL,__find,(void*)farg);

        pthread_join(rec_t,NULL);
        pthread_join(find_t,NULL);








    }

    return 0;


}