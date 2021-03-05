x = seq(0, 100*pi, .01)
a=11
plot(a+((a^2+1)^.5)*cos(x),((a^2+1)^.5)*sin(x), col="blue",type="l")
Sys.sleep(.1)
for ( a in 2:10) {
points(a+((a^2+1)^.5)*cos(x),((a^2+1)^.5)*sin(x), col="blue",type="l")
Sys.sleep(.1)
}
for (b in (6:16)/5) {
points((b^2-1)*cos(x), b+(b^2-1)*sin(x), col="red", type="l")
Sys.sleep(.1)
}

