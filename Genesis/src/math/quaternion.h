#ifndef _GENESIS_QUATERNION_H_
#define _GENESIS_QUATERNION_H_

namespace Genesis
{

class Quaternion
{
public:
    Quaternion();
    Quaternion( float mx, float my, float mz, float mw );

    static float Length( const Quaternion& q );
    static void Normalize( Quaternion& q );
    static void Conjugate( Quaternion& q );
    static void Multiply( const Quaternion& q1, const Quaternion& q2, Quaternion& qOut );

    float x, y, z, w;
};
}

#endif