#include <iostream>
class fsg {
	public:
		fsg() {
			a=b=1;
		}
		~fsg() {}
		long operator() () {
			auto r=a;
			a=b;
			b=r+b;
			return r;
		}
	private:
		long a, b;
};

long FibGen() {
  static long a=1;
  static long b=1;
  auto r = a;
  a=b;
  b=r+b;
  return r;
}

long Fibonacci(int n) {
   if (n<1 || n>92) return -1;
   fsg F;
   long r;
   while(n--) {
     r = F();
   }
   return r;
}
int main() {
	fsg FS;
	int i = 1;
	for (auto f = FS(); f>0; f = FS()) {
		std::cout << i++  << " : " << f << std::endl;
	}
	i = 1;
	for (auto f = FibGen(); f>0; f = FibGen()) {
		std::cout << i++  << " : " << f << std::endl;
	}
	std::cout << "91 : " << Fibonacci(91) << std::endl;
	return 0;
}
