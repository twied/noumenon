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

#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "Expression.h"
#include "Statement.h"
#include "Value.h"

namespace noumenon {

class Program : public StatementWalker, public ExpressionWalker, public ObjectValue {
public:
    explicit Program(const bool&);
    explicit Program(Program& parent);
    ~Program();

    /* execute a statement */
    std::shared_ptr<Value> statement(AssignmentStatement&);
    std::shared_ptr<Value> statement(CallStatement&);
    std::shared_ptr<Value> statement(ForStatement&);
    std::shared_ptr<Value> statement(IfStatement&);
    std::shared_ptr<Value> statement(ReturnStatement&);
    std::shared_ptr<Value> statement(VarStatement&);
    std::shared_ptr<Value> statement(WhileStatement&);

    /* calculate the value of an expression */
    std::shared_ptr<Value> expression(ArrayExpression&);
    std::shared_ptr<Value> expression(BinaryExpression&);
    std::shared_ptr<Value> expression(BoolExpression&);
    std::shared_ptr<Value> expression(CallExpression&);
    std::shared_ptr<Value> expression(FloatExpression&);
    std::shared_ptr<Value> expression(FunctionExpression&);
    std::shared_ptr<Value> expression(IntExpression&);
    std::shared_ptr<Value> expression(NullExpression&);
    std::shared_ptr<Value> expression(ObjectExpression&);
    std::shared_ptr<Value> expression(StringExpression&);
    std::shared_ptr<Value> expression(UnaryExpression&);
    std::shared_ptr<Value> expression(VariableExpression&);

    /* variables defined in this scope */
    std::shared_ptr<Value> readVariable(VariableExpression&);
    void writeVariable(VariableExpression&, std::shared_ptr<Value>);
    void insertVariable(const std::string&, std::shared_ptr<Value> value);
    Program* getParent();

private:
    bool quiet;
    Program* parent;
};

} /* namespace noumenon */

#endif /* PROGRAM_H_ */
