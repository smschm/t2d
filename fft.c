#include "turing.h"
#include <math.h>

double *cos_table;
double *sin_table;
void bit_swap(int *n, int a, int b);

void fill_tables(int len) {
	cos_table = malloc(sizeof(double)*len);
	sin_table = malloc(sizeof(double)*len);
	int i;
	double r = 6.28318530718/len;
	for (i = 0; i < len; i++) {
		cos_table[i] = cos(r*i);
		sin_table[i] = sin(r*i);
		// printf("table[%i]: cos %f sin %f\n",i,cos_table[i],sin_table[i]);
	}
}

void free_tables() {
	free(cos_table);
	free(sin_table);
}

void cplx_mult(struct complex *a, struct complex *b, struct complex *res) {
	res->real = a->real*b->real - a->imag*b->imag;
	res->imag = a->imag*b->real + a->real*b->imag;
}

// DFT/FFT and Convolution Algorithms, Burrus & Parks 1985
void burrus_fft_ip(struct complex* data, int bits) {
	int n1, n2, n;
	n2 = n = 1 << bits; // N2 = N
	int i, j, k, l;
	double e, c, s, a;
	struct complex t;
	for (k = 0; k < bits; k++) {	// DO 10 K=1, M
		n1 = n2;			// N1 = N2
		n2 >>= 1;			// N2 = N2/2
		e = 6.28318530718/n1;	// E = 6.28318530718/n1;
						// A = 0
		for (j = 0; j < n2; j++) {	// DO 20 J=1, N2
			a = j * e;
			c = cos(a);		// C = COS(A)
			s = -sin(a);	// S = -SIN(A)
						// A = J * E
			for (i = j; i < n; i += n1) {	// DO 30 I = J, N, N1
				l = i + n2;	// L = I + N2
				t.real = data[i].real - data[l].real;	// XT = X(I) - X(L)
				data[i].real += data[l].real;			// X(I) = X(I) + X(L)
				t.imag = data[i].imag - data[l].imag;	// YT = Y(I) - Y(L)
				data[i].imag += data[l].imag;			// Y(I) = Y(I) + Y(L)
				data[l].real = t.real * c - t.imag * s;	// X(L) = XT * C - YT * S
				data[l].imag = t.real * s + t.imag * c;	// Y(L) = XT * S + YT * C
			} // 30 CONTINUE
		} // 20 CONTINUE
	} // 10 CONTINUE
	unscramble(data, bits);
}
/*
void basic_fft(struct complex* data, int bits) {
	int len = 1 << bits;
	int i, j, k, step = len; // len/2, ..., 2, 1;
	int offset;
	int tforms = 1; // 1, 2, 4, .., len/2
	struct complex temp, twiddle;
	//double ;
	
	for (k = 0; k < bits; k++) {
		step >>= 1;	// difference between values to butterfly
		offset = 0;
		// this is some butterfly shit
		for (i = 0; i < tforms; i++) {	// # of separate butterflies
			for (j = 0; j < step; j++) {	// # of switches per b.fly
				temp = data[offset];
				data[offset].real += data[offset+step].real;
				data[offset+step].real -= temp.real;
				data[offset].imag += data[offset+step].imag;
				data[offset+step].imag -= temp.imag;
				offset++;
			}
			offset += step;
		}
		// this is some twiddle shit
		
		tforms <<= 1;
	}
}
*/
void naive_dft(struct complex* data, int bits) {
	// uhh, X[w] = sum_n=0^N-1[ x[n]exp(-jnw*(2pi/N)) ]
	// = sum n = 0 to N-1 ( x[n]cos(nwr) - ix[n]sin(nwr) )
	int len = 1 << bits;
	struct complex* dft = malloc(sizeof(struct complex) * len);
	// double r = 6.28318530718/len;
	int w, n, off;
	for (w = 0; w < len; w++) {
		dft[w].real = 0;
		dft[w].imag = 0;
		for (n = 0; n < len; n++) {
			// this segment assuming purely real input!
			off = (n * w) % len;
			dft[w].real += data[n].real * cos_table[off];
			dft[w].imag -= data[n].real * sin_table[off];
			printf("data[%i] = %f + %fj\n",n,data[n].real,data[n].imag);
		}
	}
	for (w = 0; w < len; w++) {
		data[w] = dft[w];
	}
	free(dft);
}
void unscramble(struct complex* data, int bits) {
	int i, j = 1, k;
	int n = 1 << bits;
	int n1 = n - 1;
	struct complex t;
	for (i = 1; i < n1; i++) {
		if (i < j) {
			t = data[j];
			data[j] = data[i];
			data[i] = t;
		}
		k = n >> 1;
		while (k < j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
}

// this is awful.
void unscramble(struct complex* data, int bits) {
	int i, n, pos;
	struct complex t;
	n = 1 << bits;
	short* fixed = malloc(sizeof(short) * n);
	for (i = 1; i < n; i++) {
		fixed[i] = 0;
	}
	for (i = 1; i < n; i++) {
		pos = bit_reverse(i, bits);
		if (pos != i && !fixed[pos]) {
			t = data[pos];
			data[pos] = data[i];
			data[i] = t;
		}
		fixed[i] = 1;
		fixed[pos] = 1;
	}
	free(fixed);
}
int bit_reverse(int n, int bits) {
	int i, ret = n;
	for (i = 0; i < bits/2; i++) {
		bit_swap(&ret, i, bits-i-1);
	}
	// printf("%i-bit reverse of %i is %i\n",bits,n,ret);
	return ret;
}
void bit_swap(int *n, int a, int b) {
	int a_mask, b_mask, a_bit, b_bit, d = b - a;
	a_mask = 1 << a;
	b_mask = 1 << b;
	a_bit = (a_mask & *n) << d;
	b_bit = (b_mask & *n) >> d;
	*n &= ~(a_mask | b_mask);
	*n = *n | a_bit | b_bit;
}

void int2complex(int * from, struct complex * to, int n) {
	int i;
	for (i = 0; i < n; i++) {
		to[i].imag = 0.0;
		to[i].real = from[i];
	}
}

double magnitude(struct complex z) {
	return z.real*z.real + z.imag*z.imag;
}
