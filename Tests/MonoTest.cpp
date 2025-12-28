#include <Engine/Core/Misc/StrLib.hpp>
#include <dlfcn.h>
#include <cxxabi.h>
#include <iostream>

std::string get_test_name(bool (*test)()) {
	Dl_info info;
    dladdr((void*)test, &info);
    int status;
    char* demangled = abi::__cxa_demangle(info.dli_sname, 0, 0, &status);
	return (status == 0 ? demangled : info.dli_sname);
}

bool test__count_char_in_string() {
	auto result = Engine::count_char_in_string("12345c1234c123c", 'c');
	return result == 3;
};

bool test__split_string0() {
	std::vector<std::string> out;
	Engine::split_string("12345c1234c123c", 'c', out);
	if (out.size() != 3) return false;
	if (out[0] != "12345") return false;
	if (out[1] != "1234") return false;
	if (out[2] != "123") return false;
	return true; 
};

bool test__split_string1() {
	std::vector<std::string> out;
	Engine::split_string("f   34/37 36/38  29/31 27/30 ", ' ', out);
	if (out.size() != 5) return false;
	if (out[0] != "f") return false;
	if (out[1] != "34/37") return false;
	if (out[2] != "36/38") return false;
	if (out[3] != "29/31") return false;
	if (out[4] != "27/30") return false;
	return true; 
};

bool test__split_string2() {
	std::vector<std::string> out;
	Engine::split_string("34/21/52", '/', out);
	if (out.size() != 3) return false;
	if (out[0] != "34") return false;
	if (out[1] != "21") return false;
	if (out[2] != "52") return false;
	return true; 
};

bool test__split_string3() {
	std::vector<std::string> out;
	Engine::split_string("34/21", '/', out);
	if (out.size() != 2) return false;
	if (out[0] != "34") return false;
	if (out[1] != "21") return false;
	return true; 
};

bool test__split_string4() {
	std::vector<std::string> out;
	Engine::split_string("34/21", '/', out);
	if (out.size() != 2) return false;
	if (out[0] != "34") return false;
	if (out[1] != "21") return false;
	return true; 
};

bool test__split_string5() {
	std::vector<std::string> out;
	Engine::split_string("34//21", '/', out);
	if (out.size() != 2) return false;
	if (out[0] != "34") return false;
	if (out[1] != "21") return false;
	return true; 
};

bool test__split_string6() {
	std::vector<std::string> out;
	Engine::split_string("34", '/', out);
	if (out.size() != 1) return false;
	if (out[0] != "34") return false;
	return true; 
};


int main() {
	std::vector<bool (*)()> tests = {
		test__count_char_in_string,
		test__split_string0,
		test__split_string1,
		test__split_string2,
		test__split_string3,
		test__split_string4,
		test__split_string5,
		test__split_string6
	};

	int passed_test_count = 0;
	for (auto& f : tests) {
		std::cout << get_test_name(f) << ": ";
		if (f()) {
			passed_test_count++;
			std::cout << " OK\n";
		} else {
			std::cout << " Failed\n";
		}
	}
}