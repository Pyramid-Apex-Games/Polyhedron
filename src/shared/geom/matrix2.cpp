#include "matrix2.h"
#include "matrix3.h"
#include "matrix4.h"

matrix2::matrix2(const matrix4 &m) : a(m.a), b(m.b) {}
matrix2::matrix2(const matrix3 &m) : a(m.a), b(m.b) {}