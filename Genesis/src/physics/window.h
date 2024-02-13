// Copyright 2018 Pedro Nunes
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

namespace Genesis
{
namespace Physics
{

class Simulation;

class Window
{
public:
	Window( Simulation* pSimulation );
	~Window();

	void Update( float delta );
private:
	Simulation* m_pSimulation;
	bool m_Open;
	bool m_ModeWireframe;
	bool m_ModeAABB;
	bool m_ModeTransforms;
	bool m_ModeRayTests;
	bool m_ModeContactPoints;
};

} // namespace Physics
} // namespace Genesis
