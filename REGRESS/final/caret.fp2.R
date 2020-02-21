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
#SHOULD WE center Y
#Q2 just says "standardize the variables"
#ym = mean(Y)
#Y = Y - ym

n = nrow(X)
I = rep(1, n)
X = cbind(I, X)
m = ncol(X)

grid = 10^seq(10, -2, length=100)
M = computeRidgeCoefficeints ( grid, X, Y )

#library(glmnet)
#set.seed(1)
#result = cv.glmnet(X[, -1], Y, alpha=0, lambda = grid, nfolds = 10, thresh = 1e-10)
#plot(result)
#selectedLambda = result$lambda.min

library(caret)
set.seed(1)
a = seq(1, n)
folds = createFolds(a, k = 10, list = FALSE, returnTrain = FALSE)
#folds = sample(1:10, n, replace=TRUE)
CVError = rep (0, 100)
for (j in 1:100) {
   mspe = rep(0, 10)
   for (i in 1:10) {
      Xvalidation = X[which(folds==i), ]
      Xtraining = X[-(which(folds==i)), ]
      Ytraining = as.matrix(Y[-(which(folds==i)), ])
      B = ridge(Xtraining, Ytraining, grid[j])
      Yp = Xvalidation %*% B
      Ya = as.matrix(Y[which(folds==i)])
      mspe[i] =  MSE ( Yp, Ya )
   }
   CVError[j] = (1/10)*sum(mspe)
}
plot(log10(grid), CVError)

selectedLambda = grid[which.min(CVError)]
selectedB = ridge(X, Y, selectedLambda)
message("Report Ridge model for selected lamda ", selectedLambda)
print(selectedB)
