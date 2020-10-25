#pragma once

/**

The engine uses 3 different linear coordinate systems
which are oriented around each of the axis dimensions.

So any point within the game can be defined by four coordinates: (d, x, y, z)

d is the reference axis dimension
x is the coordinate of the ROW dimension
y is the coordinate of the COL dimension
z is the coordinate of the reference dimension (DEPTH)

typically, if d is not used, then it is implicitly the Z dimension.
ie: d=z => x=x, y=y, z=z

**/

// DIM: X=0 Y=1 Z=2.
constexpr int R[3]  = {1, 2, 0}; // row
constexpr int C[3]  = {2, 0, 1}; // col
constexpr int D[3]  = {0, 1, 2}; // depth

constexpr auto PI = 3.14159265358979f;
constexpr auto SQRT2 = 1.4142135623731f;
constexpr auto SQRT3 = 1.73205080756888f;
constexpr auto SQRT5 = 2.23606797749979f;
constexpr auto RAD = PI / 180.0f;