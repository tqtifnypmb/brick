//
//  View.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Range.h"
#include "../editor/Editor.h"

#include <memory>
#include <gsl/gsl>

namespace brick
{
    
class Editor;
class View {
public:
    View(size_t viewId, gsl::span<const char> text, Range sel = Range());
    
    void scroll();
    void insert();
    void erase();
    void undo();
    void select(Range sel);
    
private:
    Range seletion_;
    std::unique_ptr<Editor> editor_;
};
    
}
