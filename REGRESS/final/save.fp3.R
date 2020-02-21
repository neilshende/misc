standardize = function (X) 
{
   N = nrow(X)
   D = ncol(X)
   NX = X 
   for (i in 1:D) {
      mXi = mean(X[,i])
      sXi = sd(X[,i])
      if (sXi != 0) {
         NX[,i]=(X[,i] - mXi)/sXi
      }
      #else keep the column unchanged. all 1s case.
   }   
   return(NX)
}

ln = function(x)
{
   return(log(x, base = exp(1)))
}

BIC = function(Yp, Y, n, k) 
{
  rss = sum((Yp - Y) ^ 2)
  bic = n * ln(rss / n) + k * ln(n)
  return(bic)
}

objective = function (X, Y, B)
{
   n = ncol(Y)
   s = sum ( ( Y - X %*% B ) ^ 2 )
   return (s/(2 * n))
}

supp = function (B, k)
{
   Bs = sort( abs(B), decreasing = TRUE)
   n = length(B)
   S = B
   for (i in 1:n) {
      if (abs(B[i]) < Bs[k]) {
         S[i] = 0
      } else {
         S[i] = B[i]
      }
   }
   return (S)
}

computeSupp = function (X, Y, B, k)
{
   minScore = objective(X, Y, B)
   minB = B
   m = ncol(X)
   for (i in 1 : k) {
      CB = B
      count = 0
      for (j in 1:m)  {
         if (CB[j] != 0) {
            count = count + 1
            if (count == i) {
               CB[count] = 0
               break
            }
         } 
      }
      score = objective(X, Y, CB)
      if (score < minScore) {
         minScore = score
         minB = CB
      }
   }
   return(minB)
}

GraHTP = function(X, Y, k, stepsize)
{
   iter = 30000
   nv = ncol(X)
   B = rep(0, nv)
   B = as.matrix(B)
   n = ncol(Y)
   for (t in 1:iter) {
      Yh = X%*%B
      Btd = B - (stepsize/n) * (t(X) %*% (Yh - Y)) 
      Sd = supp( Btd, k)
      # compute Bt
      Bt = computeSupp(X, Y, Sd, k)
      if ( sum ( ( Bt - B ) ^ 2 ) < 10^-4 ) {
         break
      }
      B = Bt
   }
   message ("t is ", t)
   return(B)
}

df = read.csv("hitters.csv", header=TRUE, stringsAsFactors=FALSE)
x = df[, c(-1, -20)]
y = df[ , 20]
X = as.matrix(x)
X = standardize(X)
Y = as.matrix(y)
Y = standardize(Y)
n = nrow(X)
I = rep(1, n)
p = ncol(X)
score = matrix(nrow = p, ncol =2)
minScore = 10^5
mimodel = rep(0, p)
for (k in 1:p) {
  B = GraHTP(X, Y, k, 0.001)
  score[k, 1] = k
  score[k, 2] = BIC(X%*%B, Y, n, k+1)
  if (score[k, 2] < minScore) {
     minScore = score[k, 2]
     minModel = B
  }
  message ("scores ", k, " ", score[k, 2])
}
message("The BIC selected GraHTP model score: ", minScore)
minModel
colnames(score) = c("p", "BIC score")
plot(score, main="GraHTP scores")
