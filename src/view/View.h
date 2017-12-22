//
//  View.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Range.h"

#include <memory>
#include <gsl/gsl>

namespace brick
{
    
class Editor;
class View {
public:
    
    View(size_t viewId);
    View(size_t viewId, gsl::span<const char> text, Range sel = Range());
    View(size_t viewId, const char* filePath);
    
    void scroll(Range visibleRange);
    void insert(gsl::span<const char> text);
    void erase();
    void undo();
    void select(Range sel);
    
    size_t viewId() const {
        return viewId_;
    }
    
    Editor* editor() const {
        return editor_.get();
    }
    
private:
    Range visibleRange_;
    Range seletion_;
    size_t viewId_;
    std::unique_ptr<Editor> editor_;
};
    
}
