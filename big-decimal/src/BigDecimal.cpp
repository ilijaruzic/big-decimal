#include "BigDecimal.h"
#include <algorithm>
#include <cmath> 
#include <stdexcept>

BigDecimal::BigDecimal() : sgn('+'), digits(new char[1]), digitsCnt(1), wholeDigitsCnt(1) {
	*digits = 0;
}

BigDecimal::BigDecimal(int num) : sgn(num < 0 ? '-' : '+') {
	num = std::abs(num);
	digitsCnt = wholeDigitsCnt = (num != 0) ? static_cast<int>(std::log10(num) + 1) : 1;
	digits = new char[digitsCnt];
	numToDigits(digits, digitsCnt, num);
}

BigDecimal::BigDecimal(const char* num) {
	if (num == nullptr) {
		throw std::invalid_argument("arg 'num' is null!");
	}
	digitsCnt = std::strlen(num);
	if (digitsCnt == 0) {
		throw std::invalid_argument("arg 'num' is empty!");
	}
	sgn = (*num == '-') ? '-' : '+';

	for (; std::strchr("+-0", *num); ++num, --digitsCnt);

	auto* temp = std::strchr(num, '.');
	if (temp) {
		--digitsCnt;
	}
	else {
		temp = num + digitsCnt;
	}
	auto* numEnd = num + std::strlen(num) - 1;
	for (auto* i = numEnd; i > temp && *i == '0'; --i, --digitsCnt);
	wholeDigitsCnt = temp - num;
	if (wholeDigitsCnt == 0) {
		digits = new char[digitsCnt + 1];
		*digits = 0;
		copyDigits(digits + 1, num + 1, digitsCnt);
		++digitsCnt;
		++wholeDigitsCnt;
	}
	else {
		digits = new char[digitsCnt];
		copyDigits(digits, num, wholeDigitsCnt);
		copyDigits(digits + wholeDigitsCnt, num + wholeDigitsCnt + 1, digitsCnt - wholeDigitsCnt);
	}
	if (isZero()) {
		sgn = '+';
	}
}

BigDecimal::BigDecimal(char sgn, const char* digits, int digitsCnt, unsigned wholeDigitsCnt) : sgn(sgn), digits(new char[digitsCnt]), digitsCnt(digitsCnt), wholeDigitsCnt(wholeDigitsCnt) {
	copyDigits(this->digits, digits, digitsCnt);
	if (isZero()) {
		this->sgn = '+';
	}
}

BigDecimal::BigDecimal(const BigDecimal& num) : BigDecimal(num.sgn, num.digits, num.digitsCnt, num.wholeDigitsCnt) {}

BigDecimal::BigDecimal(BigDecimal&& num) noexcept : sgn(num.sgn), digits(num.digits), digitsCnt(num.digitsCnt), wholeDigitsCnt(num.wholeDigitsCnt) {
	num.digits = nullptr;
	num.digitsCnt = num.wholeDigitsCnt = 0;
}

BigDecimal::~BigDecimal() {
	delete[] digits;
	digits = nullptr;
	digitsCnt = wholeDigitsCnt = 0;
}

BigDecimal BigDecimal::shl(int n) const {
	if (n < 0) {
		return shr(-n);
	}
	if (n == 0 || isZero()) {
		return *this;
	}
	auto removingWholeDigitsCnt = wholeDigitsCnt - n;
	const auto* end = digits + digitsCnt - 1;
	auto removingDigitsCnt = digitsCnt;
	for (; *end == 0 && n > 0; --end, --n, --removingDigitsCnt);
	if (removingWholeDigitsCnt > 0) {
		return BigDecimal(sgn, digits, removingDigitsCnt, removingWholeDigitsCnt);
	}
	auto zeros = -removingWholeDigitsCnt + 1;
	auto* removingDigits = new char[removingDigitsCnt + zeros];
	for (int i = 0; i < zeros; ++i) {
		removingDigits[i] = 0;
	}
	copyDigits(removingDigits + zeros, digits, removingDigitsCnt);
	auto&& num = BigDecimal(sgn, removingDigits, removingDigitsCnt + zeros, 1);
	delete[] removingDigits;
	return num;
}

BigDecimal BigDecimal::shr(int n) const {
	if (n < 0) {
		return shr(-n);
	}
	if (n == 0 || isZero()) {
		return *this;
	}
	const auto* start = digits + digitsCnt - 1;
	auto removingDigitsCnt = digitsCnt;
	for (; *start == 0 && n > 0; ++start, --n, --removingDigitsCnt);
	auto removingWholeDigitsCnt = wholeDigitsCnt + n;
	auto* removingDigits = new char[std::max(removingDigitsCnt, removingWholeDigitsCnt)];
	copyDigits(removingDigits, start, removingDigitsCnt);
	for (; removingDigitsCnt < removingWholeDigitsCnt, ++removingDigitsCnt) {
		removingDigits[removingDigitsCnt] = 0;
	}
	auto&& num = BigDecimal(sgn, removingDigits, removingDigitsCnt, removingWholeDigitsCnt);
	delete[] removingDigits;
	return num;
}

BigDecimal BigDecimal::rmd(int* n) const {
	auto oldDigitsCnt = digitsCnt;
	auto oldWholeDigitsCnt = wholeDigitsCnt;
	auto partDigitsCnt = digitsCnt - wholeDigitsCnt;
	auto&& num = shr(partDigitsCnt);
	if (n != nullptr) {
		*n = oldDigitsCnt - num.digitsCnt + num.wholeDigitsCnt - wholeDigitsCnt;
	}
	return num;
}

BigDecimal BigDecimal::add(const BigDecimal* num) const {
	if (isZero()) {
		return num->negate();
	}
	if (num->isZero()) {
		return *this;
	}
	char correctSgn;
	if (isPositive() && num->isPositive()) {
		correctSgn = '+';
	}
	else if (isNegative() && num->isNegative()) {
		correctSgn = '-';
	}
	else {
		auto lhs = this->abs();
		auto rhs = num->abs();
		auto&& number = lhs.greater(&rhs) ? lhs.sub(&rhs) : rhs.sub(&lhs);
		correctSgn = lhs.greater(&rhs) ? this->sgn : num->sgn;
		return BigDecimal(correctSgn, number.digits, number.digitsCnt, number.wholeDigitsCnt);
	}
	int l, r;
	auto lrmd = this->rmd(&l);
	auto rrmd = num->rmd(&r);
	auto n = std::max(l, r);
	auto lshr = lrmd.shr(n - l);
	auto rshr = rrmd.shr(n - r);
	auto correctDigitsCnt = std::max(lshr.digitsCnt, rshr.digitsCnt);
	auto correctWholeDigitsCnt = std::max(this->wholeDigitsCnt, num->wholeDigitsCnt);
	auto correctDigits = new char[correctDigitsCnt];
	char residue = 0;
	for (int i = lshr.digitsCnt - 1, j = rshr.digitsCnt - 1, k = correctDigitsCnt - 1; k >= 0; --i, --j, --k) {
		auto sum = (i >= 0 ? lshr.digits[i] : 0) + (j >= 0 ? rshr.digits[j] : 0) + residue;
		correctDigits[k] = sum % 10;
		residue = sum / 10;
	}
	if (residue > 0) {
		auto* oldDigits = correctDigits;
		correctDigits = new char[correctDigitsCnt + 1];
		*correctDigits = residue;
		copyDigits(correctDigits + 1, oldDigits, correctDigitsCnt);
		++correctDigitsCnt, ++correctWholeDigitsCnt; 
		delete[] oldDigits;
	}
	auto&& number = BigDecimal(correctSgn, correctDigits, correctDigitsCnt, correctDigitsCnt);
	delete[] correctDigits;
	return number.shl(n);
}

BigDecimal BigDecimal::sub(const BigDecimal* num) const {
	if (isZero()) {
		return num->negate();
	}
	if (num->isZero()) {
		return *this;
	}
	auto lhs = this->abs();
	auto rhs = num->abs();
	if (this->sgn != num->sgn) {
		auto&& number = lhs.add(&rhs);
		return BigDecimal(this->sgn, number.digits, number.digitsCnt, number.wholeDigitsCnt);
	}
	if (rhs.greater(&lhs)) {
		auto&& number = rhs.sub(&lhs);
		char correctSgn = this->isPositive() ? '-' : '+';
		return BigDecimal(correctSgn, number.digits, number.digitsCnt, number.wholeDigitsCnt);
	}
	int l, r;
	auto lrmd = this->rmd(&l);
	auto rrmd = num->rmd(&r);
	auto n = std::max(l, r);
	auto lshr = lrmd.shr(n - l);
	auto rshr = rrmd.shr(n - r);
	auto correctDigitsCnt = std::max(lshr.digitsCnt, rshr.digitsCnt);
	auto correctWholeDigitsCnt = std::max(this->wholeDigitsCnt, num->wholeDigitsCnt);
	auto correctDigits = new char[correctDigitsCnt];
	char residue = 0;
	for (int i = lshr.digitsCnt - 1, j = rshr.digitsCnt - 1, k = correctDigitsCnt - 1; k >= 0; --i, --j, --k) {
		auto sum = (i >= 0 ? lshr.digits[i] : 0) - (j >= 0 ? rshr.digits[j] : 0) - residue;
		correctDigits[k] = (sum + 10) % 10;
		residue = sum < 0;
	}
	auto* start = correctDigits;
	for (; *start == 0 && correctDigitsCnt > 1; ++start, --correctDigitsCnt, --correctWholeDigitsCnt);
	auto&& number = BigDecimal(this->sgn, start, correctDigitsCnt, correctDigitsCnt);
	delete[] correctDigits;
	return number.shl(n);
}

bool BigDecimal::greater(const BigDecimal* num) const {
	return !less(num) && !equals(num);
}

bool BigDecimal::less(const BigDecimal* num) const {
	if ((isPositive() || isZero()) && (num->isNegative() || num->isZero())) {
		return false;
	}
	if (isNegative() && (num->isPositive() || num->isZero()) || isZero() && num->isPositive()) {
		return true;
	}
	if (wholeDigitsCnt > num->wholeDigitsCnt) {
		return isNegative();
	}
	if (wholeDigitsCnt < num->wholeDigitsCnt) {
		return isPositive();
	}
	for (int i = 0; i < std::min(digitsCnt, num->digitsCnt); ++i) {
		auto lhs = this->digits[i];
		auto rhs = num->digits[i];
		if (lhs > rhs) {
			return isNegative();
		}
		if (lhs < rhs) {
			return isPositive();
		}
	}
	return false;
}

bool BigDecimal::equals(const BigDecimal* num) const {
	if (sgn != num->sgn || digitsCnt != num->digitsCnt || wholeDigitsCnt != num->wholeDigitsCnt) {
		return false;
	}
	for (int i = 0; i < digitsCnt; ++i) {
		if (digits[i] != num->digits[i]) {
			return false;
		}
	}
	return true;
}

BigDecimal BigDecimal::abs() const {
	return BigDecimal('+', digits, digitsCnt, wholeDigitsCnt);
}

std::ostream& operator<<(std::ostream& os, const BigDecimal& num) {
	if (num.isNegative()) {
		os << '-';
	}
	for (int i = 0; i < num.digitsCnt; ++i) {
		if (i == num.wholeDigitsCnt) {
			os << (i == 0 ? "0." : ".");
		}
		os << static_cast<char>(num.digits[i] + '0');
	}
	return os;
}

void BigDecimal::numToDigits(char* dst, int len, int num) {
	for (int i = len - 1; i >= 0; --i) {
		dst[i] = num % 10;
		num /= 10;
	}
}

int BigDecimal::digitsToNum(const char* src, int len) {
	int num = 0;
	for (int i = 0; i < len; ++i) {
		num = num * 10 + src[i];
	}
	return num;
}

void BigDecimal::copyDigits(char* dst, const char* src, int len) {
	for (int i = 0; i < len; ++i) {
		dst[i] = src[i];
	}
}

void BigDecimal::copyDigits(char* dst, const char* src, int len) {
	for (int i = 0; i < len; ++i) {
		dst[i] = src[i] - '0';
	}
}

bool BigDecimal::isNegative() const {
	return sgn == '-';
}

bool BigDecimal::isPositive() const {
	return sgn == '+' && !isZero();
}

bool BigDecimal::isZero() const {
	return digitsCnt == 1 && *digits == 0;
}

BigDecimal BigDecimal::negate() const {
	return BigDecimal(isPositive() ? '-' : '+', digits, digitsCnt, wholeDigitsCnt);
}