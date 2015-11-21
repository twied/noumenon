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

#include "Value.h"
#include "Expression.h"
#include "Program.h"

#include <codecvt>
#include <locale>

using namespace std;

namespace noumenon {

Value::~Value() {
}

ArrayValue::ArrayValue() : values() {
}

ArrayValue::ArrayValue(const vector<std::shared_ptr<Value>>& values) : values(values.begin(), values.end()) {
}

BoolValue::BoolValue(const bool& value) : value(value) {
}

FloatValue::FloatValue(const double& value) : value(value) {
}

FunctionValue::FunctionValue() : parameters(), statements() {
}

FunctionValue::FunctionValue(const std::vector<std::u32string>& parameters, const std::vector<std::shared_ptr<Statement>>& statements) : parameters(parameters.begin(), parameters.end()), statements(statements.begin(), statements.end()) {
}

IntValue::IntValue(const signed long long& value) : value(value) {
}

std::shared_ptr<NullValue> NullValue::singleton = make_shared<NullValue>();

ObjectValue::ObjectValue() : values() {
}

ObjectValue::ObjectValue(const map<u32string, shared_ptr<Value>>& values) : values(values.begin(), values.end()) {
}

std::string StringValue::UTF32toUTF8(const std::u32string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}

std::u32string StringValue::UTF8toUTF32(const std::string& s) {
    std::wstring_convert<codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}

StringValue::StringValue() : value() {
}

StringValue::StringValue(const u32string& value) : value(value) {
}

std::shared_ptr<Value> ArrayValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> BoolValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> FloatValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> FunctionValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> IntValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> NullValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> ObjectValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

std::shared_ptr<Value> StringValue::walk(ValueWalker& walker) {
    return walker.value(*this);
}

bool Value::isTrue() {
    return false;
}

bool BoolValue::isTrue() {
    return value;
}

shared_ptr<Value> Value::doSelect(shared_ptr<Value>) {
    return NullValue::singleton;
}

shared_ptr<Value> ArrayValue::doSelect(shared_ptr<Value> value) {
    struct Walker : public DefaultValueWalker {
        Walker(ArrayValue& arrayValue) : arrayValue(arrayValue) {
        }

        shared_ptr<Value> value(IntValue& node) {
            if (node.value >= 0 && (unsigned long long) node.value < arrayValue.values.size()) {
                return arrayValue.values[node.value];
            }
            return NullValue::singleton;
        }

        ArrayValue& arrayValue;
    } walker(*this);
    return value->walk(walker);
}

shared_ptr<Value> ObjectValue::doSelect(shared_ptr<Value> value) {
    struct Walker : public DefaultValueWalker {
        Walker(ObjectValue& objectValue) : objectValue(objectValue) {
        }

        shared_ptr<Value> value(StringValue& node) {
            auto iterator = objectValue.values.find(node.value);
            if (iterator != objectValue.values.end()) {
                return iterator->second;
            }
            return NullValue::singleton;
        }

        ObjectValue& objectValue;
    } walker(*this);
    return value->walk(walker);
}

shared_ptr<Value> StringValue::doSelect(shared_ptr<Value> value) {
    struct Walker : public DefaultValueWalker {
        Walker(StringValue& stringValue) : stringValue(stringValue) {
        }

        shared_ptr<Value> value(IntValue& node) {
            if (node.value >= 0 && (unsigned long long) node.value < stringValue.value.size()) {
                return make_shared<StringValue>(u32string(1, stringValue.value[node.value]));
            }
            return NullValue::singleton;
        }

        StringValue& stringValue;
    } walker(*this);
    return value->walk(walker);
}

shared_ptr<Value> Value::doUnary(const UnaryOperator&) {
    return NullValue::singleton;
}

shared_ptr<Value> BoolValue::doUnary(const UnaryOperator& oper) {
    if (oper == UnaryOperator::NOT) {
        return make_shared<BoolValue>(!value);
    }

    return NullValue::singleton;
}

shared_ptr<Value> FloatValue::doUnary(const UnaryOperator& oper) {
    if (oper == UnaryOperator::NEG) {
        return make_shared<FloatValue>(-value);
    }

    return NullValue::singleton;
}

shared_ptr<Value> IntValue::doUnary(const UnaryOperator& oper) {
    if (oper == UnaryOperator::NEG) {
        return make_shared<IntValue>(-value);
    }

    return NullValue::singleton;
}

shared_ptr<Value> Value::doBinary(const BinaryOperator&, shared_ptr<Value>) {
    return NullValue::singleton;
}

shared_ptr<Value> ArrayValue::doBinary(const BinaryOperator& oper, shared_ptr<Value> rhs) {
    /* add element */
    if (oper == BinaryOperator::ADD) {
        auto result = make_shared<ArrayValue>();
        result->values.insert(result->values.begin(), values.begin(), values.end());
        result->values.push_back(rhs);
        return result;
    }

    /* remove element */
    if (oper == BinaryOperator::SUB) {
        auto result = make_shared<ArrayValue>();
        for (auto& value : values) {
            if (!value->doBinary(BinaryOperator::EQU, rhs)->isTrue()) {
                result->values.push_back(value);
            }
        }
        return result;
    }

    return NullValue::singleton;
}

shared_ptr<Value> BoolValue::doBinary(const BinaryOperator& oper, shared_ptr<Value> rhs) {
    struct Walker : public DefaultValueWalker {
        Walker(BoolValue& lhs, const BinaryOperator& oper) : lhs(lhs), oper(oper) {
        }

        shared_ptr<Value> value(BoolValue& rhs) {
            switch (oper) {
            case BinaryOperator::AND:
                return make_shared<BoolValue>(lhs.value && rhs.value);
            case BinaryOperator::OR:
                return make_shared<BoolValue>(lhs.value || rhs.value);
            case BinaryOperator::EQU:
                return make_shared<BoolValue>(lhs.value == rhs.value);
            case BinaryOperator::NEQ:
                return make_shared<BoolValue>(lhs.value != rhs.value);
            default:
                return NullValue::singleton;
            }
        }

        BoolValue& lhs;
        BinaryOperator oper;
    } walker(*this, oper);
    return rhs->walk(walker);
}

shared_ptr<Value> FloatValue::doBinary(const BinaryOperator& oper, shared_ptr<Value> rhs) {
    struct Walker : public DefaultValueWalker {
        Walker(FloatValue& lhs, const BinaryOperator& oper) : lhs(lhs), oper(oper) {
        }

        shared_ptr<Value> value(FloatValue& rhs) {
            switch (oper) {
            case BinaryOperator::ADD:
                return make_shared<FloatValue>(lhs.value + rhs.value);
            case BinaryOperator::SUB:
                return make_shared<FloatValue>(lhs.value - rhs.value);
            case BinaryOperator::MUL:
                return make_shared<FloatValue>(lhs.value * rhs.value);
            case BinaryOperator::DIV:
                if (rhs.value != 0.0) {
                    return make_shared<FloatValue>(lhs.value / rhs.value);
                }
                break;
            case BinaryOperator::EQU:
                return make_shared<BoolValue>(lhs.value == rhs.value);
            case BinaryOperator::NEQ:
                return make_shared<BoolValue>(lhs.value != rhs.value);
            case BinaryOperator::LES:
                return make_shared<BoolValue>(lhs.value < rhs.value);
            case BinaryOperator::LEQ:
                return make_shared<BoolValue>(lhs.value <= rhs.value);
            case BinaryOperator::GRT:
                return make_shared<BoolValue>(lhs.value > rhs.value);
            case BinaryOperator::GEQ:
                return make_shared<BoolValue>(lhs.value >= rhs.value);
            default:
                break;
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(IntValue& rhs) {
            switch (oper) {
            case BinaryOperator::ADD:
                return make_shared<FloatValue>(lhs.value + rhs.value);
            case BinaryOperator::SUB:
                return make_shared<FloatValue>(lhs.value - rhs.value);
            case BinaryOperator::MUL:
                return make_shared<FloatValue>(lhs.value * rhs.value);
            case BinaryOperator::DIV:
                if (rhs.value != 0) {
                    return make_shared<FloatValue>(lhs.value / rhs.value);
                }
                break;
            case BinaryOperator::LES:
                return make_shared<BoolValue>(lhs.value < rhs.value);
            case BinaryOperator::LEQ:
                return make_shared<BoolValue>(lhs.value <= rhs.value);
            case BinaryOperator::GRT:
                return make_shared<BoolValue>(lhs.value > rhs.value);
            case BinaryOperator::GEQ:
                return make_shared<BoolValue>(lhs.value >= rhs.value);
            default:
                break;
            }
            return NullValue::singleton;
        }

        FloatValue& lhs;
        BinaryOperator oper;
    } walker(*this, oper);
    return rhs->walk(walker);
}

shared_ptr<Value> FunctionValue::doBinary(const BinaryOperator&, shared_ptr<Value>) {
    return NullValue::singleton;
}

shared_ptr<Value> IntValue::doBinary(const BinaryOperator& oper, shared_ptr<Value> rhs) {
    struct Walker : public DefaultValueWalker {
        Walker(IntValue& lhs, const BinaryOperator& oper) : lhs(lhs), oper(oper) {
        }

        shared_ptr<Value> value(FloatValue& rhs) {
            switch (oper) {
            case BinaryOperator::ADD:
                return make_shared<FloatValue>(lhs.value + rhs.value);
            case BinaryOperator::SUB:
                return make_shared<FloatValue>(lhs.value - rhs.value);
            case BinaryOperator::MUL:
                return make_shared<FloatValue>(lhs.value * rhs.value);
            case BinaryOperator::DIV:
                if (rhs.value != 0.0) {
                    return make_shared<FloatValue>(lhs.value / rhs.value);
                }
                break;
            case BinaryOperator::LES:
                return make_shared<BoolValue>(lhs.value < rhs.value);
            case BinaryOperator::LEQ:
                return make_shared<BoolValue>(lhs.value <= rhs.value);
            case BinaryOperator::GRT:
                return make_shared<BoolValue>(lhs.value > rhs.value);
            case BinaryOperator::GEQ:
                return make_shared<BoolValue>(lhs.value >= rhs.value);
            default:
                break;
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(IntValue& rhs) {
            switch (oper) {
            case BinaryOperator::ADD:
                return make_shared<IntValue>(lhs.value + rhs.value);
            case BinaryOperator::SUB:
                return make_shared<IntValue>(lhs.value - rhs.value);
            case BinaryOperator::MUL:
                return make_shared<IntValue>(lhs.value * rhs.value);
            case BinaryOperator::DIV:
                if (rhs.value != 0) {
                    return make_shared<IntValue>(lhs.value / rhs.value);
                }
                break;
            case BinaryOperator::MOD:
                if (rhs.value != 0) {
                    return make_shared<IntValue>(lhs.value % rhs.value);
                }
                break;
            case BinaryOperator::EQU:
                return make_shared<BoolValue>(lhs.value == rhs.value);
            case BinaryOperator::NEQ:
                return make_shared<BoolValue>(lhs.value != rhs.value);
            case BinaryOperator::LES:
                return make_shared<BoolValue>(lhs.value < rhs.value);
            case BinaryOperator::LEQ:
                return make_shared<BoolValue>(lhs.value <= rhs.value);
            case BinaryOperator::GRT:
                return make_shared<BoolValue>(lhs.value > rhs.value);
            case BinaryOperator::GEQ:
                return make_shared<BoolValue>(lhs.value >= rhs.value);
            default:
                break;
            }
            return NullValue::singleton;
        }

        IntValue& lhs;
        BinaryOperator oper;
    } walker(*this, oper);
    return rhs->walk(walker);
}

shared_ptr<Value> NullValue::doBinary(const BinaryOperator&, shared_ptr<Value>) {
    return NullValue::singleton;
}

shared_ptr<Value> ObjectValue::doBinary(const BinaryOperator& oper, shared_ptr<Value> rhs) {
    struct Walker : public DefaultValueWalker {
        Walker(ObjectValue& lhs, const BinaryOperator& oper) : lhs(lhs), oper(oper) {
        }

        shared_ptr<Value> value(StringValue& rhs) {
            if (oper == BinaryOperator::SUB) {
                auto returnValue = make_shared<ObjectValue>(lhs.values);
                returnValue->values.erase(rhs.value);
                return returnValue;
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(ObjectValue& rhs) {
            auto returnValue = make_shared<ObjectValue>(lhs.values);

            switch (oper) {
            case BinaryOperator::AND:
                for (const auto& key : rhs.values) {
                    if (returnValue->values.find(key.first) == returnValue->values.end()) {
                        returnValue->values.erase(key.first);
                    }
                }
                return returnValue;

            case BinaryOperator::OR:
                for (const auto& key : rhs.values) {
                    if (returnValue->values.find(key.first) == returnValue->values.end()) {
                        returnValue->values[key.first] = key.second;
                    }
                }
                return returnValue;

            case BinaryOperator::EQU:
                if (lhs.values.size() != rhs.values.size()) {
                    return make_shared<BoolValue>(false);
                }

                for (const auto& key : rhs.values) {
                    const auto& value = lhs.values.find(key.first);

                    if (value == lhs.values.end()) {
                        return make_shared<BoolValue>(false);
                    }

                    if (!value->second->doBinary(BinaryOperator::EQU, key.second)->isTrue()) {
                        return make_shared<BoolValue>(false);
                    }
                }

                return make_shared<BoolValue>(true);

            case BinaryOperator::NEQ:
                if (lhs.values.size() != rhs.values.size()) {
                    return make_shared<BoolValue>(true);
                }

                for (const auto& key : rhs.values) {
                    const auto& value = lhs.values.find(key.first);

                    if (value == lhs.values.end()) {
                        return make_shared<BoolValue>(true);
                    }

                    if (!value->second->doBinary(BinaryOperator::NEQ, key.second)->isTrue()) {
                        return make_shared<BoolValue>(true);
                    }
                }

                return make_shared<BoolValue>(false);

            default:
                break;
            }
            return NullValue::singleton;
        }

        ObjectValue& lhs;
        BinaryOperator oper;
    } walker(*this, oper);
    return rhs->walk(walker);
}

shared_ptr<Value> StringValue::doBinary(const BinaryOperator& oper, shared_ptr<Value> rhs) {
    struct Walker : public ValueWalker {
        Walker(StringValue& lhs, const BinaryOperator& oper) : lhs(lhs), oper(oper) {
        }

        shared_ptr<Value> value(ArrayValue&) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + U"Array");
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(BoolValue& rhs) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + (rhs.value ? U"true" : U"false"));
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(FloatValue& rhs) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + StringValue::UTF8toUTF32(to_string(rhs.value)));
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(FunctionValue&) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + U"Function");
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(IntValue& rhs) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + StringValue::UTF8toUTF32(to_string(rhs.value)));
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(NullValue&) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + U"null");
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(ObjectValue&) {
            if (oper == BinaryOperator::ADD) {
                return make_shared<StringValue>(lhs.value + U"Object");
            }
            return NullValue::singleton;
        }

        shared_ptr<Value> value(StringValue& rhs) {
            switch (oper) {
            case BinaryOperator::ADD:
                return make_shared<StringValue>(lhs.value + rhs.value);
            case BinaryOperator::EQU:
                return make_shared<BoolValue>(lhs.value == rhs.value);
            case BinaryOperator::NEQ:
                return make_shared<BoolValue>(lhs.value != rhs.value);
            default:
                break;
            }
            return NullValue::singleton;
        }

        StringValue& lhs;
        BinaryOperator oper;
    } walker(*this, oper);
    return rhs->walk(walker);
}

shared_ptr<Value> Value::doCall(Program&, vector<shared_ptr<Value>>&) {
    return NullValue::singleton;
}

shared_ptr<Value> FunctionValue::doCall(Program& scope, vector<shared_ptr<Value>>& values) {
    for (decltype(parameters.size()) i = 0; i < parameters.size(); ++i) {
        scope.insertVariable(parameters[i], i < values.size() ? values[i] : NullValue::singleton);
    }

    for (auto& statement : statements) {
        const auto& returnValue = statement->walk(scope);
        if (returnValue != nullptr) {
            return returnValue;
        }
    }

    return NullValue::singleton;
}

void Value::doModify(shared_ptr<Value>, shared_ptr<Value>) {
}

void ArrayValue::doModify(shared_ptr<Value> index, shared_ptr<Value> value) {
    struct Walker : public DefaultValueWalker {
        Walker(ArrayValue& array, shared_ptr<Value> newValue) : array(array), newValue(newValue) {
        }

        shared_ptr<Value> value(IntValue& index) {
            if (index.value >= 0 && (unsigned long long) index.value < array.values.size()) {
                array.values[index.value] = newValue;
            }
            return NullValue::singleton;
        }

        ArrayValue& array;
        shared_ptr<Value> newValue;
    } walker(*this, value);
    index->walk(walker);
}

void ObjectValue::doModify(shared_ptr<Value> index, shared_ptr<Value> value) {
    struct Walker : public DefaultValueWalker {
        Walker(ObjectValue& object, shared_ptr<Value> newValue) : object(object), newValue(newValue) {
        }

        shared_ptr<Value> value(StringValue& index) {
            object.values[index.value] = newValue;
            return NullValue::singleton;
        }

        ObjectValue& object;
        shared_ptr<Value> newValue;
    } walker(*this, value);
    index->walk(walker);
}

unsigned long long Value::getLength() {
    return 0;
}

unsigned long long ArrayValue::getLength() {
    return values.size();
}

unsigned long long ObjectValue::getLength() {
    return values.size();
}

unsigned long long StringValue::getLength() {
    return value.size();
}

shared_ptr<Value> Value::getKey(const unsigned long long&) {
    return NullValue::singleton;
}

shared_ptr<Value> ArrayValue::getKey(const unsigned long long& index) {
    return make_shared<IntValue>(index);
}

shared_ptr<Value> ObjectValue::getKey(const unsigned long long& index) {
    auto iterator = values.begin();
    std::advance(iterator, index);

    if (iterator != values.end()) {
        return make_shared<StringValue>(iterator->first);
    }
    return NullValue::singleton;
}

shared_ptr<Value> StringValue::getKey(const unsigned long long& index) {
    return make_shared<IntValue>(index);
}

shared_ptr<Value> Value::getValue(const unsigned long long&) {
    return NullValue::singleton;
}

shared_ptr<Value> ArrayValue::getValue(const unsigned long long& index) {
    if (index < values.size()) {
        return values[index];
    }
    return NullValue::singleton;
}

shared_ptr<Value> ObjectValue::getValue(const unsigned long long& index) {
    auto iterator = values.begin();
    std::advance(iterator, index);

    if (iterator != values.end()) {
        return iterator->second;
    }
    return NullValue::singleton;
}

shared_ptr<Value> StringValue::getValue(const unsigned long long& index) {
    if (index < value.size()) {
        return make_shared<StringValue>(u32string(1, value[index]));
    }

    return NullValue::singleton;
}

ValueWalker::~ValueWalker() {
}

DefaultValueWalker::~DefaultValueWalker() {
}

std::shared_ptr<Value> DefaultValueWalker::value(ArrayValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(BoolValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(FloatValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(FunctionValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(IntValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(NullValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(ObjectValue&) {
    return NullValue::singleton;
}

std::shared_ptr<Value> DefaultValueWalker::value(StringValue&) {
    return NullValue::singleton;
}

} /* namespace noumenon */
