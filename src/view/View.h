//
//  View.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../rope/Range.h"
#include "../types.h"
#include "../editor/Editor.h"

#include <memory>
#include <utility>
#include <gsl/gsl>
#include <map>
#include <string>

namespace brick
{
    
class View {
public:
    
    View(size_t viewId);
    
    template <class Converter>
    View(size_t viewId, gsl::span<const char> text, Range sel = Range());
    
    View(size_t viewId, const detail::CodePointList& cplist, Range sel = Range());
    View(size_t viewId, const std::string& filePath);
    
    void scroll(size_t begRow, size_t endRow);
    
    template<class Converter>
    void insert(gsl::span<const char> bytes);
    void insert(const detail::CodePointList& cplist);
    
    void erase();
    void undo();
    void select(Range sel);
    std::map<size_t, detail::CodePointList> region(size_t begRow, size_t endRow);
    
    size_t viewId() const {
        return viewId_;
    }
    
    size_t viewSize() const {
        return viewSize_;
    }
    
private:
    std::pair<size_t, size_t> visibleRange_;
    Range sel_;
    size_t viewId_;
    size_t viewSize_;
    std::unique_ptr<Editor> editor_;
};
    
template <class Converter>
View::View(size_t viewId, gsl::span<const char> text, Range sel)
    : View(viewId, Converter::encode(text)) {}
    
template<class Converter>
void View::insert(gsl::span<const char> bytes) {
    insert(Converter::encode(bytes));
}
    
}
