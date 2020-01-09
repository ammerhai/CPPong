#pragma once

#include <xmmintrin.h>
#include <stdint.h>
#include <assert.h>

//wurzelberechnung
float square_root(float a) {
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(a)));
}

float reciprocal_square_root(float a) {
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(a)));
}


//Vektorberechnung 2-dim
union V2 {
	struct {
		float x;
		float y;
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

//Matrix addieren
inline M2x2 operator +(M2x2 a, M2x2 b) {
	return {
		a[0][0] + b[0][0], a[0][1] + b[0][1],
		a[1][0] + b[1][0], a[1][1] + b[1][1]
	};
}

//Matrix subtrahieren
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
inline M2x2 operator *(M2x2 a, V2 b) {
	return {
		a[0][0] * b[0] + a[0][1] * b[1],
		a[1][0] * b[0] + a[1][1] * b[1],
	};
}