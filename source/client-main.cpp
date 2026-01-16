#include <iostream>
#include "nlohmann/json.hpp"

int main () {
	
	std::cout << "hello-client" << "\n";
	std::cout << NLOHMANN_JSON_VERSION_MAJOR << '\n';
	return 0;

}