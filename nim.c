#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
int d_flag = 0;
int array[5] = { 1, 1, 1, 1, 1};
int next[5];
int winner[][5] =  { { 1, 1, 1, 0, 0}, 
                     { 0, 2, 0, 0, 0},
                     { 0, 4, 0, 0, 0},
                     { 0, 6, 0, 0, 0},
                     { 0, 2, 0, 2, 0},
                     { 0, 2, 2, 0, 0},
                     { 1, 0, 0, 1, 1},
                     { 0, 0, 2, 0, 0},
                     { 0, 0, 0, 2, 0},
                     { 1, 1, 1, 2, 0},
                     { 1, 2, 0, 1, 1},
                     { 0, 1, 1, 1, 1},
                     { 1, 2, 0, 1, 1},
                     { 1, 0, 0, 0, 0} };

#define true 1
#define false 0
#define COPY memcpy(next, array, sizeof(array))
#define RCOPY memcpy(array, next, sizeof(array))
int valid_moves[][2] = { {5,4},
                         {5,31},
                         {5,13},
                         {5,22},
                         {5,3},
                         {5,21},
                         {5,12},
                         {5,2},
                         {5,11},
                         {5,1},
                         {5,0},
                         {4,3},
                         {4,21},
                         {4,12},
                         {4,2},
                         {4,11},
                         {4,1},
                         {4,0},
                         {3,2},
                         {3,11},
                         {3,1},
                         {3,0},
                         {2,1},
                         {2,0},
                         {1,0}};
int is_winner(int *myarray)
{
    for (int i = 0; i < sizeof(winner)/sizeof(array); i++)
    {
         if((myarray[0]&1) == winner[i][0] &&
            myarray[1]   == winner[i][1] &&
            myarray[2]   == winner[i][2] &&
            myarray[3]   == winner[i][3] &&
            myarray[4]   == winner[i][4]) return true;
    }
    return false;
}
void convert(int f, int t)
{
   RCOPY;
   printf("Convert %d to %d\n", f, t);
}
void adjust(int *my, int f, int t)
{
   if (f > 5 || f < 1) {
      printf("Invalid move\n"); exit(1);
   }
   if (my[f-1] > 0 ) {
      my[f-1]--;
   } else {
      printf("Invalid move\n"); exit(1);
   }
   while (t>0) {
      int tt = t%10;
      if (tt > 5 || tt < 1) {
         printf("Invalid move\n"); exit(1);
      }
      my[tt-1]++;
      t /= 10;
   }
}
void random_move(void)
{
   int i,f,t;
   int n = sizeof(valid_moves)/(sizeof(valid_moves[0]));
   while (1) {
   i = rand()%n;
   f = valid_moves[i][0];
   t = valid_moves[i][1];
   if (d_flag) printf("DEBUG %d %d %d\n", i, f, t);

   if (array[f-1] > 0) {
      printf("Convert %d to %d.\n", f, t);
      adjust(array, f , t);
      break;
   }
   }
/*
   for (int i=4; i>0 ; i--) {
       if (array[i] > 0) {
          printf("Convert %d to %d.\n", i+1, i);
          adjust(array, i+1, i);
          return;
       }
   }
*/
}
void next_move(void)
{
int f, t;
    for (int i = 0; i < sizeof(valid_moves)/(sizeof(valid_moves[0])); i++) {
       f = valid_moves[i][0];
       t = valid_moves[i][1];
       if (array[f-1] > 0) {
          COPY;
          adjust(next, f, t);
          if (is_winner(next)) {
             convert(f,t);
             return;
          }
        }
     }
     random_move();          
}
void print_valid_moves(void) 
{
   int i,j;
    printf("Valid moves are\n");
    for (j = 0; j < 5; j++) {
       if (array[j] == 0) continue;
       for (i = 0; i < sizeof(valid_moves)/(2*sizeof(int)); i++) {
          if (j+1 == valid_moves[i][0]) printf("%d %d\n", valid_moves[i][0], valid_moves[i][1]);
       }
    }
}
int validate_move(int c, int f, int t)
{
   int i;
   if (c==0 || c==-1) exit(0);
   if ( c != 2 || f < 1 || f > 5 ) return false;
   if (array[f-1] == 0) return false;

   for (i = 0; i < sizeof(valid_moves)/(2*sizeof(int)); i++) {
       if (f == valid_moves[i][0]  &&
           t == valid_moves[i][1]  &&
           array[f-1] > 0 ) {
           adjust(array, f, t);
           return true;
       }
    }
    return false;
}

void draw_it(void) 
{
int i; int j;
   for (j=0; j<5; j++) {
      for (i=0; i < array[j]; i++) {
          for(int k=0; k<=j; k++) {
              printf("I");
          }
          printf(" ");
      }
    printf("\n");
    }
}

void help(void)
{
   printf("Nim is a two player, alternate moves, full information, no chance moves, finite steps game.\n"
          "There are five piles of sticks to start with. Players alternately pick as many contiguous sticks\n"
          "from any one pile. The one that has to pick the last stick is the loser.\n"
          "The input to the program is two numbers. First number indicates the pile with that many sticks\n"
          "to pick from. The second number indicates the outcome of picking the sticks.\n"
          "For example 5 22 means pick the middle stick from the pile of 5 sticks leaving behind 2 piles \n"
          "of 2 sticks each.\n"
          "Another example: 3 2 means pick the first stick from pile with 3 sticks leaving behind 2 sticks\n"
          "in the pile.\n");
}

int main(int argc, char *argv[])
{
    int t, f, c;
    int f_flag = 0;
    int r_flag = 0;
    int i;
    for (i=1; i<argc; i++) {
       if (strcmp(argv[i], "--first") == 0) f_flag=1;
       else if (strcmp(argv[i], "--random") == 0) r_flag=1;
       else if (strcmp(argv[i], "--debug") == 0) d_flag=1;
       else if (strcmp(argv[i], "--help") == 0) {
           help();
           exit(0);
       }
    }
    srand(time(NULL));
    if (f_flag) {
       draw_it();
       if (r_flag) random_move(); else next_move();
    }
    while(array[0]+array[1]+array[2]+array[3]+array[4] > 0) {
    draw_it();
    if (((array[0]&1)==1) && ((array[1]+array[2]+array[3]+array[4])==0)) {
       if (array[0]==1) {
          printf("you lose.\n");
       } else {
          printf("you will certainly lose.\n");
       }
       break;
    }
    printf("your_move?");
    c=scanf("%d %d", &f, &t);
    if (!validate_move(c, f, t)) {
       print_valid_moves();
       continue;
    }
    draw_it();
    //sleep(100);
    if (((array[0]&1)==1) && ((array[1]+array[2]+array[3]+array[4])==0)) {
       printf("I concede, congrats!\n");
       break;
    }
    if (array[0]+array[1]+array[2]+array[3]+array[4] == 0) {
       printf("Thanks!\n");
       break;
    }
    if (r_flag) random_move(); else next_move();
    if (array[0]+array[1]+array[2]+array[3]+array[4] == 0) {
       printf("Oops!\n");
       break;
    }

    }
    return 0;   
}

