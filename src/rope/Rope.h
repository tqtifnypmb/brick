//
//  rope.hpp
//  Fans
//
//  Created by tqtifnypmb on 06/12/2017.
//  Copyright © 2017 newsjet. All rights reserved.
//

#pragma once

#include <memory>
#include <type_traits>

namespace brick
{
    
template <class Unit>
class Rope {
public:
    template <class V>
    struct Node {
        Node<V>* left;
        Node<V>* right;
        size_t weight;
        bool is_leaf;
        
        Leaf<V>* asLeaf() {
            return static_cast<Leaf<V>*>(this);
        }
    };
    
    template <class V>
    struct Leaf: public Node<V> {
        std::vector<V> values;
    };

    Rope(Rope&& r);
    
//    template <class T>
//    Rope(std::enable_if_t<std::is_same_v<T, Node<Unit>>,
//                          T>&& left,
//         std::enable_if_t<std::is_same_v<T, Node<Unit>>,
//                          T>&& right);
    
    
    Rope(const Rope&) = delete;
    Rope& operator=(const Rope&) = delete;
    
private:
    void index(int i);
    void concat(const Rope& l);
    
    std::unique_ptr<Node<Unit>> root_ {nullptr};
};

}   // namespace brick
