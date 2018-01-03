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
    
    using UpdateCb = std::function<void(size_t viewId, const Engine::Delta&)>;
    
    View(size_t viewId, UpdateCb cb);
    
    template <class Converter>
    View(size_t viewId, gsl::span<const char> text, UpdateCb cb, Range sel = Range());
    View(size_t viewId, View* parent, UpdateCb cb);
    
    View(size_t viewId, const detail::CodePointList& cplist, UpdateCb cb, Range sel = Range());
    View(size_t viewId, const std::string& filePath, UpdateCb cb);
    
    ~View();
    
    void scroll(size_t begRow, size_t endRow);
    
    template<class Converter>
    void insert(gsl::span<const char> bytes);
    void insert(const detail::CodePointList& cplist);
    
    void erase();
    void undo();
    void select(Range sel);
    Range selection() const {
        return sel_;
    }
    
    void save();
    void save(const std::string& filePath);
    
    template <class Converter>
    std::map<size_t, std::string> region() {
        return region<Converter>(visibleRange_.first, visibleRange_.second);
    }
    
    template <class Converter>
    std::map<size_t, std::string> region(size_t begRow, size_t endRow);
    
    size_t viewId() const {
        return viewId_;
    }
    
    size_t viewSize() const {
        return viewSize_;
    }
    
    bool hasChildren() const {
        return !children_.empty();
    }
    
    void removeChild(const View* child) {
        auto found = std::find_if(children_.begin(), children_.end(), [child](auto v) { return v->viewId_ == child->viewId_; });
        if (found != children_.end()) {
            children_.erase(found);
        }
    }
    
    const std::vector<View*>& children() const {
        return children_;
    }
    
    const std::string filePath() const {
        return filePath_;
    }
    
    bool hasParent() const {
        return parent() != nullptr;
    }
    
    const View* parent() const {
        return parent_;
    }
    
private:
    void update(View* src);
    
    std::map<size_t, detail::CodePointList> regionImpl(size_t begRow, size_t endRow);
    std::map<size_t, detail::CodePointList> regionImpl() {
        return regionImpl(visibleRange_.first, visibleRange_.second);
    }
    
    UpdateCb update_cb_;
    
    std::pair<size_t, size_t> visibleRange_;
    Range sel_;
    size_t viewId_;
    size_t viewSize_;
    View* parent_;
    std::vector<View*> children_;
    std::unique_ptr<Editor> editor_;
    std::string filePath_;
};
    
template <class Converter>
View::View(size_t viewId, gsl::span<const char> text, UpdateCb cb, Range sel)
    : View(viewId, Converter::encode(text), cb) {}
    
template<class Converter>
void View::insert(gsl::span<const char> bytes) {
    insert(Converter::encode(bytes));
}
    
template <class Converter>
std::map<size_t, std::string> View::region(size_t begRow, size_t endRow) {
    auto lines = regionImpl(begRow, endRow);
    std::map<size_t, std::string> ret;
    for (const auto& line : lines) {
        ret[line.first] = Converter::decode(line.second);
    }
    return ret;
}
    
}
