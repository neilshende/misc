tml = function()
{
   x = rnorm(8, 17, 3)
   m = min(x)
   M = max(x)
   if ( 17 <= M & 17 >= m) {
      return (1)
   } else {
      return (0)
   }
}

N = replicate(10000, tml())
message("The CL is ", sum(N==1)/10000)
