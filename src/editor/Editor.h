//
//  editor.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../crdt/Engine.h"
#include "../types.h"

#include <memory>
#include <gsl/gsl>

namespace brick
{
 
class Rope;
class View;
class Editor {
public:
    Editor(View* view, const detail::CodePointList& cplist);
    
    void scroll();
    void insert(const detail::CodePointList& cplist, size_t pos);
    void erase(Range range);
    void undo();
    
    std::string region(size_t beginRow, size_t endRow);
    
private:
    View* view_;
    Engine engine_;
    std::unique_ptr<Rope> rope_;
};
    
}
