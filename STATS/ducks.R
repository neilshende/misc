ducks <- function(N, M, S, n, m) {
   dd=0
   x = rnorm(N, M, S)
   for (i in 1:(N-n)) {
     d =0
     for (j in 1:n) {
       if (x[i+j-1] > m) {
          d = 0
          break
       }
       d = 1
     }
     dd = dd+d
   }
   return(dd)
}


find_prob <- function(N, M, S, n, m) {
NN = 10000 #number of trials
d = replicate( NN, ducks(N, M, S, n, m))

prob = 100*sum(d>0)/length(d) #prob of n ducks in a row.

cat("innings ", N, "average ", M, "sd ", S, "prob of ", n, " cons scores under ", m, " ", prob, "%\n")
}

#start of main

N = 100   #number of innings
M = 50    #batting average
S = 50    #standard deviation
m = 1      #min score
n = 3      #numner of consecutive ducks

find_prob(N, M, S, n, m)
