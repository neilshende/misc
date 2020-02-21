def p44(L, nf, ns):
	L+= ns
	f = [0]*L
	for n in xrange(2, L):
		if f[n] == nf:
			c+= 1
			if c == ns:
				print n-ns+1
				c-= 1
		else:
			c = 0
			if f[n] == 0: f[n::n] = [x+1 for x in f[n::n]]
	return
p44(300000,4,4)
