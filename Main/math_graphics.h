#pragma once

#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <stdint.h>
#include <assert.h>

//clamp
constexpr inline float clamp(float min, float a, float max) {
	float result = a;
	if (a < min)
		result = min;
	if (a > max)
		result = max;
	return result;
}

//clamp für 0-1 Bereich (Grafik)
constexpr inline float clamp01(float a) {
	return clamp(0, a, 1);
}


//wurzelberechnung
inline float square_root(float a) {
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(a)));
}

inline float reciprocal_square_root(float a) {
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(a)));
}

constexpr inline float min(float a, float b) {
	return a < b ? a : b;
}

constexpr inline float max(float a, float b) {
	return a > b ? a : b;
}

constexpr inline double min(double a, double b) {
	return a < b ? a : b;
}

constexpr inline double max(double a, double b) {
	return a > b ? a : b;
}

constexpr inline int64_t min(int64_t a, int64_t b) {
	return a < b ? a : b;
}

template<typename T>
constexpr inline T min(T a, T b) {
	return a < b ? a : b;
}

constexpr float lerp(float a, float t, float b) {
	return (1.0f - t) * a + t * b;
}


//-----------------------------------------------
//Vektorberechnung 2-dim
union V2 {
	struct {
		float x;
		float y;
	};

	struct {
		float u;
		float v;
	};

	struct {
		float width;
		float height;
	};

	struct {
		float E[2];
	};

	float operator [](size_t index) {
		assert(index < 2);
		return  E[index];
	}
};


//Negation von 2-dim Vektor
inline V2 operator -(V2 a) {
	return {
		-a.x,
		-a.y
	};
}

//Addition 2er 2-dim Vektoren
inline V2 operator +(V2 a, V2 b) {
	return {
		a.x + b.x,
		a.y + b.y
	};
}

//Subtraktion 2er 2-dim Vektoren
inline V2 operator -(V2 a, V2 b) {
	return {
		a.x - b.x,
		a.y - b.y
	};
}

//Skalarmultiplikation -> erst Skalar, dann Vektor
inline V2 operator *(float a, V2 b) {
	return {
		a * b.x,
		a * b.y
	};

}

//Skalarmultiplikation -> erst Vektor, dann Skalar
inline V2 operator *(V2 a, float b) {
	return {
		a.x * b,
		a.y * b
	};

}

//Division mit nem Skalar Oo -> Skalar geteilt durch Vektor
inline V2 operator /(float a, V2 b) {
	return {
		a / b.x,
		a / b.y
	};
}

//Division mit nem Skalar Oo -> Vektor geteilt durch Skalar
inline V2 operator /(V2 a, float b) {
	return {
		a.x / b,
		a.y / b
	};
}

//Skalarprodukt
inline float dot(V2 a, V2 b) {
	return a.x * b.x + a.y * b.y;
}

//Hadamard-Produkt
inline V2 hadamard(V2 a, V2 b) {
	return {
		a.x * b.x,
		a.y * b.y
	};
}

//Betrag des Vektors quadrieren
inline float length_squared(V2 a) {
	return dot(a, a);
}

//Betrag eines Vektors
inline float length(V2 a) {
	return square_root(length_squared(a));
}

//Reziproke der Länge
inline float reciprocal_length(V2 a) {
	return reciprocal_square_root(length_squared(a));
}

//Einheitsvektor
inline V2 normalize(V2 a) {
	return a * reciprocal_length(a);
}

//Vektor der 90°
inline V2 perp(V2 a) {
	return {
		-a.y,
		a.x
	};
}

//clamp für 2-dim Vektor
inline V2 clamp01(V2 a) {
	return {
		clamp01(a.x),
		clamp01(a.y)
	};
}

inline V2 min(V2 a, V2 b) {
	return {
		min(a.x, b.x),
		min(a.y, b.y),
	};
}

inline V2 max(V2 a, V2 b) {
	return {
		max(a.x, b.x),
		max(a.y, b.y),
	};
}

inline float min(V2 a) {
	return min(a.x, a.y);
}

inline float max(V2 a) {
	return max(a.x, a.y);
}


//-----------------------------------------------
//Vektorberechnung 3-dim
union V3 {
	struct {
		float x;
		float y;
		float z;
	};

	//farbvektor
	struct {
		float r;
		float g;
		float b;
	};
	
	//texturvektor
	struct {
		float u;
		float v;
		float s;
	};

	//von V3 zu V2 ohne z
	struct {
		V2 xy;
		float _z;
	};

	//von V3 zu V2 ohne x
	struct {
		float _x;
		V2 yz;
	};

	struct {
		float E[3];
	};

	float operator [](size_t index) {
		assert(index < 3);
		return  E[index];
	}
};

//Negation von 2-dim Vektor
inline V3 operator -(V3 a) {
	return {
		-a.x,
		-a.y,
		-a.z
	};
}


//Addition 2er 2-dim Vektoren
inline V3 operator +(V3 a, V3 b) {
	return {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

//Subtraktion 2er 2-dim Vektoren
inline V3 operator -(V3 a, V3 b) {
	return {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}

//Skalarmultiplikation -> erst Skalar, dann Vektor
inline V3 operator *(float a, V3 b) {
	return {
		a * b.x,
		a * b.y,
		a * b.z
	};

}

//Skalarmultiplikation -> erst Vektor, dann Skalar
inline V3 operator *(V3 a, float b) {
	return {
		a.x * b,
		a.y * b,
		a.z * b
	};

}

//Division mit nem Skalar Oo -> Skalar geteilt durch Vektor
inline V3 operator /(float a, V3 b) {
	return {
		a / b.x,
		a / b.y,
		a / b.z
	};
}

//Division mit nem Skalar Oo -> Vektor geteilt durch Skalar
inline V3 operator /(V3 a, float b) {
	return {
		a.x / b,
		a.y / b,
		a.z / b
	};
}

//Skalarprodukt
inline float dot(V3 a, V3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

//Hadamard-Produkt
inline V3 hadamard(V3 a, V3 b) {
	return {
		a.x * b.x,
		a.y * b.y,
		a.z * b.z
	};
}

//Kreuzprodukt
inline V3 cross(V3 a, V3 b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

//Betrag des Vektors quadrieren
inline float length_squared(V3 a) {
	return dot(a, a);
}

//Betrag eines Vektors
inline float length(V3 a) {
	return square_root(length_squared(a));
}

//Reziproke der Länge
inline float reciprocal_length(V3 a) {
	return reciprocal_square_root(length_squared(a));
}

//Einheitsvektor
inline V3 normalize(V3 a) {
	return a * reciprocal_length(a);
}

union V4 {
	struct {
		float x;
		float y;
		float z;
		float w;
	};

	//farbvektor
	struct {
		float r;
		float g;
		float b;
		float a;
	};

	//texturvektor
	struct {
		float u;
		float v;
		float s;
		float t;
	};

	//von V3 zu V2 ohne z
	struct {
		V2 xy;
		V2 zw;
	};

	//von V3 zu V2 ohne x
	struct {
		float _x;
		V2 yz;
		float _w;
	};

	struct {
		float E[4];
	};

	V4(V2 a, V2 b) { xy = a; zw = b; }
	V4(float a, float b, float c, float d) { x = a; y = b; z = c; w = d; }
	V4(float a, float b, float c) { x = a; y = b; z = c; w = 1; }
	V4() {}

	float operator [](size_t index) {
		assert(index < 4);
		return  E[index];
	}
};


//-----------------------------------------------
//2x2 Matrix

//M2x2 m;
//m.E[0][1]
//m.V[1]

//m[1][0]
union M2x2 {
	struct {
		float x1; float x2;
		float y1; float y2;
	};

	struct {
		float E[2][2];
	};

	struct {
		V2 V[2];
	};

	V2 &operator [](size_t index) {
		assert(index < 2);
		return V[index];
	}
};

//Matrix negieren
inline M2x2 operator -(M2x2 a){
	return {
		-a[0][0], -a[0][1],
		-a[1][0], -a[1][1]
	};
}

//Matrix Addition
inline M2x2 operator +(M2x2 a, M2x2 b) {
	return {
		a[0][0] + b[0][0], a[0][1] + b[0][1],
		a[1][0] + b[1][0], a[1][1] + b[1][1]
	};
}

//Matrix Subtraktion
inline M2x2 operator -(M2x2 a, M2x2 b) {
	return {
		a[0][0] - b[0][0], a[0][1] - b[0][1],
		a[1][0] - b[1][0], a[1][1] - b[1][1]
	};
}

//Matrix Skalarmultiplikation 
inline M2x2 operator *(M2x2 a, float b) {
	return {
		a[0][0] * b, a[0][1] * b,
		a[1][0] * b, a[1][1] * b
	};
}

//Matrix Skalarmultiplikation 
inline M2x2 operator *(float a, M2x2 b) {
	return {
		a * b[0][0], a * b[0][1],
		a * b[1][0], a * b[1][1]
	};
}

//Matrix Multiplikation
inline M2x2 operator *(M2x2 a, M2x2 b) {
	return {
		a[0][0] * b[0][0] + a[0][1] * b[1][0], a[0][0] * b[0][1] + a[0][1] * b[1][1], 
		a[1][0] * b[0][0] + a[1][1] * b[1][0], a[1][0] * b[0][1] + a[1][1] * b[1][1]
	};
}

//Matrix * Vektor
inline V2 operator *(M2x2 a, V2 b) {
	return {
		a[0][0] * b[0] + a[0][1] * b[1],
		a[1][0] * b[0] + a[1][1] * b[1],
	};
}

//Matrix Transponieren
inline M2x2 transpose(M2x2 a) {
	return {
		a[0][0], a[1][0],
		a[0][1], a[1][1]
	};
}

//Einheitsmatrix (oder Identitätsmatrix)
constexpr inline M2x2 identityM2x2() {
	return {
		1.0f, 0.0f,
		0.0f, 1.0f
	};
}


//-----------------------------------------------
//3x3 Matrix
union M3x3 {
	struct {
		float x1; float x2; float x3;
		float y1; float y2; float y3;
		float z1; float z2; float z3;
	};

	struct {
		float E[3][3];
	};

	struct {
		V3 V[3];
	};
	

	V3& operator [](size_t index) {
		assert(index < 3);
		return V[index];
	}
	
};

//Matrix negieren
inline M3x3 operator -(M3x3 a) {
	return {
		-a[0][0], -a[0][1], -a[0][1],
		-a[1][0], -a[1][1], -a[1][2],
		-a[2][0], -a[2][1], -a[2][2]
	};
}

//Matrix Addition
inline M3x3 operator +(M3x3 a, M3x3 b) {
	return {
		a[0][0] + b[0][0], a[0][1] + b[0][1], a[0][2] + b[0][2],
		a[1][0] + b[1][0], a[1][1] + b[1][1], a[1][2] + b[1][2],
		a[2][0] + b[2][0], a[2][1] + b[2][1], a[2][2] + b[2][2]
	};
}

//Matrix Subtraktion
inline M3x3 operator -(M3x3 a, M3x3 b) {
	return {
		a[0][0] - b[0][0], a[0][1] - b[0][1], a[0][2] - b[0][2],
		a[1][0] - b[1][0], a[1][1] - b[1][1], a[1][2] - b[1][2],
		a[2][0] - b[2][0], a[2][1] - b[2][1], a[2][2] - b[2][2]
	};
}

//Matrix Skalarmultiplikation 
inline M3x3 operator *(M3x3 a, float b) {
	return {
		a[0][0] * b, a[0][1] * b, a[0][2] * b,
		a[1][0] * b, a[1][1] * b, a[1][2] * b,
		a[2][0] * b, a[2][1] * b, a[2][2] * b
	};
}

//Matrix Skalarmultiplikation 
inline M3x3 operator *(float a, M3x3 b) {
	return {
		a * b[0][0], a * b[0][1], a * b[0][2],
		a * b[1][0], a * b[1][1], a * b[1][2],
		a * b[2][0], a * b[2][1], a * b[2][2]
	};
}

//Matrix Multiplikation
inline M3x3 operator *(M3x3 a, M3x3 b) {
	return {
		a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0], a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1], a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2],
		a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0], a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1], a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[0][2] * b[2][2],
		a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0], a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1], a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[0][2] * b[2][2]
	};
}

//Matrix * V2
inline V2 operator *(M3x3 a, V2 b) {
	return {
		b.x * a[0][0] + b.y * a[0][1] + 1.0f * a[0][2],
		b.x * a[1][0] + b.y * a[1][1] + 1.0f * a[1][2],
	};
}

//Matrix * V3
inline V3 operator *(M3x3 a, V3 b) {
	return {
		b.x * a[0][0] + b.y * a[0][1] + b.z * a[0][2],
		b.x * a[1][0] + b.y * a[1][1] + b.z * a[1][2],
		b.x * a[2][0] + b.y * a[2][1] + b.z * a[2][2]
	};
}


//Matrix transponieren
inline M3x3 transpose(M3x3 a) {
	return {
		a[0][0], a[1][0], a[2][0],
		a[0][1], a[1][1], a[2][1],
		a[0][2], a[1][2], a[2][2]
	};
}

//Einheitsmatrix (oder Identitätsmatrix)
inline M3x3 identityM3x3() {
	return {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};
}



//-----------------------------------------------
//m128i
struct m128i {
	__m128i val;
};

inline __m128i operator &(m128i a, m128i b) {
	return _mm_and_si128(a.val, b.val);
}

inline __m128i operator |(m128i a, m128i b) {
	return _mm_or_si128(a.val, b.val);
}

inline __m128i operator >>(m128i a, int b) {
	return _mm_srli_epi32(a.val, b);
}

inline __m128i operator <<(m128i a, int b) {
	return _mm_slli_epi32(a.val, b);
}


//-----------------------------------------------
//m128
struct m128 {
	__m128 val;
};

inline __m128 operator +(m128 a, m128 b) {
	return _mm_mul_ps(a.val, b.val);
}

inline __m128 operator *(m128 a, m128 b) {
	return _mm_mul_ps(a.val, b.val);
}

inline __m128 operator *(float a, m128 b) {
	return _mm_mul_ps(_mm_set1_ps(a), b.val);
}

inline __m128 square_root(__m128 a) {
	return _mm_sqrt_ps(a);
}

inline __m128 operator /(m128 a, m128 b) {
	return _mm_div_ps(a.val, b.val);
}

inline __m128 lerp(__m128 a, float t, float b) {
	return (1.0f - t) * a + (t * b);
}