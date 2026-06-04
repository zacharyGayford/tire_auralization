#ifndef MATH_H
#define MATH_H

template<typename T>
union v2 {
	struct { T x, y; };
	struct { T s, t; };
	struct { T width, height; };
	T e[2];
};

template<typename T>
union v3 {
	struct { T x, y, z; };
	struct { T s, t, p; };
	struct { T r, g, b; };
	struct { T width, height, depth; };
	T e[3];
};

template<typename T>
union v4 {
	struct { T x, y, z, w; };
	struct { T s, t, p, q; };
	struct { T r, g, b, a; };
	T e[3];
};

#endif // MATH_H
