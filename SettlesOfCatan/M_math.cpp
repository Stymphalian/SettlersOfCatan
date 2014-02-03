#include <cmath>
#include "Logger.h"
#include "M_math.h"

M_math::~M_math(){
	Logger::getLog().log(Logger::DEBUG, "M_math destructor");
}

M_math& M_math::get(){
	static M_math instance;
	return instance;
}

unsigned M_math::factorial(unsigned n){
	unsigned count = 0;
	while(n--){count *= n;}
	return count;
}
unsigned M_math::combination(unsigned n, unsigned r){
	return factorial(n) / (factorial(r)*factorial(n - r));
}
unsigned M_math::permutation(unsigned n, unsigned r){
	return factorial(n) / factorial(r);
}
