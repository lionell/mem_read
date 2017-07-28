#include <iostream>
#include <chrono>
#include <thread>

int main() {
	new char[8]{'l', 'i', 'o', 'n', 'e', 'l', 'l', '\0'};
	// std::cout << "Pointer: " << a << std::endl;
	// std::cout << "Size of element: " << sizeof(*a) << std::endl;
	// std::cout << "Size of array: " << sizeof(*a) * 8 << std::endl;
	int i = 0;
	while(true) {
		std::cout << i << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		i++;
	}
}
