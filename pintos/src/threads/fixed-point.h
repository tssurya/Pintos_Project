/*In the formulas used, priority, niceness, and ready_threads are integers, but recent_cpu_ticks and load_avg are real
numbers. Unfortunately, Pintos does not support floating-point arithmetic in the kernel, because it would complicate 
and slow the kernel. Real kernels often have the same limitation, for the same reason. This means that calculations 
on real quantities must be simulated using integers. This is not difficult, but many students do not know how to do 
it. This section explains the basics.

The fundamental idea is to treat the rightmost bits of an integer as representing a fraction. For example, we can 
designate the lowest 14 bits of a signed 32-bit integer as fractional bits, so that an integer x represents the real 
number x/(2**14), where ** represents exponentiation. This is called a 17.14 fixed-point number representation, 
because there are 17 bits before the decimal point, 14 bits after it, and one sign bit.(7) A number in 17.14 format 
represents, at maximum, a value of (2**31 - 1)/(2**14) = approx. 131,071.999.

Suppose that we are using a p.q fixed-point format, and let f = 2**q. By the definition above, we can convert an 
integer or real number into p.q format by multiplying with f. For example, in 17.14 format the fraction 59/60 used in
the calculation of load_avg, above, is 59/60*(2**14) = 16,110. To convert a fixed-point value back to an integer, 
divide by f. (The normal / operator in C rounds toward zero, that is, it rounds positive numbers down and negative 
numbers up. To round to nearest, add f / 2 to a positive number, or subtract it from a negative number, before 
dividing.)

Many operations on fixed-point numbers are straightforward. Let x and y be fixed-point numbers, and let n be an 
integer. Then the sum of x and y is x + y and their difference is x - y. The sum of x and n is x + n * f; difference, 
x - n * f; product, x * n; quotient, x / n.

Multiplying two fixed-point values has two complications. First, the decimal point of the result is q bits too far to
the left. Consider that (59/60)*(59/60) should be slightly less than 1, but 16,111*16,111 = 259,564,321 is much 
greater than 2**14 = 16,384. Shifting q bits right, we get 259,564,321/(2**14) = 15,842, or about 0.97, the correct 
answer. Second, the multiplication can overflow even though the answer is representable. For example, 64 in 17.14 
format is 64*(2**14) = 1,048,576 and its square 64**2 = 4,096 is well within the 17.14 range, but 1,048,576**2 = 2**40,
greater than the maximum signed 32-bit integer value 2**31 - 1. An easy solution is to do the multiplication as a 
64-bit operation. The product of x and y is then ((int64_t) x) * y / f.

Dividing two fixed-point values has opposite issues. The decimal point will be too far to the right, which we fix by 
shifting the dividend q bits to the left before the division. The left shift discards the top q bits of the dividend,
which we can again fix by doing the division in 64 bits. Thus, the quotient when x is divided by y is ((int64_t) x) *
f / y.*/

#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#define Q 14
#define F (1<<Q)

typedef int real;

static real fp_create(int num, int denom);
static real fp_multiply(real x, real y);
static real fp_divide (real x , real y);
static int fp_round_down(real x);
static int fp_round_nearest(real x);

/*return the real number obtained by dividing the numerator by the denominator.*/
static real fp_create(int num, int denom)
{
	return (num * F) / denom;
}

static real fp_multiply(real x , real y)
{
	return ((int64_t)x)*y / F;
}

static real fp_divide(real x, real y)
{
	return ((int64_t)x)*F / y;
}

static int fp_round_down(real x)
{
	return x/F;
}

static int fp_round_nearest(real x)
{
	return (x>=0) ? (x+F/2)/F : (x-F/2)/F ;
}

#endif

