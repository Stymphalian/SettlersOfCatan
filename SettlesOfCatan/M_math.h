#pragma once

class M_math
{
public:
	static M_math& get();
	unsigned factorial(unsigned n);
	unsigned combination(unsigned n, unsigned r);
	unsigned permutation(unsigned n, unsigned r);
private:
	M_math(){};
	virtual ~M_math();
	M_math(const M_math& orig);
	void operator= (M_math const&);

};


