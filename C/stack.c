int stack[1000];

int *sp;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))
...
{
    sp = stack; /* initialize */

    push(sp, 10);
    x = pop(sp);
}
