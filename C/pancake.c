flip(stack s, int n)
{
int i;
fifo f;
   for (i = 0; i < n; i++) {
     f.push(s.pop());
   }
   for (i = 0; i < n; i++) {
     s.push(f.pop());
   }
}
pancakesort(stack s, int n)
{  
   if (n < 2) return; //paranoid.
   if (n == 2) {
     m = max(s, n);
     if (m != n) {
        flip(s, 2);
     }
     return;
   }
   m = max(s, n);
   if (m != n) {
     if (m != 1) flip(s, m);
     flip(s, n);
   }
   pancakesort(s, n-1);
}
