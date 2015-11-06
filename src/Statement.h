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

#ifndef STATEMENT_H_
#define STATEMENT_H_

#include <memory>
#include <vector>

namespace noumenon {

class StatementWalker;
struct Expression;
struct Value;
struct VariableExpression;

struct Statement {
    virtual ~Statement() = 0;
    virtual std::shared_ptr<Value> walk(StatementWalker& processor) = 0;
};

struct AssignmentStatement : public Statement {
    std::shared_ptr<VariableExpression> variable;
    std::shared_ptr<Expression> expression;

    std::shared_ptr<Value> walk(StatementWalker&);
};

struct CallStatement : public Statement {
    std::shared_ptr<VariableExpression> function;
    std::vector<std::shared_ptr<Expression>> expressions;

    std::shared_ptr<Value> walk(StatementWalker&);
};

struct EmptyStatement : public Statement {
    std::shared_ptr<Value> walk(StatementWalker&);
};

struct ForStatement : public Statement {
    std::string key;
    std::string value;
    std::shared_ptr<Expression> expression;
    std::vector<std::shared_ptr<Statement>> statements;

    std::shared_ptr<Value> walk(StatementWalker&);
};

struct IfStatement : public Statement {
    std::shared_ptr<Expression> condition;
    std::vector<std::shared_ptr<Statement>> statementsThen;
    std::vector<std::shared_ptr<Statement>> statementsElse;

    std::shared_ptr<Value> walk(StatementWalker&);
};

struct ReturnStatement : public Statement {
    std::shared_ptr<Expression> expression;

    std::shared_ptr<Value> walk(StatementWalker&);
};

struct VarStatement : public Statement {
    std::string identifier;
    std::shared_ptr<Expression> expression;

    std::shared_ptr<Value> walk(StatementWalker&);
};

struct WhileStatement : public Statement {
    std::shared_ptr<Expression> condition;
    std::vector<std::shared_ptr<Statement>> statements;

    std::shared_ptr<Value> walk(StatementWalker&);
};

class StatementWalker {
public:
    virtual ~StatementWalker() = 0;

    virtual std::shared_ptr<Value> statement(AssignmentStatement&) = 0;
    virtual std::shared_ptr<Value> statement(CallStatement&) = 0;
    virtual std::shared_ptr<Value> statement(ForStatement&) = 0;
    virtual std::shared_ptr<Value> statement(IfStatement&) = 0;
    virtual std::shared_ptr<Value> statement(ReturnStatement&) = 0;
    virtual std::shared_ptr<Value> statement(VarStatement&) = 0;
    virtual std::shared_ptr<Value> statement(WhileStatement&) = 0;
};

} /* namespace noumenon */

#endif /* STATEMENT_H_ */
