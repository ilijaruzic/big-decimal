#include "BigDecimal.h"

#include <iostream>
#include <ctime>

#if defined PROF_TEST
void test();
#elif defined STUDENT_TEST
void test() {
	// TODO
}
#endif

int main()
{
	auto start = std::clock();
	test();
	auto duration = (std::clock() - start) * 1000.0 / CLOCKS_PER_SEC;
	std::cout << "\nDuration: " << duration << " ms\n";
}
