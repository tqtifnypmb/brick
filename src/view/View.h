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
#include <utility>
#include <gsl/gsl>

namespace brick
{
    
class Editor;
class View {
public:
    
    View(size_t viewId);
    View(size_t viewId, gsl::span<const char> text, Range sel = Range());
    View(size_t viewId, const char* filePath);
    
    void scroll(size_t begRow, size_t endRow);
    void insert(gsl::span<const char> bytes);
    void erase();
    void undo();
    void select(Range sel);
    std::string region(size_t begRow, size_t endRow);
    
    size_t viewId() const {
        return viewId_;
    }
    
private:
    std::pair<size_t, size_t> visibleRange_;
    Range sel_;
    size_t viewId_;
    std::unique_ptr<Editor> editor_;
};
    
}
