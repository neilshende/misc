package main
// #cgo CFLAGS: -g
// #include <stdio.h>
// #include "greeter.h"
import "C"

func main() {
	C.putchar('a')
	C.greet()
}
