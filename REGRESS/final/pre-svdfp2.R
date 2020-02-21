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

MSE = function(yp, ya) 
{
   N = nrow(yp)
   S = sum( (ya - yp)^2 )
   mse = S/N 
   return(mse)
}

ridge = function(X, Y, lambda)
{
   B = solve(t(X) %*% X + lambda * diag(nrow(t(X) %*% X))) %*% t(X) %*% Y
   return(B)
}

# Q 2.1
computeRidgeCoefficeints = function( grid, X, Y )
{
   n = length(grid)
   B = ridge(X, Y, grid[1])
   M = cbind(B)
   for (i in 2:n) {
      lambda = grid[i]
      B = ridge(X, Y, lambda)
      M = cbind(M, B)
   }
   return(M)
}

df = read.csv("hitters.csv", header=TRUE, stringsAsFactors=FALSE)
x = df[, c(-1, -20)]
y = df[ , 20]
X = as.matrix(x)
X = standardize(X)
Y = as.matrix(y)
# center Y
ym = mean(Y)
Y = Y - ym

n = nrow(X)
I = rep(1, n)
X = cbind(I, X)
m = ncol(X)

grid = 10^seq(10, -2, length=100)
M = computeRidgeCoefficeints ( grid, X, Y )

# Q 2.2
set.seed(1)
#set up the k=10 folds
folds = sample(1:10, n, replace=TRUE)
CVError = rep (0, 100)
#outer loop: for each value of lambda, calculate cross validation error.
for (i in 1:100) {
   mspe = rep(0, 10)
   #inner loop: implements the k-fold cross validation
   for (j in 1:10) {
      #pick jth fold as validation set
      Xvalidation = X[which(folds==j), ]
      #pick rest as training set
      Xtraining = X[-(which(folds==j)), ]
      Ytraining = as.matrix(Y[-(which(folds==j)), ])
      #find the ridge coef for training set
      B = ridge(Xtraining, Ytraining, grid[i])
      #apply these coef on validation set and predict salary.
      Yp = Xvalidation %*% B
      Ya = as.matrix(Y[which(folds==j)])
      #find mspe for this fold
      mspe[j] =  MSE ( Yp, Ya )
   }
   #average of k=10  mspe is the Test error for ith lambda
   CVError[i] = (1/10)*sum(mspe)
}
#we plot CVErro versus log of lambda (instead of lambda, as plot looks like upside down L)
plot(log10(grid), CVError, main="Cross-Validation Error versus log(lambda)")

# Q 2.3
selectedLambda = grid[which.min(CVError)]
selectedB = ridge(X, Y, selectedLambda)
message("Report Ridge model for selected lamda ", selectedLambda)
print(selectedB)
