gr35 = function()
{
   x = rexp(35, 13)
   m = mean (x)
   return (1/m)
}
sam = replicate(10000, gr35())
hist(sam, 20)

