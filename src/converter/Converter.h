//
//  Encoder.h
//  PretendToWork
//
//  Created by tqtifnypmb on 08/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include "../types.h"

#include <string>
#include <gsl/gsl>

namespace brick
{

struct ASCIIConverter {
    typedef std::string result_type;
    
    static detail::CodePointList encode(gsl::span<const char> bytes);
    static result_type decode(const detail::CodePointList& cplist);
    static size_t numOfLine(const detail::CodePointList& cplist);
};
    
struct UTF8Converter {
    typedef std::string result_type;
    
    static detail::CodePointList encode(gsl::span<const char> bytes);
    static result_type decode(const detail::CodePointList& cplist);
    static size_t numOfLine(const detail::CodePointList& cplist);
};
    
}
