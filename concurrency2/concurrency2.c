
//Jake Smith and Steven Silvers
//The Dining Philosophers Problem
//Operating systems 2


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#define Philosophers 5

// Variables for Mersenne Twister 
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

typedef struct {
        const char *name;
        int seat;
        sem_t *fork;
        sem_t *lock;
        int eat_count;
} values;


pthread_t phils[Philosophers];

//*****************************************************
//THE FOLLOWING TWO FUNCTIONS WERE PULLED FROM THE Mersenne Twister 
//by Takuji Nishimura and Makoto Matsumoto.
//*****************************************************


// initializes mt[N]
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

// generates a random number
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}



//Program Functions
void think(const char *name)
{
        srand(time(NULL));
        int think_time;

        // Random think time between 1 and 20
        think_time = genrand_int32() % 20 + 1;
        printf("%s is thinking \n", name);
        sleep(think_time);
}

void eat(values *temp)
{
        srand(time(NULL));
        int eat_time;

        printf("forks: %d and %d now in use\n",temp->seat, (temp->seat+1) % Philosophers);

        // Random eat time between 2 and 9 using Mersenne Twister
        eat_time = genrand_int32() % 7 + 2;
        printf("%s is eating \n", temp->name);
}

void get_forks(values *temp)
{
        // Wait on the table lock, and the fork to the right and left 
        sem_wait(temp->lock);
        sem_wait(&temp->fork[temp->seat]);
        sem_wait(&temp->fork[(temp->seat + 1) % Philosophers]);
}

void put_forks(values *temp)
{
        // Put down each fork, and release table lock
        sem_post(&temp->fork[temp->seat]);
        sem_post(&temp->fork[(temp->seat + 1) % Philosophers]);
        sem_post(temp->lock);
        printf("forks: %d and %d put back on table\n",temp->seat, (temp->seat+1) % Philosophers);
}

void *philosopher(void *args)
{
        values temp = *(values *)args;

        //keeps program running indefinitely
        while(1){
                think(temp.name);
                get_forks(&temp);
                eat(&temp);
                // Eat counter to prove that no philosopher is starving 
                temp.eat_count++;
                printf("%s has eaten %d times\n", temp.name, temp.eat_count);
                put_forks(&temp);
        }
}



//end functions


//main
int main()
{
        sem_t forks[Philosophers];
        sem_t lock;

        const char *philos[5];
        philos[0] = "Aristotle";
        philos[1] = "Plato";
        philos[2] = "Socrates";
        philos[3] = "Confucius";
        philos[4] = "Immanuel Kant";

        // Initialize Semaphores
        for(int i = 0; i < Philosophers; i++){
                sem_init(&forks[i], 0, 1);
        }
        
        
        //initialize lock
        sem_init(&lock, 0, 4);

        //initialize Philosophers
        printf("creating philosophers\n");
        for(int i = 0; i < Philosophers; i++){
                values *move = malloc(sizeof(values));
                move->name = philos[i];
                move->seat = i;
                move->fork = forks;
                move->lock = &lock;
                move->eat_count = 0;

                pthread_create(&phils[i], NULL, philosopher, (void *)move);
        }

        for(int i = 0; i < Philosophers; i++){
                pthread_join(phils[i], NULL);
        }

        return 0;
}



