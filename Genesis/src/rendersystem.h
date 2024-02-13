// Copyright 2016 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <bitset>
#include <vector>

#include "rendersystem.fwd.h"
#include "glm/gtx/transform.hpp"
#include "render/rendertarget.h"
#include "color.h"
#include "inputmanager.h"
#include "shader.h"
#include "shaderuniformtype.h"
#include "taskmanager.h"

namespace Genesis
{

class ShaderCache;
class VertexBuffer;

///////////////////////////////////////////////////////////////////////////
// RenderSystem
///////////////////////////////////////////////////////////////////////////

class RenderSystem : public Task
{
public:
    RenderSystem();
    virtual ~RenderSystem();
    TaskStatus Update( float delta );
    void Initialize( GLuint screenWidth, GLuint screenHeight );
    void ViewOrtho();
    void ViewPerspective();
    ShaderCache* GetShaderCache() const;

    glm::vec3 Raycast( const glm::vec2& screenCoordinates );

    inline float GetShaderTimer() const;
    void SetRenderTarget( RenderTargetId renderTargetId );

    bool IsScreenshotScheduled() const;
    void BeginCapture();
    void EndCapture();
    bool IsCaptureInProgress() const;

    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;

    unsigned int GetDrawCallCount() const;
    void IncreaseDrawCallCount();
    void ResetDrawCallCount();

    BlendMode GetBlendMode() const;
    void SetBlendMode( BlendMode blendMode );

	void PrintFramebufferInfo( GLuint fboId );

    enum class PostProcessEffect
    {
        BleachBypass,
        Glow,
        Vignette,

        Count
    };
    void EnablePostProcessEffect( PostProcessEffect effect, bool enable );
    bool IsPostProcessEffectEnabled( PostProcessEffect effect );

private:
	void CreateRenderTargets();
    void ClearAll();
    void DrawDebugWindow();
    void InitializePostProcessing();
    void InitializeGlowChain();
    void RenderScene();
    void RenderGlow();
    RenderTarget* GetRenderTarget( RenderTargetId id );
    void ScreenPosToWorldRay( int mouseX, int mouseY, int screenWidth, int screenHeight, const glm::mat4& ViewMatrix, const glm::mat4& ProjectionMatrix, glm::vec3& out_origin, glm::vec3& out_direction );
    IntersectionResult LinePlaneIntersection( const glm::vec3& position, const glm::vec3& direction, const glm::vec3& planePosition, const glm::vec3& planeNormal, glm::vec3& result );

    bool GetScreenshotFilename( std::string& filename ) const;
    SDL_Surface* FlipSurfaceVertical( SDL_Surface* pSurface ) const;
    void SetRenderTarget( RenderTarget* pRenderTarget );
    void TakeScreenshot();
    void TakeScreenshotAux( bool immediate );
    void Capture();

	void InitializeDebug();
	std::string GetRenderbufferParameters( GLuint id );
	std::string GetTextureParameters( GLuint id );
	std::string ConvertInternalFormatToString( GLenum format );

    bool m_ScreenshotScheduled;
    bool m_CaptureInProgress;

    // Shader support
    ShaderCache* m_pShaderCache;

    GLuint m_ScreenWidth;
    GLuint m_ScreenHeight;

    // Used for post-processing effects
    Shader* m_pPostProcessShader;
    VertexBuffer* m_pPostProcessVertexBuffer;
    RenderTargetUniquePtr m_ScreenRenderTarget;

    Shader* m_pGlowShader;
    ShaderUniform* m_pGlowShaderSampler;
    ShaderUniform* m_pGlowShaderDirection;
    VertexBuffer* m_pGlowVertexBuffer;
    RenderTargetUniquePtr m_pGlowRenderTarget;
    RenderTargetUniquePtr m_pGlowBlurRenderTarget[2];

    // Auxiliary render targets
    RenderTargetUniquePtr m_RadarRenderTarget;

    typedef std::map<GLenum, std::string> InternalFormatMap;
    InternalFormatMap mInternalFormatMap;

    float m_ShaderTimer;
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;

    unsigned int m_DrawCallCount;
    BlendMode m_BlendMode;
    InputCallbackToken m_InputCallbackScreenshot;
    InputCallbackToken m_InputCallbackCapture;
    RenderTarget* m_pCurrentRenderTarget;
    bool m_DebugWindowOpen;

    using PostProcessBitSet = std::bitset<static_cast<size_t>( PostProcessEffect::Count )>;
    PostProcessBitSet m_ActivePostProcessEffects;
    std::array<ShaderUniform*, static_cast<size_t>( PostProcessEffect::Count )> m_PostProcessShaderUniforms;
};

inline ShaderCache* RenderSystem::GetShaderCache() const
{
    return m_pShaderCache;
}

inline float RenderSystem::GetShaderTimer() const
{
    return m_ShaderTimer;
}

inline bool RenderSystem::IsScreenshotScheduled() const
{
    return m_ScreenshotScheduled;
}

inline void RenderSystem::BeginCapture()
{
    m_CaptureInProgress = true;
}

inline void RenderSystem::EndCapture()
{
    m_CaptureInProgress = false;
}

inline bool RenderSystem::IsCaptureInProgress() const
{
    return m_CaptureInProgress;
}

inline const glm::mat4& RenderSystem::GetViewMatrix() const
{
    return m_ViewMatrix;
}

inline const glm::mat4& RenderSystem::GetProjectionMatrix() const
{
    return m_ProjectionMatrix;
}

inline unsigned int RenderSystem::GetDrawCallCount() const
{
    return m_DrawCallCount;
}

inline void RenderSystem::IncreaseDrawCallCount()
{
    m_DrawCallCount++;
}

inline void RenderSystem::ResetDrawCallCount()
{
    m_DrawCallCount = 0;
}

inline BlendMode RenderSystem::GetBlendMode() const
{
    return m_BlendMode;
}

inline bool RenderSystem::IsPostProcessEffectEnabled( PostProcessEffect effect )
{
    return m_ActivePostProcessEffects[ static_cast<size_t>( effect ) ];
}

///////////////////////////////////////////////////////////////////////////
// Auxiliary structures for VBO manipulation
///////////////////////////////////////////////////////////////////////////

struct VboFloat4
{
    VboFloat4()
        : x( 0.0f )
        , y( 0.0f )
        , z( 0.0f )
        , w( 0.0f ){};
    VboFloat4( float inX, float inY, float inZ, float inW )
        : x( inX )
        , y( inY )
        , z( inZ )
        , w( inW ){};
    float x, y, z, w;
};
typedef std::vector<VboFloat4> VboFloat4Vec;

struct VboFloat3
{
    VboFloat3()
        : x( 0.0f )
        , y( 0.0f )
        , z( 0.0f ){};
    VboFloat3( float inX, float inY, float inZ )
        : x( inX )
        , y( inY )
        , z( inZ ){};
    float x, y, z;
};
typedef std::vector<VboFloat3> VboFloat3Vec;

struct VboFloat2
{
    VboFloat2()
        : x( 0.0f )
        , y( 0.0f ){};
    VboFloat2( float inX, float inY )
        : x( inX )
        , y( inY ){};
    float x, y;
};
typedef std::vector<VboFloat2> VboFloat2Vec;
}
