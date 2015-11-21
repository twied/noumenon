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

#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace noumenon {

class ExpressionWalker;

struct Statement;
struct Value;

enum class BinaryOperator {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    AND,
    OR,
    EQU,
    NEQ,
    LES,
    LEQ,
    GRT,
    GEQ
};

enum class UnaryOperator {
    NEG,
    NOT
};

struct Expression {
    virtual ~Expression() = 0;
    virtual std::shared_ptr<Value> walk(ExpressionWalker&) = 0;
};

struct VariableExpression : public Expression {
    std::u32string identifier;
    std::vector<std::shared_ptr<Expression>> expressions;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct ArrayExpression : public Expression {
    std::vector<std::shared_ptr<Expression>> expressions;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct BinaryExpression : public Expression {
    BinaryOperator oper;
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct BoolExpression : public Expression {
    bool value;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct CallExpression : public Expression {
    std::shared_ptr<VariableExpression> function;
    std::vector<std::shared_ptr<Expression>> expressions;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct FloatExpression : public Expression {
    double value;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct FunctionExpression : public Expression {
    std::vector<std::u32string> parameters;
    std::vector<std::shared_ptr<Statement>> statements;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct IntExpression : public Expression {
    signed long long value;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct NullExpression : public Expression {
    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct ObjectExpression : public Expression {
    std::map<std::u32string, std::shared_ptr<Expression>> values;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct StringExpression : public Expression {
    std::u32string value;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

struct UnaryExpression : public Expression {
    UnaryOperator oper;
    std::shared_ptr<Expression> rhs;

    std::shared_ptr<Value> walk(ExpressionWalker&);
};

class ExpressionWalker {
public:
    virtual ~ExpressionWalker() = 0;

    virtual std::shared_ptr<Value> expression(ArrayExpression&) = 0;
    virtual std::shared_ptr<Value> expression(BinaryExpression&) = 0;
    virtual std::shared_ptr<Value> expression(BoolExpression&) = 0;
    virtual std::shared_ptr<Value> expression(CallExpression&) = 0;
    virtual std::shared_ptr<Value> expression(FloatExpression&) = 0;
    virtual std::shared_ptr<Value> expression(FunctionExpression&) = 0;
    virtual std::shared_ptr<Value> expression(IntExpression&) = 0;
    virtual std::shared_ptr<Value> expression(NullExpression&) = 0;
    virtual std::shared_ptr<Value> expression(ObjectExpression&) = 0;
    virtual std::shared_ptr<Value> expression(StringExpression&) = 0;
    virtual std::shared_ptr<Value> expression(UnaryExpression&) = 0;
    virtual std::shared_ptr<Value> expression(VariableExpression&) = 0;
};

} /* namespace noumenon */

#endif /* EXPRESSION_H_ */
