#pragma once

namespace Genesis
{

class SceneObject
{
public:
    SceneObject();
    virtual ~SceneObject();
    virtual void Update( float );
    virtual void Render() = 0;

    inline void SetTerminating() { m_Terminating = true; }
    inline bool IsTerminating() const { return m_Terminating; }

private:
    bool m_Terminating;
};
}
