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

#ifndef VALUE_H_
#define VALUE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace noumenon {

class Program;
struct Statement;
enum class BinaryOperator;
enum class UnaryOperator;

class ValueWalker;

struct Value {
    virtual ~Value();

    virtual std::shared_ptr<Value> walk(ValueWalker&) = 0;
    virtual bool isTrue();

    virtual std::shared_ptr<Value> doSelect(std::shared_ptr<Value>);
    virtual std::shared_ptr<Value> doUnary(const UnaryOperator&);
    virtual std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
    virtual std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
    virtual void doModify(std::shared_ptr<Value> index, std::shared_ptr<Value> value);
    virtual unsigned long long getLength();
    virtual std::shared_ptr<Value> getKey(const unsigned long long&);
    virtual std::shared_ptr<Value> getValue(const unsigned long long&);
};

struct ArrayValue : public Value {
    std::vector<std::shared_ptr<Value>> values;

    ArrayValue();
    ArrayValue(const std::vector<std::shared_ptr<Value>>&);
    std::shared_ptr<Value> walk(ValueWalker&);
    virtual std::shared_ptr<Value> doSelect(std::shared_ptr<Value>);
    virtual std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
    virtual void doModify(std::shared_ptr<Value> index, std::shared_ptr<Value> value);
    virtual unsigned long long getLength();
    virtual std::shared_ptr<Value> getKey(const unsigned long long&);
    virtual std::shared_ptr<Value> getValue(const unsigned long long&);
};

struct BoolValue : public Value {
    bool value;

    BoolValue(const bool& value);
    std::shared_ptr<Value> walk(ValueWalker&);
    bool isTrue();
    std::shared_ptr<Value> doUnary(const UnaryOperator&);
    std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
};

struct FloatValue : public Value {
    double value;

    FloatValue(const double& value);
    std::shared_ptr<Value> walk(ValueWalker&);
    std::shared_ptr<Value> doUnary(const UnaryOperator&);
    std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
};

struct FunctionValue : public Value {
    std::vector<std::u32string> parameters;
    std::vector<std::shared_ptr<Statement>> statements;

    FunctionValue();
    FunctionValue(const std::vector<std::u32string>&, const std::vector<std::shared_ptr<Statement>>&);
    std::shared_ptr<Value> walk(ValueWalker&);
    virtual std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
    virtual std::shared_ptr<Value> doCall(Program&, std::vector<std::shared_ptr<Value>>&);
};

struct IntValue : public Value {
    signed long long value;

    IntValue(const signed long long& value);
    std::shared_ptr<Value> walk(ValueWalker&);
    std::shared_ptr<Value> doUnary(const UnaryOperator&);
    std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
};

struct NullValue : public Value {
    static std::shared_ptr<NullValue> singleton;

    std::shared_ptr<Value> walk(ValueWalker&);
    std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
};

struct ObjectValue : public Value {
    std::map<std::u32string, std::shared_ptr<Value>> values;

    ObjectValue();
    ObjectValue(const std::map<std::u32string, std::shared_ptr<Value>>&);
    std::shared_ptr<Value> walk(ValueWalker&);
    virtual std::shared_ptr<Value> doSelect(std::shared_ptr<Value>);
    virtual std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
    virtual void doModify(std::shared_ptr<Value> index, std::shared_ptr<Value> value);
    virtual unsigned long long getLength();
    virtual std::shared_ptr<Value> getKey(const unsigned long long&);
    virtual std::shared_ptr<Value> getValue(const unsigned long long&);
};

struct StringValue : public Value {
    static std::string UTF32toUTF8(const std::u32string&);
    static std::u32string UTF8toUTF32(const std::string&);

    std::u32string value;

    StringValue();
    StringValue(const std::u32string& value);
    std::shared_ptr<Value> walk(ValueWalker&);
    std::shared_ptr<Value> doSelect(std::shared_ptr<Value>);
    std::shared_ptr<Value> doBinary(const BinaryOperator&, std::shared_ptr<Value>);
    unsigned long long getLength();
    std::shared_ptr<Value> getKey(const unsigned long long&);
    std::shared_ptr<Value> getValue(const unsigned long long&);
};

class ValueWalker {
public:
    virtual ~ValueWalker() = 0;

    virtual std::shared_ptr<Value> value(ArrayValue& node) = 0;
    virtual std::shared_ptr<Value> value(BoolValue& node) = 0;
    virtual std::shared_ptr<Value> value(FloatValue& node) = 0;
    virtual std::shared_ptr<Value> value(FunctionValue& node) = 0;
    virtual std::shared_ptr<Value> value(IntValue& node) = 0;
    virtual std::shared_ptr<Value> value(NullValue& node) = 0;
    virtual std::shared_ptr<Value> value(ObjectValue& node) = 0;
    virtual std::shared_ptr<Value> value(StringValue& node) = 0;
};

class DefaultValueWalker : public ValueWalker {
public:
    virtual ~DefaultValueWalker() = 0;

    virtual std::shared_ptr<Value> value(ArrayValue& node);
    virtual std::shared_ptr<Value> value(BoolValue& node);
    virtual std::shared_ptr<Value> value(FloatValue& node);
    virtual std::shared_ptr<Value> value(FunctionValue& node);
    virtual std::shared_ptr<Value> value(IntValue& node);
    virtual std::shared_ptr<Value> value(NullValue& node);
    virtual std::shared_ptr<Value> value(ObjectValue& node);
    virtual std::shared_ptr<Value> value(StringValue& node);
};

} /* namespace noumenon */

#endif /* VALUE_H_ */
