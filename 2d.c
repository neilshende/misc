#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int N = 3;
int DEBUG = 0;
int BLOCK = 0;
int SLEEP = 0;
int valid_move(int p, int rx, int cx, int r, int c);
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
void (*pf[])(void) = {reset, red, yellow, blue, green};
int board[5][5];// = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int players[4][4][2];// = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int DOUBLES=0;
void dump_players() {
    pf[0]();printf("The players:\n");
    for (int i=1; i<5; i++) {
        for (int j = 0; j < 4; j++) {
            pf[i](); printf("%d,%d\t", players[i-1][j][0], players[i-1][j][1]);
        }
        printf("\n");
    }
}
static void
signal_handler(int SignalNum)
{
    dump_players();
    reset();
    printf("The answer is 42. Thanks for all the fish.\n");
    exit(0);
}
void restart(void) {
    int r,c,s,p, d;
    for (r=0; r<5; r++)
        for (c=0; c<5; c++)
            board[r][c]=0;
    for (p=0; p<4; p++)
        for (s=0; s<4; s++)
            for (d=0; d<2; d++)
                players[p][s][d]=-1;
}
void Dprintf(char *s) {
    if (DEBUG) {pf[0]();printf("%s\n",s);}
}
void print_board(void) {
    int v, p;
    int r, c, s, i;
    int X[N];
    for (i=0; i<N; i++) X[i]=0;
    reset();
    for (r=0; r<5; r++) {
        //pf[0](); printf("+-----------------+\n|");
        for (c=0; c<5; c++) {
            p = board[r][c];
            X[p-1]++;
            if (p<0 || p>4) {printf("Corruption of board.\n"); exit(0);}
            pf[p]();printf(p==0?" ":"o");
        }
        printf("\n");
        
    }
    pf[0](); printf("+-----------------+\n");
    for (i=1; i<N; i++) if (X[i]>4) {printf("Bug player count invalid\n"); dump_players(), exit(0);}
}

int is_winner(int p) {
    int i, j, k, w;
    
    // 2x2
    w=1;
    for (i=0; i<4; i++){
        for (j=0; j<4; j++) {
            if (board[i][j]==p && board[i+1][j+1]==p && board[i+1][j]==p && board[i][j+1]==p )
                return 1;
        }
    }
    
    //top columns
    
    for (i=0; i<5; i++){
        w= 1;
        for (j=0; j<4; j++) {
            w = w && (board[i][j] == p);
            if (!w) break;
        }
        if (w) return 1;
    }
    
    //bottom columns
    
    for (i=0; i<5; i++){
        w = 1;
        for (j=1; j<5; j++) {
            w = w && (board[i][j] == p);
            if (!w) break;
        }
        if (w) return 1;
    }
    // left rows
    
    for (j=0; j<5; j++){
        w = 1;
        for(i=0; i<4; i++) {
            w = w && (board[i][j] == p);
            if (!w) break;
        }
        if (w) return 1;
    }
    // right rows
    
    for (j=0; j<5; j++){
        w = 1;
        for (i=1; i<5; i++) {
            w = w && (board[i][j] == p);
            if (!w) break;
        }
        if (w) return 1;
    }
    
    //center top left
    w=1;
    for (i=1; i<5; i++) {
        w = w && (board[i][i] == p);
    }
    if (w) return 1;
    
    // center bottom right
    w=1;
    for (i=0; i<4; i++) {
        w = w && (board[i][i] == p);
    }
    if (w) return 1;
    
    //
    w=1;
    for (i=0; i<4; i++) {
        w = w && (board[i+1][i] == p);
    }
    if (w) return 1;
    
    w=1;
    for (i=0; i<4; i++) {
        w = w && (board[i][i+1] == p);
    }
    if (w) return 1;
    
    w=1;
    for (i=0; i<4; i++) {
        w = w && (board[i][3-i] == p);
    }
    if (w) return 1;
    
    w=1;
    for (i=1; i<5; i++) {
        w = w && (board[i][5-i] == p);
    }
    if (w) return 1;
    
    w=1;
    for (i=1; i<5; i++) {
        w = w && (board[i][4-i] == p);
    }
    if (w) return 1;
    
    w=1;
    for (i=0; i<4; i++) {
        w = w && (board[i][4-i] == p);
    }
    if (w) return 1;
    
    return 0;
}
int find_empty(int rx, int cx, int *rr, int *cc)
{
    int i, j;
    if (rx == -1 && cx == -1) {
        for (i=0; i<5; i++) {
            for(j=0; j<5; j++) {
                if (board[i][j] == 0) {
                    if (rand()%8!=0) continue;
                    *rr=i;
                    *cc=j;
                    return 0;
                }
            }
        }
        for (int i=0; i<5; i++) {
            for(j=0; j<5; j++) {
                if (board[i][j] == 0) {
                    *rr=i;
                    *cc=j;
                    return 0;
                }
            }
        }
        
        return 1;
    }
    if (rx == -1 && cx != -1) {printf("Bug: only rx -1\n"); exit(0);}
    if (rx != -1 && cx == -1) {printf("Bug: only cx -1\n"); exit(0);}

    if (rand()%8==0 && cx+1 < 5 && board[rx][cx+1] == 0) {*rr=rx; *cc = cx+1; return 0;}
    if (rand()%8==0 && rx+1 < 5 && board[rx+1][cx] == 0) {*rr=rx+1; *cc= cx; return 0;}
    if (rand()%8==0 && rx-1 >= 0 && board[rx-1][cx] == 0) {*rr=rx-1; *cc= cx; return 0;}
    if (rand()%8==0 && rx-1 >= 0 && cx-1 >= 0 && board[rx-1][cx-1] == 0) {*rr=rx-1; *cc = cx-1; return 0;}
    if (rand()%8==0 && cx-1 >= 0 && board[rx][cx-1] == 0) {*rr=rx; *cc = cx-1; return 0;}
    if (rand()%8==0 && rx-1 >= 0 && cx+1 < 5 && board[rx-1][cx+1] == 0) {*rr=rx-1; *cc = cx+1; return 0;}
    if (rand()%8==0 && rx+1 < 5 && cx+1 < 5 && board[rx+1][cx+1] == 0) {*rr=rx+1; *cc = cx+1; return 0;}
    if (rand()%8==0 && rx+1 < 5 && cx-1 >= 0 && board[rx+1][cx-1] == 0) {*rr=rx+1; *cc = cx-1; return 0;}
    
    if (cx+1 < 5 && board[rx][cx+1] == 0) {*rr=rx; *cc = cx+1; return 0;}
    if (rx+1 < 5 && board[rx+1][cx] == 0) {*rr=rx+1; *cc= cx; return 0;}
    if (rx-1 >= 0 && board[rx-1][cx] == 0) {*rr=rx-1; *cc= cx; return 0;}
    if (rx-1 >= 0 && cx-1 >= 0 && board[rx-1][cx-1] == 0) {*rr=rx-1; *cc = cx-1; return 0;}
    if (cx-1 >= 0 && board[rx][cx-1] == 0) {*rr=rx; *cc = cx-1; return 0;}
    if (rx-1 >= 0 && cx+1 < 5 && board[rx-1][cx+1] == 0) {*rr=rx-1; *cc = cx+1; return 0;}
    if (rx+1 < 5 && cx+1 < 5 && board[rx+1][cx+1] == 0) {*rr=rx+1; *cc = cx+1; return 0;}
    if (rx+1 < 5 && cx-1 >= 0 && board[rx+1][cx-1] == 0) {*rr=rx+1; *cc = cx-1; return 0;}
    
    //if (DEBUG) printf("Unable to find empty %d,%d, returning 1.\n", rx, cx);
    return 1;
}

int pick_winner(int p, int *rx, int *cx, int *rr, int *cc)
{
    int i, j, s, r, c;
    for (s=0; s<4; s++) {
        r = players[p-1][s][0];
        c = players[p-1][s][1];
        if (r==-1 && c==-1) {
            for (int i=0; i<5; i++) {
                for(j=0; j<5; j++) {
                    if (board[i][j] == 0) {
                        board[i][j]=p;
                        if (is_winner(p)) {
                            board[i][j]=0;
                            *rr=i;
                            *cc=j;
                            *rx=-1;
                            *cx=-1;
                            if (DEBUG) {pf[p](); printf("%d winner %d %d %d %d.\n", p, *rx, *cx, *rr, *cc);}
                            return 0;
                        } else {
                            board[i][j]=0;
                        }
                    }
                }
            }
        }
    }
    for (s=0; s<4; s++) {
        r = players[p-1][s][0];
        c = players[p-1][s][1];
        for (i= r-1; i<=r+1; i++) {
            for (j=c-1; j<=c+1; j++) {
                if (i==r && j==c) continue;
                if (i<0 || i>4) continue;
                if (j<0 || j>4) continue;
                if (board[i][j] != 0) continue;
                board[i][j]=p;
                board[r][c]=0;
                if (is_winner(p)) {
                    board[i][j]=0;
                    board[r][c]=p;
                    *rx = r;
                    *cx = c;
                    *rr = i;
                    *cc = j;
                    if (DEBUG) {pf[p](); printf("%d winner %d %d %d %d.\n", p, *rx, *cx, *rr, *cc);}
                    return 0;
                } else {
                    board[i][j]=0;
                    board[r][c]=p;
                }
            }
        }
    }
    return 1;
}

int pick_blocker(int o, int p, int *rx, int *cx, int *rr, int *cc)
{
    int i, j, s, r, c;
    int io, jo, so, ro, co;
    for (s=0; s<4; s++) {
        r = players[p-1][s][0];
        c = players[p-1][s][1];
        if (r==-1 && c==-1) {
            for (int i=0; i<5; i++) {
                for(j=0; j<5; j++) {
                    if (board[i][j] == 0) {
                        board[i][j]=p;
                        if (is_winner(p)) {
                            board[i][j]=0;
                            for (so=0; so<4; so++) {
                                ro = players[o-1][so][0];
                                co = players[o-1][so][1];
                                if (ro == -1 && co == -1) {
                                    *rx = ro;
                                    *cx = co;
                                    *rr = i;
                                    *cc = j;
                                    if (DEBUG) {pf[o](); printf("%d blocker %d %d %d %d.", o, *rx, *cx, *rr, *cc); pf[p](); printf(" %d blocked\n", p);}
                                    return 0;
                                }
                            }
                            for (so=0; so<4; so++) {
                                ro = players[o-1][so][0];
                                co = players[o-1][so][1];
                                for (io= ro-1; io<=ro+1; io++) {
                                    for (jo=co-1; jo<=co+1; jo++) {
                                        if (io==ro && jo==co) continue;
                                        if (io<0 || io>4) continue;
                                        if (jo<0 || jo>4) continue;
                                        if (board[io][jo] != 0) continue;
                                        if (io==i && jo==j) {
                                            *rx = ro;
                                            *cx = co;
                                            *rr = io;
                                            *cc = jo;
                                            if (DEBUG) {pf[o](); printf("%d blocker %d %d %d %d.", o, *rx, *cx, *rr, *cc); pf[p](); printf(" %d blocked\n", p);}
                                            return 0;
                                        }
                                    }
                                }
                            }
                        } else {
                            board[i][j]=0;
                        }
                    }
                }
            }
        }
    }
    for (s=0; s<4; s++) {
        r = players[p-1][s][0];
        c = players[p-1][s][1];
            for (i= r-1; i<=r+1; i++) {
                for (j=c-1; j<=c+1; j++) {
                    if (i==r && j==c) continue;
                    if (i<0 || i>4) continue;
                    if (j<0 || j>4) continue;
                    if (board[i][j] != 0) continue;
                    board[i][j]=p;
                    board[r][c]=0;
                    if (is_winner(p)) {
                        board[i][j]=0;
                        board[r][c]=p;
                        
                        for (so=0; so<4; so++) {
                            ro = players[o-1][so][0];
                            co = players[o-1][so][1];
                            if (ro == -1 && co == -1) {
                                *rx = ro;
                                *cx = co;
                                *rr = i;
                                *cc = j;
                                if (DEBUG) {pf[o](); printf("%d blocker %d %d %d %d.", o, *rx, *cx, *rr, *cc); pf[p](); printf(" %d blocked\n", p);}
                                return 0;
                            }
                        }
                        for (so=0; so<4; so++) {
                            ro = players[o-1][so][0];
                            co = players[o-1][so][1];
                            for (io= ro-1; io<=ro+1; io++) {
                                for (jo=co-1; jo<=co+1; jo++) {
                                    if (io==ro && jo==co) continue;
                                    if (io<0 || io>4) continue;
                                    if (jo<0 || jo>4) continue;
                                    if (board[io][jo] != 0) continue;
                                    if (io==i && jo==j) {
                                        *rx = ro;
                                        *cx = co;
                                        *rr = io;
                                        *cc = jo;
                                        if (DEBUG) {pf[o](); printf("%d blocker %d %d %d %d.", o, *rx, *cx, *rr, *cc); pf[p](); printf(" %d blocked\n", p);}
                                        return 0;
                                    }
                                }
                            }
                        }
                        
                    } else {
                        board[i][j]=0;
                        board[r][c]=p;
                    }
                }
            }
    }
    return 1;
}

int compute(int p, int *rx, int *cx, int *rr, int *cc)
{
    int i, count;
    
    
    // find winner.
    if (pick_winner(p, rx, cx, rr, cc) == 0) {
        if (!valid_move(p, *rx, *cx, *rr, *cc)) {pf[p](); printf("Bug in pick winner. %d,%d - %d, %d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
        return 0;
    }
    
    
    //find blocker.
    if (pick_blocker(p, next(p), rx, cx, rr, cc) == 0) {
        if (!valid_move(p, *rx, *cx, *rr, *cc)) {pf[p](); printf("Bug in pick blocker1. %d,%d - %d, %d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
        return 0;
    }
    if (N>=3 && pick_blocker(p, next(next(p)), rx, cx, rr, cc) == 0) {
        if (!valid_move(p, *rx, *cx, *rr, *cc)) {pf[p](); printf("Bug in pick blocker2. %d,%d - %d, %d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
        return 0;
    }
    if (N==4 && pick_blocker(p, next(next(next(p))), rx, cx, rr, cc) == 0) {
        if (!valid_move(p, *rx, *cx, *rr, *cc)) {pf[p](); printf("Bug in pick blocker3. %d,%d - %d, %d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
        return 0;
    }
    
    // initial moves to random empty slots to make it interesting.
    for (i=0; i<4; i++) {
        if (players[p-1][i][0] == -1 && players[p-1][i][1] == -1) {
            *rx = -1;
            *cx = -1;
            if (find_empty(-1, -1, rr, cc)) {pf[p](); printf("BUG -- cant find initial empty slot\n"); exit(0);}
            if (!valid_move(p, -1, -1, *rr, *cc)) {pf[p](); printf("Bug in pick initial empty. %d,%d - %d, %d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
            return 0;
        }
    }
    
    //find legal empty slot randomly
    i =rand()%4;
    count = 0;
    int n;
    int skip;
    int SKIP=0;
    while (count < 4) {
        skip =0;
        *rx=players[p-1][i][0];
        *cx=players[p-1][i][1];
        if (find_empty(*rx, *cx, rr, cc) == 0) {
            if (!valid_move(p, *rx, *cx, *rr, *cc)) {pf[p](); printf("Bug in pick empty. %d,%d - %d,%d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
            if (count != 3) {
                for (n=next(p); n!=p; n=next(n)) {
                    // can n actually move fourth piece to rx,cx? FIXME
                    board[*rx][*cx] = n;
                    if (is_winner(n)) {
                        skip =1;
                        SKIP =1;
                        break;
                    }
                }
            }
            board[*rx][*cx] = p;
            if (skip) {
                printf("Avoiding unblock %d,%d - %d,%d ", *rx, *cx, *rr, *cc);
                pf[n](); printf("for %d\n", n); pf[p]();
                count++;
                i = (i==3?0:i+1);
                continue;
            }
            return 0;
        }
        i = (i==3?0:i+1);
        count++;
    }
    if (SKIP) {
        i =rand()%4;
        count=0;
        while (count < 4) {
            *rx=players[p-1][i][0];
            *cx=players[p-1][i][1];
            if (find_empty(*rx, *cx, rr, cc) == 0) {
                if (!valid_move(p, *rx, *cx, *rr, *cc)) {pf[p](); printf("Bug in pick empty. %d,%d - %d,%d\n", *rx, *cx, *rr, *cc); dump_players(); exit(0);}
                return 0;
            }
            i = (i==3?0:i+1);
            count++;
        }
    }
    pf[p](); printf("Unable to find valid move: count(%d),i(%d)\n", count, i);
    dump_players();
    //if (DEBUG) exit(0);
    return 1;
}
void help(void) {
    pf[0]();
    printf("The default is three players. --two or --four to change that.\n");
    printf("Some or all of the players can be the computer. --comp1/2/3/4/all to pick which player is/are computer(s).\n");
    printf(" --comp1 --comp2 --comp3 --comp4 --compall --two --four --debug --help  : are valid options.\n");
    exit(0);
}
void error(int n) {
    printf("invalid option.\n");
    printf(" --comp1 --comp2 --comp3 --comp4 --compall --four --two --debug --help  : are valid options.\n");
    exit(n);
}

int valid_move(int p, int rx, int cx, int r, int c) {
    if (p<1 || p>4) {
        printf("invalid p %d\n", p);
        return 0;
    }
    if (rx==r && cx==c) {
        printf("source %d,%d and dest %d,%d same.", rx, cx, r, c);
        return 0;
    }
    if (rx != -1 && cx != -1 && board[rx][cx] != p) {
        printf("source %d,%d not occupied by %d.\n", rx, cx, p);
        return 0;
    }
    if (r<0 || r>4 || c<0 || c>4 ||board[r][c] != 0) {
        printf("destination %d,%d not valid or empty.\n", r, c);
        return 0;
    }
    for (int x=0; x<4; x++) {
        if (players[p-1][x][0] == rx && players[p-1][x][1] == cx) {
            return 1;
        }
    }
    printf("source %d,%d not valid.\n", rx, cx);
    return 0;
}

void execute_move(int p, int rx, int cx, int r, int c) {
    int i;
    if (rx != -1 && cx != -1  ) {
        if (board[rx][cx] == p)
           board[rx][cx] = 0;
        else {
           printf("BUG - source %d,%d not valid.\n", rx, cx);
           dump_players();
           exit(0);
        }
    }
    if (board[r][c] == 0) board[r][c] = p;
    else {printf("BUG - dest %d,%d not valid.\n", r, c); dump_players(); exit(0);}
    for (i=0; i<4; i++) {
        if (players[p-1][i][0] == rx && players[p-1][i][1]== cx) {
            players[p-1][i][0] = r;
            players[p-1][i][1] = c;
            break;
        }
    }
    if (i == 5) {printf("BUG - source %d,%d not found.\n", rx, cx); dump_players(); exit(0);}
    return;
}

int main(int argc, char *argv[])
{
    int i, j, k, p;
    long long m;
    int r, c, s, rx, cx;
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
        else if (strcmp(argv[i], "--four") == 0) N=4;
        else if (strcmp(argv[i], "--two") == 0) N=2;
        else if (strcmp(argv[i], "--help") == 0) help();
        else if (strcmp(argv[i], "--debug") ==0) DEBUG=1;
        else if (strcmp(argv[i], "--compall") == 0) {LOOP=1;
            for(int l=1; l<5; l++) comp[l]=l;  }
        else if (strcmp(argv[i], "--block") == 0) BLOCK=1;
        else if (strcmp(argv[i], "--sleep") == 0) SLEEP=1;
        else error(0);
    }
    
    restart();
    for (m=0; ; m++) {
        print_board();
        p = (m%N)+1;
        pf[p]();
        if (comp[p] == p) {
            printf("%d Thinking...\n", p);
            if (SLEEP) sleep(1);
            if (compute(p, &rx, &cx, &r, &c)) {continue;}
            if (!valid_move(p, rx, cx, r, c)) {
                pf[p]();
                printf("Bad compute2. %d %d %d %d.\n", rx, cx, r, c);
                dump_players();
                pf[0]();
                exit(0);
            }
        } else {
        again:
            printf("Move for player %d (four intgers between 0..4) : ", p);
            rx = cx = r = c = -1;
            int ret=scanf("%d %d %d %d", &rx, &cx, &r, &c);
            if (ret != 4) {
                char ch;
                while ((ch=getchar())!='\n');
                printf("input four integers for row column and size between 0 and 4.\n");
                printf("OR following for changing control:\n");
                printf("-1 0 0 0 for asking computer to make move.\n");
                printf("-2 0 0 0 for asking computer to take over this player.\n");
                printf("-3 0 0 0 to quit.\n");
                goto again;
            }
            if (rx < 0 && cx >= 0) {
                if ( rx == -3) break;
                if (rx == -2) comp[p]=p; // let computer handle now on.
                if (compute(p, &rx, &cx, &r, &c)) {continue;}
                if (!valid_move(p, rx, cx, r, c)) {
                    pf[p]();
                    printf("Bad compute2. %d %d %d %d.\n", rx, cx, r, c);
                    dump_players();
                    pf[0](); exit(0);
                }
            } else if (!valid_move(p, rx, cx, r, c)) goto again;
        }
        execute_move(p, rx, cx, r, c); //on error exits.
        if ( is_winner(p)) {
                printf("Player %d won. %lld steps\n", p, m); break;
        }
    }
    print_board();
    if (DEBUG) dump_players();
    reset();
    return 0;
}
