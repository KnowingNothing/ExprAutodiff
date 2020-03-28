/*
 * MIT License
 * 
 * Copyright (c) 2020 Size Zheng

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <vector>


namespace Boost {

namespace Internal {

enum class TypeCode : uint8_t {
    Int,
    UInt,
    Float,
    String,
    Handle
};

/**
 * This maybe a heavy implementation
 */ 
class LanesList {
 private:
    std::vector<uint16_t> lanes_list;
 public:
    LanesList() {}

    LanesList(LanesList &other) : lanes_list(other.lanes_list) {}

    LanesList(LanesList &&other) : lanes_list(std::move(other.lanes_list)) {}

    LanesList(std::vector<uint16_t> _lanes_list) : lanes_list(_lanes_list) {};

    LanesList(std::vector<uint16_t> &&_lanes_list) {
        lanes_list = std::move(_lanes_list);
    }

    LanesList &push_back(uint16_t v) {
        lanes_list.push_back(v);
        return *this;
    }

    uint16_t pop_back() {
        uint16_t ret = lanes_list.back();
        lanes_list.pop_back();
        return ret;
    }

    size_t size() {
        return lanes_list.size();
    }

    uint16_t &operator[](size_t pos) {
        return lanes_list[pos];
    }

    LanesList &operator=(LanesList &other) {
        this->lanes_list = other.lanes_list;
        return *this;
    }

    LanesList &operator=(LanesList &&other) {
        this->lanes_list = std::move(other.lanes_list);
        return *this;
    }
};


class Type {
 public:
    TypeCode code;
    uint16_t bits;
    LanesList lanes_list;

    Type() : lanes_list() {};

    Type(TypeCode _code, uint16_t _bits, LanesList _lanes_list) : code(_code),
        bits(_bits), lanes_list(_lanes_list) {}

    Type(Type &other) : code(other.code), bits(other.bits), lanes_list(other.lanes_list) {}

    Type(Type &&other) : code(other.code), bits(other.bits), lanes_list(std::move(other.lanes_list)) {}

    Type &operator=(Type &other) {
        this->code = other.code;
        this->bits = other.bits;
        this->lanes_list = other.lanes_list;
        return *this;
    }

    Type &operator=(Type &&other) {
        this->code = other.code;
        this->bits = other.bits;
        this->lanes_list = std::move(other.lanes_list);
        return *this;
    }

    size_t dim() {
        return lanes_list.size();
    }
};

}  // namespace Internal

}  // namespace Boost