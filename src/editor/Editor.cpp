//
//  editor.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 14/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Editor.h"
#include "../converter/Converter.h"
#include "../rope/Rope.h"

namespace brick
{
    
Editor::Editor(View* view, gsl::span<const char> text)
    : view_(view)
    , rope_() {
    if (!text.empty()) {
        rope_->insert<ASCIIConverter>(text, 0);
    }
}
    
}
