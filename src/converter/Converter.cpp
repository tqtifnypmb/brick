//
//  Encoder.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 08/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Converter.h"

using namespace brick::detail;

namespace brick
{

CodePointList ASCIIConverter::encode(gsl::span<const char> bytes) {
    CodePointList cplist;
    cplist.reserve(bytes.length());
    for (auto byte : bytes) {
        auto cp = static_cast<uint8_t>(byte);
        cplist.push_back(CodePoint {cp});
    }
    return cplist;
}
    
ASCIIConverter::result_type ASCIIConverter::decode(const detail::CodePointList& cplist) {
    result_type str;
    for (auto& cp : cplist) {
        str.append(1, cp[0]);
    }
    return str;
}
    
size_t ASCIIConverter::numOfLine(const detail::CodePointList& cplist) {
    size_t line = 0;
    for (auto& cp : cplist) {
        auto ch = static_cast<uint8_t>(cp[0]);
        if (ch == '\n') {
            line += 1;
        }
    }
    return line;
}
    
}   // namespace brick
