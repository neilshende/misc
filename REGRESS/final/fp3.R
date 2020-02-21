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
      Sd = which.maxn(abs(Btd), k)
      Bt = lm( Y ~ 0 + X[, Sd] )
      if ( max ( abs( coef(Bt) - t(B)[Sd] ) ) < 10^-4 ) {
         break
      }
      B = rep(0, nv)
      trB = t(B)
      trB[Sd] = coef(Bt)
      B = t(trB)
   }
   return(B)
}

library(doBy)
df = read.csv("hitters.csv", header=TRUE, stringsAsFactors=FALSE)
x = df[, c(-1, -20)]
y = df[ , 20]
X = as.matrix(x)
X = standardize(X)
Y = as.matrix(y)
ym = mean(Y)
Y = Y - ym
n = nrow(X)
p = ncol(X)
score = matrix(nrow = p, ncol =2)
minScore = 10^5
mimodel = rep(0, p)
whichK = 0
for (k in 1:p) {
  B = GraHTP(X, Y, k, 0.000001)
  score[k, 1] = k
  score[k, 2] = BIC(X%*%B, Y, n, k+1)
  if (score[k, 2] < minScore) {
     minScore = score[k, 2]
     minModel = B
     whichK = k
  }
  message ("Number of Variables ", k, " BIC ", score[k, 2])
}
message("The BIC selected GraHTP model k ", whichK, " BIC: ", minScore)
rownames(minModel)=colnames(df)[c(-1,-20)]
print(minModel)
colnames(score) = c("Number of Variables", "BIC")
