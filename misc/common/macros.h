#ifndef MACROS_H
#define MACROS_H

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SAT(in, max, min) MIN(MAX(in, min), max)

#endif  // MACROS_H
