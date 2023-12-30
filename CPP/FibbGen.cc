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
int main() {
	fsg FS;

	for (auto f = FS.next() ; f>0 ; f = FS.next()) {
		std::cout << f << std::endl;
	}
	return 0;
}
