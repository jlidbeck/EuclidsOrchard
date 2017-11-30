#pragma once

#include <stdio.h>
#include <afx.h>

//
//	Random number utils
//

// returns a random float uniformly distributed in the interval a...b
template<class T>
inline T random(T a, T b) {
	return a + (T)(rand()) * ((b - a) / (T)RAND_MAX);
}

inline float randomFloat() {
	return (float)(rand()) / (float)RAND_MAX;
}

inline bool randomBool(float probability) {
	return (rand() < probability * RAND_MAX);
}

// random pair of floats with normal distribution
// The polar form of the Box - Muller transformation 
inline float* randomGaussianPair() {

	float x1, x2, w;
	static float f = 2.0f / RAND_MAX;
	static float out[2];

	do {
		x1 = float(rand()) * f - 1.0f;
		x2 = float(rand()) * f - 1.0f;
		w = x1 * x1 + x2 * x2;
	} while(w >= 1.0);

	w = sqrt((-2.0f * log(w)) / w);
	out[0] = x1 * w;
	out[1] = x2 * w;

	return out;
}

inline float randomGaussian() {
	static int i = 0;
	static const float * buf;
	if(!i) {
		buf = randomGaussianPair();
	}
	float f = buf[i];
	i = i ^ 1;
	return buf[i];
}

//	exponential distribution. 
//	If lambda is the rate parameter of the exponential distribution,
//	the mean is 1 / lambda.
//	mean is used as the parameter to avoid division and for more intuitive control.
//
//	range: [0, +inf)
//	median = ln(2) / lambda = ln(2) * mean
inline float randomExponential(float mean) {
	return log(1.0f - (rand() / (float)RAND_MAX)) * -mean;
}

// shuffle array of elements in-place
template <class C>
void shuffle(C *str, int n) {
	for(int i = n - 1; i > 0; --i) {
		int j = rand() % i;
		C x = str[j];
		str[j] = str[i];
		str[i] = x;
	}
}

// shuffle a null-terminated string in-place
template <class C>
void shuffle(C *str) {
	for(int i = strlen(str) - 1; i > 0; --i) {
		int j = rand() % i;
		C c = str[j];
		str[j] = str[i];
		str[i] = c;
	}
}

//
//	modulus
//

// true modulus function: unlike remainder (%) operator, 
// the returned value has same sign as second parameter
inline int modi(int a, int b) {
	return a < 0
		? a % b + b
		: a % b;
}

// true modulus function: unlike remainder (%) operator, 
// the returned value has same sign as second parameter
inline unsigned int modi(int a, unsigned int b) {
	return a < 0
		? a % b + b
		: a % b;
}

//
//	std::priority_queue lacks a clear() method:
//	this can be used to clear priority_queue, queue, and lots of other containers
//

template <class Q>
void clearQueue(Q & q) {
	q = Q();
}


