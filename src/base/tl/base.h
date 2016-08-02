

#ifndef BASE_TL_BASE_H
#define BASE_TL_BASE_H

#include <base/system.h>

inline void tl_assert(bool statement)
{
	dbg_assert(statement, "assert!");
}

template<class T>
inline void tl_swap(T &a, T &b)
{
	T c = b;
	b = a;
	a = c;
}

#endif
