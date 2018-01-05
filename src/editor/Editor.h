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
    using DeltaList = std::vector<std::pair<Range, Revision::Operation>>;
    
    Editor(View* view, const detail::CodePointList& cplist);
    explicit Editor(View* view);
    
    template <class Converter>
    void insert(gsl::span<const char> bytes, size_t pos);
    void insert(const detail::CodePointList& cplist, size_t pos);
    void erase(Range range);
    void undo();
    
    Editor::DeltaList merge(const Editor& editor);
    
    std::map<size_t, detail::CodePointList> region(size_t beginRow, size_t endRow);
    
    void clearRevisions() {
        engine_.revisions().clear();
    }
    
private:
    std::map<size_t, detail::CodePointList> region(size_t initIndex, size_t initRow, size_t begRow, size_t endRow);
    void adjust(const Engine::DeltaList& delta);
    Editor::DeltaList convertEngineDelta(const Engine::DeltaList& deltas);
    
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
