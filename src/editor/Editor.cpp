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

using namespace brick::detail;

namespace brick
{
    
Editor::Editor(View* view, const CodePointList& cplist)
    : view_(view)
    , rope_()
    , engine(view->viewId()){
    if (!cplist.empty()) {
        rope_->insert(cplist, 0);
    }
}
   
void Editor::insert(const CodePointList &cplist, size_t pos) {
    //engine_.insert(<#gsl::span<const char> bytes#>, <#size_t pos#>)
}
    
std::string Editor::region(size_t beginRow, size_t endRow) {
    return "";
}
    
}
