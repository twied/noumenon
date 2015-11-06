/*
 * Noumenon: A dynamic, strongly typed script language.
 * Copyright (C) 2015 Tim Wiederhake
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef RUNTIME_H_
#define RUNTIME_H_

#include "Value.h"

namespace noumenon {
namespace rtl {

struct Print : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct Println : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct Typeof : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct Range : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct Length : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct List : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct Require : public FunctionValue {
    std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

} /* namespace rtl */
} /* namespace noumenon */

#endif /* RUNTIME_H_ */
