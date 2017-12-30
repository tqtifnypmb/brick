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
#include <map>

namespace brick
{
 
class Rope;
class View;
class Editor {
public:
    Editor(View* view, const detail::CodePointList& cplist);
    Editor(View* view);
    
    template <class Converter>
    void insert(gsl::span<const char> bytes, size_t pos);
    void insert(const detail::CodePointList& cplist, size_t pos);
    void erase(Range range);
    void undo();
    
    void merge(const Editor& editor);
    
    std::map<size_t, detail::CodePointList> region(size_t beginRow, size_t endRow);
    
private:
    std::map<size_t, detail::CodePointList> region(size_t initIndex, size_t initRow, size_t begRow, size_t endRow);
    
    View* view_;
    std::unique_ptr<Rope> rope_;
    Engine engine_;
    std::map<size_t, size_t> linesIndex_;     // line num -> index;
};
    
template <class Converter>
void Editor::insert(gsl::span<const char> bytes, size_t pos) {
    insert(Converter::encode(bytes), pos);
}
    
}   // namespace brick
