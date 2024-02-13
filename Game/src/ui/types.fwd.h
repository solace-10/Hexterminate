// Copyright 2021 Pedro Nunes
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

#include <memory>
#include <vector>

namespace Hexterminate::UI
{

class Button;
using ButtonSharedPtr = std::shared_ptr<Button>;

class Checkbox;
using CheckboxSharedPtr = std::shared_ptr<Checkbox>;

class Design;
using DesignUniquePtr = std::unique_ptr<Design>;

class Element;
using ElementSharedPtr = std::shared_ptr<Element>;
using ElementWeakPtr = std::weak_ptr<Element>;

class Image;
using ImageSharedPtr = std::shared_ptr<Image>;

class InputArea;
using InputAreaSharedPtr = std::shared_ptr<InputArea>;

class IToggleable;
using IToggleableSharedPtr = std::shared_ptr<IToggleable>;
using IToggleableWeakPtr = std::weak_ptr<IToggleable>;
using IToggleableVector = std::vector<IToggleableWeakPtr>;

class Meter;
using MeterSharedPtr = std::shared_ptr<Meter>;

class Panel;
using PanelSharedPtr = std::shared_ptr<Panel>;

class RadioButton;
using RadioButtonSharedPtr = std::shared_ptr<RadioButton>;

class ScrollingElement;
using ScrollingElementSharedPtr = std::shared_ptr<ScrollingElement>;

class Slider;
using SliderSharedPtr = std::shared_ptr<Slider>;

class Text;
using TextSharedPtr = std::shared_ptr<Text>;

class ToggleGroup;
using ToggleGroupSharedPtr = std::shared_ptr<ToggleGroup>;
using ToggleGroupWeakPtr = std::weak_ptr<ToggleGroup>;

class Window;
using WindowSharedPtr = std::shared_ptr<Window>;

} // namespace Hexterminate::UI
