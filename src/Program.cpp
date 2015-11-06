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

#include "Program.h"
#include "Value.h"

#include <iostream>
#include <fstream>

using namespace std;

namespace noumenon {

Program::Program(const bool& quiet) : quiet(quiet), parent(nullptr) {
}

Program::Program(Program& parent) : ObjectValue(), quiet(parent.quiet), parent(&parent) {
}

Program::~Program() {
}

std::shared_ptr<Value> Program::statement(AssignmentStatement& node) {
    writeVariable(*node.variable, node.expression->walk(*this));
    return nullptr;
}

std::shared_ptr<Value> Program::statement(CallStatement& node) {
    vector<shared_ptr<Value>> parameters;
    for (auto& expression : node.expressions) {
        parameters.push_back(expression->walk(*this));
    }

    Program subscope(*this);
    node.function->walk(*this)->doCall(subscope, parameters);
    return nullptr;
}

std::shared_ptr<Value> Program::statement(ForStatement& node) {
    auto value = node.expression->walk(*this);

    for (unsigned long long i = 0; i < value->getLength(); ++i) {
        Program subscope(*this);
        if (!node.key.empty()) {
            subscope.insertVariable(node.key, value->getKey(i));
        }
        subscope.insertVariable(node.value, value->getValue(i));

        for (auto& statement : node.statements) {
            const auto& returnValue = statement->walk(subscope);
            if (returnValue != nullptr) {
                return returnValue;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Value> Program::statement(IfStatement& node) {
    auto condition = node.condition->walk(*this);
    Program body(*this);
    if (condition->isTrue()) {
        for (auto& statement : node.statementsThen) {
            const auto& returnValue = statement->walk(body);
            if (returnValue != nullptr) {
                return returnValue;
            }
        }
    } else {
        for (auto& statement : node.statementsElse) {
            const auto& returnValue = statement->walk(body);
            if (returnValue != nullptr) {
                return returnValue;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Value> Program::statement(ReturnStatement& node) {
    return node.expression->walk(*this);
}

std::shared_ptr<Value> Program::statement(VarStatement& node) {
    insertVariable(node.identifier, node.expression->walk(*this));
    return nullptr;
}

std::shared_ptr<Value> Program::statement(WhileStatement& node) {
    while (node.condition->walk(*this)->isTrue()) {
        Program body(*this);
        for (auto& statement : node.statements) {
            const auto& returnValue = statement->walk(body);
            if (returnValue) {
                return returnValue;
            }
        }
    }
    return nullptr;
}

shared_ptr<Value> Program::expression(ArrayExpression& node) {
    vector<shared_ptr<Value>> values;
    for (auto& expression : node.expressions) {
        values.push_back(expression->walk(*this));
    }
    return make_shared<ArrayValue>(values);
}

shared_ptr<Value> Program::expression(BinaryExpression& node) {
    return node.lhs->walk(*this)->doBinary(node.oper, node.rhs->walk(*this));
}

shared_ptr<Value> Program::expression(BoolExpression& node) {
    return make_shared<BoolValue>(node.value);
}

shared_ptr<Value> Program::expression(CallExpression& node) {
    vector<shared_ptr<Value>> expressions;
    for (auto& expression : node.expressions) {
        expressions.push_back(expression->walk(*this));
    }

    Program subscope(*this);
    return node.function->walk(subscope)->doCall(subscope, expressions);
}

shared_ptr<Value> Program::expression(FloatExpression& node) {
    return make_shared<FloatValue>(node.value);
}

shared_ptr<Value> Program::expression(FunctionExpression& node) {
    return make_shared<FunctionValue>(node.parameters, node.statements);
}

shared_ptr<Value> Program::expression(IntExpression& node) {
    return make_shared<IntValue>(node.value);
}

shared_ptr<Value> Program::expression(NullExpression&) {
    return NullValue::singleton;
}

shared_ptr<Value> Program::expression(ObjectExpression& node) {
    map<string, shared_ptr<Value>> values;
    for (auto& pair : node.values) {
        values[pair.first] = pair.second->walk(*this);
    }
    return make_shared<ObjectValue>(values);
}

shared_ptr<Value> Program::expression(StringExpression& node) {
    return make_shared<StringValue>(node.value);
}

shared_ptr<Value> Program::expression(UnaryExpression& node) {
    return node.rhs->walk(*this)->doUnary(node.oper);
}

shared_ptr<Value> Program::expression(VariableExpression& node) {
    return readVariable(node);
}

shared_ptr<Value> Program::readVariable(VariableExpression& variable) {
    for (Program* scope = this; scope != nullptr; scope = scope->parent) {
        auto iterator = scope->values.find(variable.identifier);

        if (iterator == scope->values.end()) {
            continue;
        }

        auto result = iterator->second;
        for (auto& selector : variable.expressions) {
            result = result->doSelect(selector->walk(*this));
        }

        return result;
    }

    if (!quiet) {
        cerr << "no such variable: \"" + variable.identifier + "\"" << endl;
    }
    return NullValue::singleton;
}

void Program::writeVariable(VariableExpression& variable, shared_ptr<Value> value) {
    for (Program* scope = this; scope != nullptr; scope = scope->parent) {
        auto iterator = scope->values.find(variable.identifier);

        if (iterator == scope->values.end()) {
            continue;
        }

        if (variable.expressions.empty()) {
            iterator->second = value;
            return;
        }

        auto result = iterator->second;
        for (auto& expression : vector<shared_ptr<Expression>>(variable.expressions.begin(), variable.expressions.end() - 1)) {
            result = result->doSelect(expression->walk(*this));
        }

        result->doModify(variable.expressions.back()->walk(*this), value);
        return;
    }

    if (!quiet) {
        cerr << "no such variable: \"" + variable.identifier + "\"" << endl;
    }
}

void Program::insertVariable(const string& identifier, shared_ptr<Value> value) {
    if (values.find(identifier) != values.end()) {
        if (!quiet) {
            cerr << "redefinition of variable: \"" + identifier + "\"" << endl;
        }
        return;
    }

    values[identifier] = value;
}

Program* Program::getParent() {
    return parent;
}

} /* namespace scriptlanguage */
