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

GDS = function(X, Y)
{
   step = .001 
   iter = 30000
   nv = ncol(X)
   B = rep(0, nv)
   B = as.matrix(B)
   n = ncol(Y)
   for (i in 1:iter) {
      B = B - (step/n) * (t(X) %*% (X%*%B - Y)) 
      if (max(abs(X%*%B - Y)) < 10^-5) break
   }
   return(B)
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

df = read.csv("hitters.csv", header=TRUE, stringsAsFactors=FALSE)
x = df[, c(-1, -20)]
y = df[ , 20]
X = as.matrix(x)
X = standardize(X)
Y = as.matrix(y)
n = nrow(X)
I = rep(1, n)
m = ncol(X)
FSminscore = 10^15
FSmodel = rep(0,m)
FSmodelX = X
p = matrix(nrow=m, ncol=2)
selected = rep(0, m)
H = cbind(I)
for (i in 1:m) {
   score = rep(10^15, m)
   Hb = H
   for (j in 1:m) {
      if (selected[j] == 0) {
         H = cbind( H, X[, j])
         B = GDS(H, Y)
         score[j] = BIC(H%*%B, Y, n, 1+ncol(H))
         H = Hb
      }
   }  
   k = which.min(score)
   H = cbind(H, X[, k])
   colnames(H)[i+1] = colnames(X)[k]
   selected[k] = 1
   #message ("FS i k score ", i, " ", k, " ", score[k])
   if (score[k] < FSminscore) {
      FSminscore = score[k]
      FSmodelX = H
      FSmodel = GDS(H, Y)
   }
   message("Number of variables: ", i, " BIC: ", score[k])
   p[i,1] = i
   p[i,2] = score[k]  
}
message("Null Forward Step: minimum score ", FSminscore)
plot(p, xlab="Number of variables", ylab="BIC", main="Null forward step")
FSmodel

H = cbind(I, X)
B = GDS(H, Y)
BSminscore = 10^15
BSmodel = B
BSmodelX = H
q = matrix(nrow=m, ncol=2)
q[1,1] = 19
q[1,2] = BIC(H%*%B, Y, n, 1+ncol(H))
message("Number of variables: ", q[1,1], " BIC: ", q[1,2])
removed = rep(0, m+1)
for (i in 2:m) {
   Hncol = ncol(H)
   score = rep(10^15, Hncol)
   Hb = H
   for (j in 2:Hncol) {
      H = H[, -j]
      B = GDS(H, Y)
      score[j] = BIC(H%*%B, Y, n, 1+ncol(H))
      H = Hb
   }  
   k = which.min(score)
   H = H[, -k]
   #message ("BS i k score ", i, " ", k, " ", score[k])
   if (score[k] <= BSminscore) {
      BSminscore = score[k]
      BSmodelX = H
      BSmodel = GDS(H, Y)
   }
   message("Number of variables: ", 19-i+1, " BIC: ", score[k])
   q[i,1] = 19-i+1
   q[i,2] = score[k] 
}
message("Full Backward Step: minimum score ", BSminscore)
plot(q, xlab="Number of variables", ylab="BIC", main="Full backward step")
BSmodel
