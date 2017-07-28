#include <sys/uio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <functional>
#include <cstring> // memset
#include <string>
#include <exception>

const size_t SHIFT = 72736;

size_t rand_ptr(size_t min_ptr, size_t max_ptr) {
	if (max_ptr < min_ptr) 
		throw std::logic_error("max_ptr can't be less than min_ptr");
	return min_ptr + rand() % (max_ptr - min_ptr);
}

void print_raw(char *mem, size_t size) {
	std::cout << "(";
	for (size_t i = 0; i < size - 1; i++) {
		std::cout << std::dec << int(mem[i]) << " ";
	}
	std::cout << std::dec << int(mem[size - 1]) << ")";
}

void print(char *mem, size_t size) {
	std::cout << "\"" << mem << "\"";
}

void print(int *mem, size_t size) {
	std::cout << "(";
	for (size_t i = 0; i < size - 1; i++) {
		std::cout << std::dec << mem[i] << " ";
	}
	std::cout << std::dec << mem[size - 1] << ")";
}

void read(size_t pid, size_t ptr, char *mem, size_t size) {
	iovec local[1] = {{mem, size}};
	iovec remote[1] = {{(void *) ptr, size}};
	ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
	if (nread != size)
		throw std::logic_error("Error while reading");
}

void write(size_t pid, size_t ptr, char *mem, size_t size) {
	iovec local[1] = {{mem, size}};
	iovec remote[1] = {{(void *) ptr, size}};
	ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
	if (nread != size)
		throw std::logic_error("Error while writing");
}

void rand_loop(size_t pid, size_t min_ptr, size_t max_ptr, size_t size, std::function<void(char *, size_t)> func, std::chrono::milliseconds millis) {
	while (true) {
		size_t ptr = rand_ptr(min_ptr, max_ptr - size);
		std::cout << "[" << std::hex << ptr << "]: ";
		char *mem = new char[size];
		read(pid, ptr, mem, size);
		print_raw(mem, size);
		func(mem, size);
		std::cout << "-->";
		print_raw(mem, size);
		write(pid, ptr, mem, size);
		std::cout << std::endl;
		std::this_thread::sleep_for(millis);
	}
}

void set_zero(char *mem, size_t size) {
	memset(mem, 0, size);
}

void increment(char *mem, size_t size) {
	for (size_t i = 0; i < size; i++) {
		mem[i]++;
	}
}

std::string execute(std::string cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

pid_t get_pid(std::string pname) {
	std::string res = execute("pidof " + pname);
	if (res == "") 
		throw std::logic_error("Can't find process with given name");
	return std::stoull(res);
}

size_t get_heap_begin(pid_t pid) {
	std::string res = execute("cat /proc/" + std::to_string(pid) +
			"/maps | grep heap");
	if (res == "") 
		throw std::logic_error("Can't find process heap memory page");
	size_t pos = res.find('-');
	if (pos == std::string::npos)
		throw std::logic_error("Can't find '-' in heap memory page");
	return std::stoull("0x" + res.substr(0, pos), nullptr, 16);
}

int main(int argc, char **argv) {
	srand(17);

	// rand_loop(PID, MIN, MAX, 2, set_zero, std::chrono::milliseconds(50));
	
	pid_t pid = get_pid("./main");
	std::cout << "PID: " << std::dec << pid << std::endl;
	size_t heap_begin = get_heap_begin(pid);
	std::cout << "Heap begin: " << std::hex << heap_begin << std::endl;
	std::cout << "Shift: " << std::dec << SHIFT << std::endl;
	size_t poi = heap_begin + SHIFT;
	std::cout << "Point of Interest: " << std::hex << poi << std::endl;

	char mem[8] = {};
	read(pid, poi, mem, 8);
	std::cout << "Memory read: ";
	print(mem, 8);
	return 0;
}
