//
//  View.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "View.h"
#include "../editor/Editor.h"
#include "../converter/Converter.h"

#include <iostream>

using namespace gsl;

namespace brick
{
    
View::~View() {
    if (hasParent()) {
        parent_->removeChild(this);
    }
}
    
View::View(size_t viewId, UpdateCb cb)
    : update_cb_(cb)
    , viewId_(viewId)
    , parent_(nullptr)
    , sel_(Range()) {
    editor_ = std::make_unique<Editor>(viewId);
}
        
View::View(size_t viewId, const std::string& filePath, UpdateCb cb)
    : View(viewId, cb) {
    // FIXME: - read file content
    filePath_ = filePath;
    editor_ = std::make_unique<Editor>(viewId);
}
    
View::View(size_t viewId, View* parent, UpdateCb cb)
    : View(viewId, cb) {
    parent_ = parent;
    parent_->children_.push_back(this);
    auto deltas = editor_->merge(*parent->editor_);
    update_cb_(this, deltas);
}
    
void View::scroll(size_t begRow, size_t endRow) {
    visibleRange_.first = begRow;
    visibleRange_.second = endRow;
    
    viewSize_ = std::max(viewSize_, endRow - begRow);
}
 
void View::insert(const detail::CodePointList& cplist) {
//    if (sel_.length > 0) {
//        editor_->erase(sel_);
//        sel_.length = 0;
//    }
    editor_->insert(cplist, sel_.location);
    sel_.offset(static_cast<int>(cplist.size()));
    
    auto src = std::vector<View*> {this};
    update(src);
}
    
void View::erase() {
    editor_->erase(sel_);
    sel_.length = 0;
    
    auto src = std::vector<View*> {this};
    update(src);
}
  
void View::select(Range sel) {
    sel_ = sel;
}
    
std::map<size_t, detail::CodePointList> View::regionImpl(size_t begRow, size_t endRow) {
    return editor_->region(begRow, endRow);
}
    
void View::save()  {
    if (filePath_.empty()) {
        throw std::invalid_argument("Invalid file path");
    }
    save(filePath_);
}
    
void View::save(const std::string& filePath) {
}
  
void View::update(std::vector<View*>& src) {
    // 1. update self
    bool selfNotExist = std::find(src.begin(), src.end(), this) == src.end();
    if (selfNotExist) {
        auto deltas = editor_->merge(*src.front()->editor_);
        update_cb_(this, deltas);
        src.push_back(this);
    }
    
    // 2. popagate
    if (hasChildren() || hasParent()) {
        if (hasParent()) {
            auto notExist = std::find(src.begin(), src.end(), parent_) == src.end();
            if (notExist) {
                parent_->update(src);
            }
        }
        
        for (auto child : children_) {
            auto notExist = std::find(src.begin(), src.end(), child) == src.end();
            if (notExist) {
                child->update(src);
            }
        }
    }
    
    if (src.front() == this || selfNotExist) {
        //FIXME: consider undo
        
        // clear revisions when view's state was already async with editor's
        editor_->clearRevisions();
    }
}
    
}   // namespace brick
