#include "quaternion.h"
#include <cmath>

namespace Genesis
{

//---------------------------------------------------------------
// Quaternion
//---------------------------------------------------------------

Quaternion::Quaternion()
    : x( 0.0f )
    , y( 0.0f )
    , z( 0.0f )
    , w( 0.0f )
{
}

Quaternion::Quaternion( float mx, float my, float mz, float mw )
    : x( mx )
    , y( my )
    , z( mz )
    , w( mw )
{
}

float Quaternion::Length( const Quaternion& q )
{
    return sqrt( q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w );
}

void Quaternion::Normalize( Quaternion& q )
{
    float len = Length( q );
    q.x /= len;
    q.y /= len;
    q.z /= len;
    q.w /= len;
}

void Quaternion::Conjugate( Quaternion& q )
{
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
}

void Quaternion::Multiply( const Quaternion& q1, const Quaternion& q2, Quaternion& qOut )
{
    qOut.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    qOut.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    qOut.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    qOut.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
}
}