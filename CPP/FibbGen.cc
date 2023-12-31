#include <iostream>
class fsg {
	public:
		fsg() {
			a=b=1;
		}
		~fsg() {}
		long next() {
			auto r=a;
			a=b;
			b=a+b;
			return r;
		}
	private:
		long a, b;
};

long FibGen() {
  static long a=1;
  static long b=1;
  auto ret = a;
  a=b;
  b=a+b;
  return ret;
}
int main() {
	fsg FS;
	for (auto f = FS.next(); f>0; f = FS.next()) {
		std::cout << f << std::endl;
	}
	for (auto f = FibGen(); f>0; f = FibGen()) {
		std::cout << f << std::endl;
	}
	return 0;
}
