#pragma once

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define SAT(in, max, min) MIN(MAX(in, min), max)
