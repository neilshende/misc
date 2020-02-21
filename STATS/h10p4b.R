tml = function()
{
   x1 = rexp(1, 1/12)
   if ( 12 <= x1+4 & x1 >= 0) {
      return (1)
   } else {
      return (0)
   }
}

N = replicate(10000, tml())
message("The CL is ", sum(N==1)/10000)
