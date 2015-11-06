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

#include "Statement.h"
#include "Value.h"

namespace noumenon {

Statement::~Statement() {
}

std::shared_ptr<Value> AssignmentStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

std::shared_ptr<Value> CallStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

std::shared_ptr<Value> EmptyStatement::walk(StatementWalker&) {
    return nullptr;
}

std::shared_ptr<Value> ForStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

std::shared_ptr<Value> IfStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

std::shared_ptr<Value> ReturnStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

std::shared_ptr<Value> VarStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

std::shared_ptr<Value> WhileStatement::walk(StatementWalker& walker) {
    return walker.statement(*this);
}

StatementWalker::~StatementWalker() {
}

} /* namespace noumenon */
