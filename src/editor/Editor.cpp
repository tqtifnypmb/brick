//
//  editor.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Editor.h"
#include "../rope/Rope.h"
#include "../view/View.h"
#include "../converter/Converter.h"

#include <limits>
#include <iostream>

using namespace brick::detail;

namespace brick
{
    
Editor::Editor(View* view, const CodePointList& cplist)
    : view_(view)
    , rope_(std::make_unique<Rope>())
    , engine_(view->viewId(), rope_.get()) {
    if (!cplist.empty()) {
        rope_->insert(cplist, 0);
    }
    updateLines(0, cplist, 0);
}
   
Editor::Editor(View* view)
    : Editor(view, CodePointList()) {}
    
void Editor::insert(const CodePointList &cplist, size_t pos) {
    auto oldSize = rope_->size();
    auto delta = engine_.insert(cplist, pos);
    updateLines(pos, cplist, oldSize);
}
    
void Editor::erase(Range range) {
    auto delta = engine_.erase(range);
    updateLines(range);
}
    
void Editor::undo() {
}

namespace
{
    bool isNewLine(const CodePoint& cp) {
        char ch = static_cast<char>(cp[0]);
        return ch == '\n';
    }
}
    
void Editor::updateLines(size_t pos, const detail::CodePointList& cplist, size_t oldRopeSize) {
    std::vector<size_t> lines;
    for (auto i = 0; i < cplist.size(); ++i) {
        const auto& cp = cplist[i];
        if (isNewLine(cp)) {
            lines.push_back(i);
        }
    }

    if (linesIndex_.empty()) {
        Expects(pos == 0);
        
        linesIndex_ = lines;
        return;
    }
    
    auto closest = std::lower_bound(linesIndex_.begin(), linesIndex_.end(), pos);
    
    if (closest == linesIndex_.end()) {     // appending
        if (!lines.empty()) {
            auto prevIndex = oldRopeSize;
            std::for_each(lines.begin(), lines.end(), [prevIndex](auto& idx) { idx += prevIndex; });
            linesIndex_.insert(linesIndex_.end(), lines.begin(), lines.end());
        }
        return;
    }
    
    std::advance(closest, -1);
    if (closest == linesIndex_.begin()) {   // prepending
        auto prevIndex = cplist.size();
        std::for_each(lines.begin(), lines.end(), [prevIndex](auto& idx) { idx += prevIndex; });
        if (!lines.empty()) {
            linesIndex_.insert(linesIndex_.begin(), lines.begin(), lines.end());
        }
    } else {
        for (auto iter = closest; iter != linesIndex_.end(); ++iter) {
            *iter += cplist.size();
        }
        linesIndex_.insert(closest, lines.begin(), lines.end());
    }
}
    
void Editor::updateLines(Range r) {
    auto beg = std::upper_bound(linesIndex_.begin(), linesIndex_.end(), r.location);
    if (beg == linesIndex_.begin()) {
        throw std::out_of_range("updateLines erase");
    }
    std::advance(beg, -1);
    
    auto end = std::lower_bound(linesIndex_.begin(), linesIndex_.end(), r.maxLocation());
    
    auto p = linesIndex_.erase(beg, end);
    while (p != linesIndex_.end()) {
        *p -= r.length;
    }
}
    
Editor::DeltaList Editor::convertEngineDelta(const Engine::DeltaList& deltas) {
    if (deltas.empty()) return {};
    
    auto indexToLine = [this](int index) {
        auto iter = std::lower_bound(linesIndex_.begin(), linesIndex_.end(), index);
        return std::distance(linesIndex_.begin(), iter);
    };
    
    Editor::DeltaList ret;
    for (const auto& delta : deltas) {
        auto begRow = indexToLine(delta.range().location);
        auto endIndex = delta.range().location + delta.affectLength();
        auto endRow = indexToLine(endIndex);
        ret.emplace_back(delta, begRow, endRow);
    }

    return ret;
}
    
Editor::DeltaList Editor::merge(const Editor& other) {
    if (rope_->empty() && engine_.revisions().empty()) {
        auto rope = Rope(*other.rope_);
        rope_->swap(rope);
        linesIndex_.clear();
        if (!other.linesIndex_.empty()) {
            linesIndex_ = other.linesIndex_;
        }
        
        Engine::DeltaList ret;
        auto str = rope_->string();
        auto cplist = ASCIIConverter::encode(gsl::make_span(str.c_str(), str.length()));
        ret.emplace_back(engine_.authorId(), engine_.nextRevId(), Revision::Operation::insert, Range(0, static_cast<int>(rope_->size())), cplist);
        return convertEngineDelta(ret);
    } else {
        auto deltas = engine_.sync(other.engine_);
        return convertEngineDelta(deltas);
    }
}
    
std::map<size_t, CodePointList> Editor::region(size_t begLine, size_t endLine) {
    Expects(begLine < endLine);
    
    if (linesIndex_.empty() && !rope_->empty()) {       // single line
        std::map<size_t, CodePointList> ret;
        for (const auto& cp : *rope_) {
            ret[0].push_back(cp);
        }
        return ret;
    }
    
    if (begLine >= linesIndex_.size()) {    // out of range
        return {};
    }
    
    auto beg = linesIndex_.begin();
    std::advance(beg, begLine);
    
    auto end = linesIndex_.begin();
    std::advance(end, std::min(endLine, linesIndex_.size()));
    
    size_t line = begLine;
    std::map<size_t, CodePointList> ret;
    for (auto iter = beg; iter != end; ++iter) {
        auto endIndex = *iter;
        auto begIndex = endIndex;
        
        auto prev = iter - 1;
        if (iter != linesIndex_.begin()) {
            begIndex = *prev + 1;
        } else {
            begIndex = 0;
        }

        auto ropeIterBeg = rope_->iterator(begIndex);
        for (auto i = begIndex; i <= endIndex; ++i) {
            ret[line].push_back(*ropeIterBeg);
            ++ropeIterBeg;
            
        }
        ++line;
    }
    
    if (end == linesIndex_.end() && endLine > linesIndex_.size() && (linesIndex_.back() + 1) < rope_->size()) {
        auto begIndex = linesIndex_.back() + 1;
        auto endIndex = rope_->size();
        auto ropeIterBeg = rope_->iterator(begIndex);
        for (auto i = begIndex; i < endIndex; ++i) {
            
            ret[line].push_back(*ropeIterBeg);
            ++ropeIterBeg;
        }
    }
    return ret;
}
    
}
