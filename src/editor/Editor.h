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
class Editor {
public:
    
    using DeltaList = std::vector<std::tuple<Revision, size_t, size_t>>;     // [(delta, begRow, endRow)]
    using SyncCb = std::function<void(const Editor::DeltaList& deltaList)>;
    
    Editor(size_t viewId, const detail::CodePointList& cplist, SyncCb syncCb);
    Editor(size_t viewId, SyncCb syncCb);
    
    template <class Converter>
    void insert(gsl::span<const char> bytes, size_t pos);
    void insert(const detail::CodePointList& cplist, size_t pos);
    void erase(Range range);
    void undo();
    
    void sync(Editor& editor);
    
    std::map<size_t, detail::CodePointList> region(size_t begLine, size_t endLine);
    
    void clearRevisions() {
        engine_.revisions().clear();
    }
    
private:
    Editor::DeltaList convertEngineDelta(const Engine::DeltaList& deltas);
    
    void updateLines(size_t pos, const detail::CodePointList& cplist);
    void updateLines(Range r);
    
    std::unique_ptr<Rope> rope_;
    Engine engine_;
    std::vector<size_t> linesIndex_;
    SyncCb sync_cb_;
};
    
template <class Converter>
void Editor::insert(gsl::span<const char> bytes, size_t pos) {
    insert(Converter::encode(bytes), pos);
}
    
}   // namespace brick
