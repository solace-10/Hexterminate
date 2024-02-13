// Copyright 2017 Pedro Nunes
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

#include "timer.h"

namespace Genesis
{

Timer::Timer()
    : m_TimePoint( std::chrono::high_resolution_clock::now() )
    , m_Delta( 0.0f )
{
    m_TimePoint = std::chrono::high_resolution_clock::now();
}

void Timer::Update()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = now - m_TimePoint;
    m_TimePoint = now;
    m_Delta = delta.count();
}

} // namespace Genesis