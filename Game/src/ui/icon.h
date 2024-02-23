// Copyright 2024 Pedro Nunes
//
// This file is part of Hexterminate.
//
// Hexterminate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hexterminate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hexterminate. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "ui/image.h"

namespace Hexterminate::UI
{

// TODO: At the moment this is just a shim to the underlying Image type.
// All icons should be in an atlas.
class Icon : public Image
{
public:
    Icon( const std::string& name, const std::string& path = "" );
    virtual ~Icon() override;
};

} // namespace Hexterminate::UI
