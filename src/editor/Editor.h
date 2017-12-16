//
//  editor.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../crdt/Engine.h"

#include <memory>
#include <gsl/gsl>

namespace brick
{
 
class Rope;
class View;
class Editor {
public:
    Editor(View* view, gsl::span<const char> text);
    
    void scroll();
    void insert();
    void erase();
    void undo();
    
private:
    View* view_;
    Engine engine;
    std::unique_ptr<Rope> rope_;
};
    
}
