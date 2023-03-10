#ifndef _BIG_DECIMAL_H_
#define _BIG_DECIMAL_H_

#include <iostream>

class BigDecimal
{
public:
	BigDecimal();
	BigDecimal(int num);
	BigDecimal(const char* num);
	BigDecimal(char sgn, const char* digits, int digitsCnt, unsigned wholeDigitsCnt);
	BigDecimal(const BigDecimal& num);
	BigDecimal(BigDecimal&& num) noexcept;
	~BigDecimal();

	BigDecimal& operator=(const BigDecimal&) = delete;

	BigDecimal shl(int n) const;
	BigDecimal shr(int n) const;
	BigDecimal rmd(int* n) const;

	BigDecimal add(const BigDecimal* num) const;
	BigDecimal sub(const BigDecimal* num) const;

	bool greater(const BigDecimal* num) const;
	bool less(const BigDecimal* num) const;
	bool equals(const BigDecimal* num) const;

	BigDecimal abs() const;

	// TODO
	// operators overloading

	friend std::ostream& operator<<(std::ostream& os, const BigDecimal& num);

private:
	char sgn;
	char* digits;
	int digitsCnt, wholeDigitsCnt;

	static void numToDigits(char* dst, int len, int num);
	static int digitsToNum(const char* src, int len);

	static void copyDigits(char* dst, const char* src, int len);
	static void copyDigits(char* dst, const char* src, int len);

	bool isNegative() const;
	bool isPositive() const;
	bool isZero() const;

	BigDecimal negate() const;
};	

#endif
