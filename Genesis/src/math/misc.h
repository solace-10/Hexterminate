#pragma once

#include <cmath>
#include <cstdlib>

template <class T>
T gClamp( T v, T min, T max )
{
    if ( min > max || max < min )
        return v;
    else if ( v < min )
        return min;
    else if ( v > max )
        return max;
    else
        return v;
}

inline float gRand()
{
    return (float)rand() / (float)RAND_MAX;
}

inline float gRand( float min, float max )
{
    return min + gRand() * ( max - min );
}

inline int gRand( int min, int max )
{
	if ( min == max ) 
	{
		return max;
	}
	else
	{
		return min + rand() % ( max - min );
	}
}

inline float gMax( float v1, float v2 )
{
    if ( v1 > v2 )
        return v1;
    else
        return v2;
}

inline int gMax( int v1, int v2 )
{
    if ( v1 > v2 )
        return v1;
    else
        return v2;
}

inline float gMin( float v1, float v2 )
{
    if ( v1 < v2 )
        return v1;
    else
        return v2;
}

inline int gMin( int v1, int v2 )
{
    if ( v1 < v2 )
        return v1;
    else
        return v2;
}

inline bool gIsEqual( float x, float y )
{
    const float epsilon = 1.0e-5f;
    return std::abs( x - y ) <= epsilon * std::abs( x );
}
