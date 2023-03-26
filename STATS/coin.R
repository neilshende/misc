longestRun <- function(x, n) {
h=0
t=0
mh=0
mt=0
if (x[1]=="H") {
  h = 1
  mh = 1
  mt = 0
  t = 0
} else {
  h = 0
  mh = 0
  mt = 1
  t = 1
}
for (i in 2:n) {
   if (x[i] == x[i-1] ) {
      if (x[i] == "H" ) {
        t = t+1
        if (t > mt) {
          mt = t
        }
      } else {
        h = h+1
        if (h > mh) {
          mh = h
        }
      }
   } else {
     if (x[i] == "H" ) {
       h = 1
       t = 0
     } else {
       t = 1
       h = 0
     }
   }
}
if (mh>mt) {
   return(mh)
} else {
   return(mt)
}
}

start <- function(n, N) {
streaks=replicate(N, longestRun( sample(c("H","T"),n, 1), n))
hist(streaks)
}

n=70 # number of tosses in one test
N=1000 #number of tests
start(n, N)
