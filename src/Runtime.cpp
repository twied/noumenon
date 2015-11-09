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

#include "Runtime.h"
#include "Program.h"

#include <fstream>
#include <iostream>

using namespace std;

namespace noumenon {
namespace rtl {

struct PrintWalker : public ValueWalker {
    PrintWalker(Program& scope) : scope(scope) {
    }

    shared_ptr<Value> value(ArrayValue& node) {
        cout << '[';
        for(unsigned long long i = 0; i < node.getLength(); ++i) {
            node.getValue(i)->walk(*this);

            if (i >= node.getLength() - 1) {
                cout << ']';
                return nullptr;
            }

            cout << ", ";
        }

        cout << ']';
        return nullptr;
    }

    shared_ptr<Value> value(BoolValue& node) {
        cout << (node.value ? "true" : "false");
        return nullptr;
    }

    shared_ptr<Value> value(FloatValue& node) {
        cout << node.value;
        return nullptr;
    }

    shared_ptr<Value> value(FunctionValue& node) {
        cout << "function(";
        auto iterator = node.parameters.begin();
        while (iterator != node.parameters.end()) {
            cout << *iterator;

            if (++iterator != node.parameters.end()) {
                cout << ',';
            }
        }
        cout << ')';
        return nullptr;
    }

    shared_ptr<Value> value(IntValue& node) {
        cout << node.value;
        return nullptr;
    }

    shared_ptr<Value> value(NullValue&) {
        cout << "null";
        return nullptr;
    }

    shared_ptr<Value> value(ObjectValue& node) {
        cout << '{';
        auto iterator = node.values.begin();
        while (iterator != node.values.end()) {
            cout << iterator->first << ": ";
            iterator->second->walk(*this);

            if (++iterator != node.values.end()) {
                cout << ", ";
            }
        }
        cout << '}';
        return nullptr;
    }

    shared_ptr<Value> value(StringValue& node) {
        cout << node.value;
        return nullptr;
    }

private:
    Program& scope;
};

struct TypeWalker : public ValueWalker {
    shared_ptr<Value> value(ArrayValue&) {
        return make_shared<StringValue>("Array");
    }

    shared_ptr<Value> value(BoolValue&) {
        return make_shared<StringValue>("Bool");
    }

    shared_ptr<Value> value(FloatValue&) {
        return make_shared<StringValue>("Float");
    }

    shared_ptr<Value> value(FunctionValue&) {
        return make_shared<StringValue>("Function");
    }

    shared_ptr<Value> value(IntValue&) {
        return make_shared<StringValue>("Int");
    }

    shared_ptr<Value> value(NullValue&) {
        return make_shared<StringValue>("Null");
    }

    shared_ptr<Value> value(ObjectValue&) {
        return make_shared<StringValue>("Object");
    }

    shared_ptr<Value> value(StringValue&) {
        return make_shared<StringValue>("String");
    }
};

shared_ptr<Value> Print::doCall(Program& scope, vector<shared_ptr<Value>>& parameters) {
    PrintWalker walker(scope);
    for (auto& parameter : parameters) {
        parameter->walk(walker);
    }

    return NullValue::singleton;
}

shared_ptr<Value> Println::doCall(Program& scope, vector<shared_ptr<Value>>& parameters) {
    PrintWalker walker(scope);
    for (auto& parameter : parameters) {
        parameter->walk(walker);
    }

    cout << endl;
    return NullValue::singleton;
}

shared_ptr<Value> Typeof::doCall(Program&, vector<shared_ptr<Value>>& parameters) {
    if (parameters.size() > 0) {
        TypeWalker walker;
        return parameters[0]->walk(walker);
    }

    return NullValue::singleton;
}

shared_ptr<Value> Range::doCall(Program&, vector<shared_ptr<Value>>& parameters) {
    if (parameters.size() < 2) {
        return NullValue::singleton;
    }

    struct Walker : public DefaultValueWalker {
        Walker() : result(), valid(false) {
        }

        shared_ptr<Value> value(IntValue& node) {
            valid = true;
            result = node.value;
            return nullptr;
        }

        signed long long result;
        bool valid;
    } walkerFrom, walkerTo;

    parameters[0]->walk(walkerFrom);
    parameters[1]->walk(walkerTo);

    if (!walkerFrom.valid || !walkerTo.valid) {
        return NullValue::singleton;
    }

    class RangeValue : public ArrayValue {
    public:
        RangeValue(const long long& from, const long long& to) : from(from), to(to) {
        }

        unsigned long long getLength() {
            return from < to ? to - from : 0;
        }

        std::shared_ptr<Value> getKey(const unsigned long long& index) {
            return make_shared<IntValue>(index);
        }

        std::shared_ptr<Value> getValue(const unsigned long long& index) {
            return make_shared<IntValue>(index + from);
        }

    private:
        const long long from;
        const long long to;
    };

    return make_shared<RangeValue>(walkerFrom.result, walkerTo.result);
}

shared_ptr<Value> Length::doCall(Program&, vector<shared_ptr<Value>>& parameters) {
    if (parameters.size() > 0) {
        return make_shared<IntValue>(parameters[0]->getLength());
    }

    return NullValue::singleton;
}

shared_ptr<Value> List::doCall(Program& program, vector<shared_ptr<Value>>&) {
    PrintWalker walker(program);
    cout << "Variables in current scope:" << endl;
    for (Program* scope = &program; scope; scope = scope->getParent()) {
        for (auto& value : scope->values) {
            cout << "  " << value.first << " = ";
            value.second->walk(walker);
            cout << endl;
        }
    }
    return NullValue::singleton;
}

shared_ptr<Value> Require::doCall(Program& program, vector<shared_ptr<Value>>& parameters) {
    if (parameters.size() < 1) {
        return NullValue::singleton;
    }

    struct Walker : public DefaultValueWalker {
        Walker() : result(), valid(false) {
        }

        shared_ptr<Value> value(StringValue& node) {
            valid = true;
            result = node.value;
            return nullptr;
        }

        string result;
        bool valid;
    } walker;

    parameters[0]->walk(walker);
    if (!walker.valid) {
        return NullValue::singleton;
    }

    ifstream file(walker.result);
    if (!file) {
        return NullValue::singleton;

    }

    auto arguments = make_shared<ArrayValue>();
    if (parameters.size() > 1) {
        arguments->values.assign(parameters.begin() + 1, parameters.end());
    }

    Program nestedProgram(program);
    nestedProgram.insertVariable("arg", arguments);
    return Program::execute(nestedProgram, file);
}

} /* namespace rtl */
} /* namespace noumenon */
