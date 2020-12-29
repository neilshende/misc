x = seq(0, 100*pi, .01)  # change .01 to .1 to get sharp snow flakes.

m = 1/3
n = 2
plot(m*cos(x*n)+(1-m)*cos(x),m*sin(n*x)+(1-m)*sin(x), col="blue", type="l") #heart. 
m = 1/4
n =3
plot(m*cos(x*n)+(1-m)*cos(x),m*sin(n*x)+(1-m)*sin(x), col="blue", type="l") #apple
m = 1/4
n =3
plot(m*sin(x*n)+(1-m)*cos(x),m*cos(n*x)+(1-m)*sin(x), col="blue", type="l") #asteroid 4

#etc. n=20, m=.5 below produces a pleasant mandala. 
# but let's explore simple ones: (n<8) (m is ratio of radii of circles) 
# This represents a rolling a small circle inside a big circle.
# to simulate small circle rolling outside the big one, change (sin+cos, cos+sin) to (cos+cos, sin+sin)
for (n in 2:7) {
for (m in seq(.1,1,.1)) {
   plot(m*sin(x*n)+(1-m)*cos(x),m*cos(n*x)+(1-m)*sin(x), col="blue", type="l")
   Sys.sleep(ifelse((n==2)&&(m==.1),5,1))
}
}

library(RColorBrewer)
nc <- 60
qual_col_pals = brewer.pal.info[brewer.pal.info$category == 'qual',]
col_vector = unlist(mapply(brewer.pal, qual_col_pals$maxcolors, rownames(qual_col_pals)))

n = 20
m = 1/3
n2 = 10
m2 = 1/10
step=1/10
s=1
for (n2 in seq(0,100,step)) {
    plot(m2*cos(n2*x)+m*sin(n*x)+cos(x),m2*sin(n2*x)+m*cos(n*x)+sin(x), col=sample(col_vector, nc), type="l")
    if (n2==0) Sys.sleep(5)
    if ((n2*10) %% 10 == 0) Sys.sleep(1)
    m = m + ifelse(m<.8, s*.001, -s*.001)
    if (m >.8) s = -1 
}

#Nice border on septagon, with next
m2=.1; n2=9.8; m=.1;n=6 #
plot(m2*cos(n2*x)+m*sin(n*x)+cos(x),m2*sin(n2*x)+m*cos(n*x)+sin(x), col=sample(col_vector, nc), type="l")

#hypotrochoids
for (a in 20:30) {
   for (b in 1:10) {
     for (h in 1:10) {
        plot((a-b)*cos(x) + h*cos(((a-b)/b)*x), (a-b)*sin(x) + h*sin(((a-b)/b)*x), col="red", type="l")
        Sys.sleep(1)
     }
   }
}

#five point star
a=5; b=3; h=4
plot((a-b)*cos(x) + h*cos(((a-b)/b)*x), (a-b)*sin(x) - h*sin(((a-b)/b)*x), col="red", type="l")
#seven
a=7; b=5; h=6 #not so beautiful as next:
a=7; b=3; h=2 #seven but with 16 regions in venn diagram
#reverse lotus
a=9;b=11;h=12

