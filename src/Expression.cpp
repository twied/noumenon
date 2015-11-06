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

#include "Expression.h"

namespace noumenon {

Expression::~Expression() {
}

std::shared_ptr<Value> VariableExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> ArrayExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> BinaryExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> BoolExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> CallExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> FloatExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> FunctionExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> IntExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> NullExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> ObjectExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> StringExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

std::shared_ptr<Value> UnaryExpression::walk(ExpressionWalker& walker) {
    return walker.expression(*this);
}

ExpressionWalker::~ExpressionWalker() {
}

} /* namespace noumenon */
