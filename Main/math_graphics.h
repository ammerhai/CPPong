#pragma once

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
