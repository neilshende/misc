#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int N = 4;
int DEBUG = 0;
int BLOCK = 0;

int diff (int p, int o) {
   if (p==o) {
      if (DEBUG) printf("Possible bug p==o==%d.\n", p);
   }
   return(p>=o ? (p-o) : (p+N-o));
}

int prev(int p) {
   return(p==1 ? N : p-1);
}

int next(int p) {
   return(p==N ? 1 : p+1);
}

void red (void) {
  printf("\033[1;31m");
}

void yellow (void) {
  printf("\033[1;33m");
}

void blue (void) {
  printf("\033[1;34m");
}

void green (void) {
  printf("\033[1;32m");
}

void reset (void) {
  printf("\033[0m");
}

static void
signal_handler(int SignalNum)
{
   reset();
   printf("The answer is 42. Thanks for all the fish.\n");
   exit(0);
}
void (*pf[])(void) = {reset, red, yellow, blue, green};
int board[3][3][3] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int players[4][3] = {3,3,3,3,3,3,3,3,3,3,3,3};
int DOUBLES=0;
void restart(void) {
   int r,c,s,p;
   for (r=0; r<3; r++)
     for (c=0; c<3; c++)
        for (s=0; s<3; s++)
            board[r][c][s]=0;
   for (p=0; p<4; p++)
      for (s=0; s<3; s++)
          players[p][s]=3;    
}
void Dprintf(char *s) {
   if (DEBUG) {pf[0]();printf("%s\n",s);}
}
void print_board(void) {
int v;
int r, c, s;
reset();
for (r=0; r<3; r++) {
  for (c=0; c<3; c++) {
     v= board[r][c][0];
     if (v) { pf[v](); printf("s"); } else { printf(" ");}
     v= board[r][c][1];
     if (v) { pf[v](); printf("m"); } else { printf(" ");}
     v= board[r][c][2];
     if (v) { pf[v](); printf("L  "); } else { printf("   ");}
  }
  printf("\n");
}
}

int is_winner(int p) {
int r, c, s;
// each cell fully occupied.
for (r = 0; r < 3; r++) {
   for (c = 0; c< 3; c++) {
      if (board[r][c][0] == p && board[r][c][1] == p && board[r][c][2] == p) return 1;
   }
}
// each row with same size
for (r = 0; r<3; r++) {
  for (s=0; s<3; s++) {
    if (board[r][0][s] == p && board[r][1][s] == p && board[r][2][s] == p) return 1;
  }
}

//each column with same size
for (c = 0; c<3; c++) {
  for (s=0; s<3; s++) {
    if (board[0][c][s] == p && board[1][c][s] == p && board[2][c][s] == p) return 1;
  }
}

// diagonals with same size
for (s=0; s<3; s++) {
   if (board[0][0][s] == p && board[1][1][s] == p && board[2][2][s] == p) return 1;
   if (board[2][0][s] == p && board[1][1][s] == p && board[0][2][s] == p) return 1;
}

//diagonals diff  size
if (board[0][0][0] == p && board[1][1][1] == p && board[2][2][2] == p) return 1;
if (board[2][0][0] == p && board[1][1][1] == p && board[0][2][2] == p) return 1;
if (board[0][0][2] == p && board[1][1][1] == p && board[2][2][0] == p) return 1;
if (board[2][0][2] == p && board[1][1][1] == p && board[0][2][0] == p) return 1;

// rows diff size
for (r=0; r<3; r++) {
   if (board[r][0][0] == p && board[r][1][1] == p && board[r][2][2] == p) return 1;
   if (board[r][0][2] == p && board[r][1][1] == p && board[r][2][0] == p) return 1;
}

// cols diff size
for (c=0; c<3; c++) {
   if (board[0][c][0] == p && board[1][c][1] == p && board[2][c][2] == p) return 1;
   if (board[0][c][2] == p && board[1][c][1] == p && board[2][c][0] == p) return 1;
}
return 0;
}
int compute(int p, int *rr, int *cc, int *ss)
{
int ignore;
int r, c, s;
int o =p;
   for (r=0; r<3; r++) {
      for (c=0; c<3; c++) {
         for(s=0; s<3; s++) {
            if (board[r][c][s] == 0 && players[p-1][s] > 0) {
               board[r][c][s] = p;
               if (is_winner(p)) {
                   board[r][c][s] = 0;
                   *rr=r; *cc=c; *ss=s;
                   return 0;
               } else {
                  board[r][c][s] = 0;
               } 
            } 
        }
      }
    }
int hack_count=0;
hack:
   ignore = 1;
   hack_count++;
   p = next(p);
   if (DOUBLES && hack_count==2) {p = next(p); hack_count++;}

   for (r=0; r<3; r++) {
      for (c=0; c<3; c++) {
         for(s=0; s<3; s++) {
            if (board[r][c][s] == 0 && players[p-1][s] > 0 && players[o-1][s] > 0) {
               board[r][c][s] = p;
               if (is_winner(p)) {
                   board[r][c][s] = 0;
                   if (!BLOCK && ignore && diff(p,o) > 1 && players[prev(p)-1][s] > 0) {
                       if (DEBUG) {
                           printf("%d ignoring position %d %d %d for ", o, r, c, s);
                           pf[p](); printf("player %d\n", p);
                           pf[o]();
                       }
                       ignore = 0;
                       continue;
                   }
                   if (DEBUG) {
                       printf("%d blocking position %d %d %d for ", o, r, c, s);
                       pf[p](); printf("player %d\n", p);
                       pf[o]();
                   }
                   *rr=r; *cc=c; *ss=s;
                   return 0;
               } else {
                  board[r][c][s] = 0;
               } 
            } 
         }
      }
   }
if (hack_count<N-1) goto hack;
 

if (board[1][1][1] == 0 && players[o-1][1] > 0 && (rand()&0x1)) {
   *rr=*cc=*ss=1;
   return 0;
}
for (r=0; r<3; r+=2) {
  for (c=0; c<3; c+=2) {
     for (s=0; s<3; s+=2) {
        if (board[r][c][s] == 0 && players[o-1][s] > 0) {
           if (s==0 && players[o-1][0] > players[o-1][2]) continue;
           if (rand()&0x1) continue;
           *rr=r; *cc=c; *ss=s;
           return 0;
        }
      }
   }
}
for (r=0; r<3; r++) {
   for (c=0; c<3; c++) {
      if ((r+c)&0x1) {
         if (board[r][c][1] == 0 && players[o-1][1] > 0) {
            if (rand()&0x1) continue;
            *rr=r; *cc=c; *ss=1;
            return 0;
         }
       }
   }
}

   for (r=0; r<3; r++) {
      for (c=0; c<3; c++) {
         for(s=0; s<3; s++) {
            if (board[r][c][s] == 0 && players[o-1][s] > 0) {
                   *rr=r; *cc=c; *ss=s;
                   return 0; 
            } 
        }
      }
    }
    pf[o](); printf("Trouble. no valid move.\n");
    return 1;

}
void help(void) {
   pf[0]();
   printf("3d tic tac toe or otrio is played by four or three players individually or four players in doubles.\n");
   printf("The default is four players individually. --three or --doubles to change that.\n");
   printf("Some or all of the players can be the computer. --comp[1]/2/3/4 to pick which player is/are computer(s).\n"); 
   printf(" --comp[1] --comp[2] --comp[3] --comp[4] --three --doubles --loop --block --help  : are valid options.\n");
   printf(" --three and --doubles are mutually exclusive.\n");
   printf(" --loop runs the program in a loop until there is a result.\n");
   printf(" --block strategy option to always block if possible.\n");
   exit(0);
}
void error(int n) {
   printf("invalid option.\n");
   printf(" --comp[1] --comp[2] --comp[3] --comp[4] --three --doubles --loop --block --help  : are valid options.\n");
   printf(" --three and --doubles are mutually exclusive.\n");
   exit(n);
}
 
int main(int argc, char *argv[])
{
   int i, j, k, m, p;
   int r, c, s;
   int M = 27;
   long long LOOP = 0;
   int comp[5] = { 0, 0, 0, 0, 0};

   struct sigaction sigact;
   int  err;

   srand(time(NULL));

   memset(&sigact, 0, sizeof (sigact));
   sigact.sa_handler = &signal_handler;
   err = sigaction(SIGTERM, &sigact, NULL);
   err = sigaction(SIGINT, &sigact, NULL);

    for (i=1; i<argc; i++) {
       if (strcmp(argv[i], "--comp1") == 0) comp[1]=1;
       else if (strcmp(argv[i], "--comp2") == 0) comp[2]=2;
       else if (strcmp(argv[i], "--comp3") == 0) comp[3]=3;
       else if (strcmp(argv[i], "--comp4") == 0) comp[4]=4;
       else if (strcmp(argv[i], "--three") == 0) N=3;
       else if (strcmp(argv[i], "--doubles") == 0) {BLOCK=1; DOUBLES=1;}
       else if (strcmp(argv[i], "--help") == 0) help();
       else if (strcmp(argv[i], "--debug") ==0) DEBUG=1;
       else if (strcmp(argv[i], "--loop") == 0) {LOOP=1;
             for(int l=1; l<5; l++) comp[l]=l;  }
       else if (strcmp(argv[i], "--block") == 0) BLOCK=1;
       else error(0); 
    }
   if (! ((comp[1] && comp[3] && (!comp[2]) && (!comp[4]) ) || ((!comp[1]) && (!comp[3]) && comp[2] && comp[4]) )) {
      if (DOUBLES) {pf[0](); printf("FYI generally in --doubles, only two odd or even players should be selected.\n");}
      //DOUBLES=0;
   }
   if (DOUBLES && N==3) {pf[0](); printf("Ignoring --doubles, can not be used with --three.\n"); DOUBLES=0;}
loop:
   M = 27;
   for (m=0; m<M; m++) {
      print_board();
      p = (m%N)+1;
      pf[p]();
      if (comp[p] == p) {
         printf("%d Thinking...\n", p);
         if (!LOOP) sleep(1);
         if (compute(p, &r, &c, &s)) { M++; continue;}
      } else {
         again:
         printf("Move for player (three intgers between 0..2) %d: ", p);
         r = c = s = -4;
         int ret=scanf("%d %d %d", &r, &c, &s);
         if (ret != 3) {
           char ch;
           while ((ch=getchar())!='\n');
           printf("input three integers for row column and size between 0 and 2.\n");
           printf("OR following for changing control:\n");
           printf("-1 0 0 for asking computer to make move.\n");
           printf("-2 0 0 for asking computer to take over this player.\n");
           printf("-3 0 0 to quit.\n");
           goto again;
         }
         if (r < 0) {
            if ( r == -3) break;
            if (r == -2) comp[p]=p; // let computer handle now on.
            if (compute(p, &r, &c, &s)) {M++; continue;}
         } else if (r>=0 && r <=2 && c>=0 && c <= 2 && s >= 0 && s <= 2) {
            if (board[r][c][s] != 0) goto again;
            if (players[p-1][s] == 0) goto again;
         } else {
           goto again;
         }
      }
      board[r][c][s] = p;
      --players[p-1][s];
      if ( is_winner(p)) { 
         if (LOOP) {
           printf("Player %d won.\n", p);
           print_board();
           reset();
           printf("-------------------------%lld\n", LOOP);
           exit(0);
         } else {
           printf("Player %d won.\n", p); break;
         }
      }
   }
   print_board();
   reset();
   if (LOOP) {
      printf("-------------------------%lld\n", LOOP);
      LOOP++;
      restart();
      goto loop;
   }
   return 0;
}

