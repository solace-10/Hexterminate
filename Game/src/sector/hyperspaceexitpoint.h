#pragma once

namespace Hexterminate
{

class HyperspaceExitPoint
{
public:
    HyperspaceExitPoint( unsigned int index );
    void Update( float delta );
    bool IsAvailable() const;
    void Use();
    unsigned int GetIndex() const;

private:
    float m_Timer;
    unsigned int m_Index;
};

inline bool HyperspaceExitPoint::IsAvailable() const
{
    return m_Timer > 0.0f;
}

inline void HyperspaceExitPoint::Use()
{
    m_Timer = 30.0f;
}

inline unsigned int HyperspaceExitPoint::GetIndex() const
{
    return m_Index;
}

} // namespace Hexterminate
