pidigits = read.csv("pi.csv", header=TRUE, stringsAsFactors=FALSE)
n = length(pidigits$digits)
oddind = pidigits$digits[c(seq(1, n, 2))]
evenind = pidigits$digits[c(seq(2, n, 2))]

t.test(oddind, evenind)

##y = c(oddind, evenind)
##N = rep(length(oddind), 2)
y = pidigits$digits
N = n/2
group = rep(1:2, N)
data = data.frame(y=y, group=factor(group))
###fit = lm(y ~ group, data)
###anova(fit)
summary(aov(y ~ group, data))

for (i in 1:25) {
 T = rt(10000, i)
 TT = T^2
 F = rf(10000, 1, i)
 message (i)
 message ("\t mean ", mean(TT), " ", mean(F))
 message ("\t sd " , sd(TT),  " ", sd(F))
}

tmp = tapply(y, group, stem)
tmpfn = function(x) c(sum = sum(x), mean = mean(x), var = var(x),  n = length(x))
tapply(y, group, tmpfn)
tmpfn(y)
