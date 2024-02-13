// Copyright 2014 Pedro Nunes
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

#include "../resourcemanager.h"
#include "SDL.h"

namespace Genesis
{

class ResourceImage : public ResourceGeneric
{
public:
    ResourceImage( const Filename& filename );
    virtual ~ResourceImage(){};
    virtual ResourceType GetType() const override;
    virtual void Preload() override;
    virtual bool Load() override;

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetTexture() const;

    bool IsMipMapped() const;
    void EnableMipMapping( bool state );

private:
    SDL_Surface* CreateSurface( const std::string& filename );
    void CreateTexture( uint32_t internalFormat, uint32_t format );
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_TextureSlot;
    bool m_MipMapped;
    SDL_Surface* m_pTemporarySurface; // Set during the async preload
};

inline ResourceType ResourceImage::GetType() const { return ResourceType::Texture; }
inline uint32_t ResourceImage::GetWidth() const { return m_Width; }
inline uint32_t ResourceImage::GetHeight() const { return m_Height; }
inline uint32_t ResourceImage::GetTexture() const { return m_TextureSlot; }
inline bool ResourceImage::IsMipMapped() const { return m_MipMapped; }
}