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

static bool is_whitespace(const char32_t& c) {
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
        return true;
    default:
        return false;
    }
}

static bool is_alpha(const char32_t& c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static bool is_numeral(const char32_t& c) {
    return c >= '0' && c <= '9';
}

static int value_hex(const char32_t& c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }

    if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }

    return -1;
}

enum Token {
    TOKEN_UNKNOWN = -1,
    TOKEN_EOF = 0,

    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,

    BRACKET_CURLY_LEFT,
    BRACKET_CURLY_RIGHT,
    BRACKET_ROUND_LEFT,
    BRACKET_ROUND_RIGHT,
    BRACKET_SQUARE_LEFT,
    BRACKET_SQUARE_RIGHT,

    KEYWORD_ELSE,
    KEYWORD_FALSE,
    KEYWORD_FOR,
    KEYWORD_FUNCTION,
    KEYWORD_IF,
    KEYWORD_NULL,
    KEYWORD_RETURN,
    KEYWORD_TRUE,
    KEYWORD_VAR,
    KEYWORD_WHILE,

    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,

    TOKEN_EQUAL,
    TOKEN_NOTEQUAL,
    TOKEN_GREATERTHAN,
    TOKEN_GREATEROREQUAL,
    TOKEN_LESSTHAN,
    TOKEN_LESSOREQUAL,

    TOKEN_ASSIGNMENT,

    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT
};

static const char* token_name(const Token& token) {
    switch (token) {
    case TOKEN_EOF:
        return "EOF";

    case TOKEN_IDENTIFIER:
        return "identifier";
    case TOKEN_INTEGER:
        return "integer";
    case TOKEN_FLOAT:
        return "float";
    case TOKEN_STRING:
        return "string";

    case BRACKET_CURLY_LEFT:
        return "{";
    case BRACKET_CURLY_RIGHT:
        return "}";
    case BRACKET_ROUND_LEFT:
        return "(";
    case BRACKET_ROUND_RIGHT:
        return ")";
    case BRACKET_SQUARE_LEFT:
        return "[";
    case BRACKET_SQUARE_RIGHT:
        return "]";

    case KEYWORD_ELSE:
        return "else";
    case KEYWORD_FALSE:
        return "false";
    case KEYWORD_FOR:
        return "for";
    case KEYWORD_FUNCTION:
        return "function";
    case KEYWORD_IF:
        return "if";
    case KEYWORD_NULL:
        return "null";
    case KEYWORD_RETURN:
        return "return";
    case KEYWORD_TRUE:
        return "true";
    case KEYWORD_VAR:
        return "var";
    case KEYWORD_WHILE:
        return "while";

    case TOKEN_SEMICOLON:
        return ";";
    case TOKEN_COLON:
        return ":";
    case TOKEN_COMMA:
        return ",";

    case TOKEN_PLUS:
        return "+";
    case TOKEN_MINUS:
        return "-";
    case TOKEN_MULTIPLY:
        return "*";
    case TOKEN_DIVIDE:
        return "/";
    case TOKEN_MODULO:
        return "%";

    case TOKEN_EQUAL:
        return "==";
    case TOKEN_NOTEQUAL:
        return "!=";
    case TOKEN_GREATERTHAN:
        return ">";
    case TOKEN_GREATEROREQUAL:
        return ">=";
    case TOKEN_LESSTHAN:
        return "<";
    case TOKEN_LESSOREQUAL:
        return "<=";

    case TOKEN_ASSIGNMENT:
        return "=";
    case TOKEN_AND:
        return "&&";
    case TOKEN_OR:
        return "||";
    case TOKEN_NOT:
        return "!";
    default:
        break;
    }

    return "UNKNOWN TOKEN";
}

class Lexer {
public:
    Lexer(istream& stream) : currentRow(1), currentCol(0), currentIdentifier(), currentChar(' '), stream(stream) {
    }

    Token operator()() {
        /* remove whitespace */
        while (is_whitespace(currentChar)) {
            getChar();
        }

        /* end of file */
        if (stream.eof()) {
            return TOKEN_EOF;
        }

        if (is_alpha(currentChar) || currentChar == '_') {
            currentIdentifier.clear();
            while (is_alpha(currentChar) || is_numeral(currentChar) || currentChar == '_') {
                currentIdentifier += currentChar;
                getChar();
            }

            if (currentIdentifier == U"else") {
                return KEYWORD_ELSE;
            } else if (currentIdentifier == U"false") {
                return KEYWORD_FALSE;
            } else if (currentIdentifier == U"for") {
                return KEYWORD_FOR;
            } else if (currentIdentifier == U"function") {
                return KEYWORD_FUNCTION;
            } else if (currentIdentifier == U"if") {
                return KEYWORD_IF;
            } else if (currentIdentifier == U"null") {
                return KEYWORD_NULL;
            } else if (currentIdentifier == U"return") {
                return KEYWORD_RETURN;
            } else if (currentIdentifier == U"true") {
                return KEYWORD_TRUE;
            } else if (currentIdentifier == U"var") {
                return KEYWORD_VAR;
            } else if (currentIdentifier == U"while") {
                return KEYWORD_WHILE;
            }

            return TOKEN_IDENTIFIER;
        }

        if (currentChar == '\"') {
            return parseString();
        }

        if (is_numeral(currentChar) || currentChar == '.') {
            currentIdentifier.clear();
            while (is_numeral(currentChar)) {
                currentIdentifier += currentChar;
                getChar();
            }

            switch (currentChar) {
            case '.':
            case 'e':
            case 'E':
                return parseFloat();
            default:
                return TOKEN_INTEGER;
            }
        }

        const auto lastChar = currentChar;
        getChar();
        switch (lastChar) {
        case '{':
            return BRACKET_CURLY_LEFT;
        case '}':
            return BRACKET_CURLY_RIGHT;
        case '(':
            return BRACKET_ROUND_LEFT;
        case ')':
            return BRACKET_ROUND_RIGHT;
        case '[':
            return BRACKET_SQUARE_LEFT;
        case ']':
            return BRACKET_SQUARE_RIGHT;

        case ';':
            return TOKEN_SEMICOLON;
        case ':':
            return TOKEN_COLON;
        case ',':
            return TOKEN_COMMA;

        case '+':
            return TOKEN_PLUS;
        case '-':
            return TOKEN_MINUS;
        case '*':
            return TOKEN_MULTIPLY;
        case '%':
            return TOKEN_MODULO;
        case '/': {
            if (currentChar == '/') {
                /* single line comment */
                while (!stream.eof() && currentChar != '\n' && currentChar != '\r') {
                    getChar();
                }

                return operator()();
            }

            if (currentChar == '*') {
                /* multi line comment */
                while (!stream.eof()) {
                    getChar();

                    if (currentChar != '*') {
                        continue;
                    }

                    while (currentChar == '*') {
                        getChar();
                    }

                    if (currentChar == '/') {
                        getChar();
                        return operator()();
                    }
                }
                return TOKEN_EOF;
            }

            return TOKEN_DIVIDE;
        }

        case '&':
            if (currentChar != '&') {
                return TOKEN_UNKNOWN;
            }
            getChar();
            return TOKEN_AND;
        case '|':
            if (currentChar != '|') {
                return TOKEN_UNKNOWN;
            }
            getChar();
            return TOKEN_OR;
        case '>':
            if (currentChar == '=') {
                getChar();
                return TOKEN_GREATEROREQUAL;
            } else {
                return TOKEN_GREATERTHAN;
            }
        case '<':
            if (currentChar == '=') {
                getChar();
                return TOKEN_LESSOREQUAL;
            } else {
                return TOKEN_LESSTHAN;
            }
        case '=':
            if (currentChar == '=') {
                getChar();
                return TOKEN_EQUAL;
            } else {
                return TOKEN_ASSIGNMENT;
            }
        case '!':
            if (currentChar == '=') {
                getChar();
                return TOKEN_NOTEQUAL;
            } else {
                return TOKEN_NOT;
            }
        default:
            break;
        }

        return TOKEN_UNKNOWN;
    }

    unsigned currentRow;
    unsigned currentCol;
    u32string currentIdentifier;

private:
    void getChar() {
        currentCol += 1;

        const auto next_char = stream.get();

        if (next_char == '\n' || next_char == '\r') {
            /* newline */
            currentRow += 1;
            currentCol = 0;
        }

        if(next_char < 0x80) {
            /* ascii character */
            currentChar = next_char;
            return;
        }

        /* unicode character */
        string mbs;
        mbs += next_char;


        auto length = 4;
        if (next_char < 0xf0) {
            length = 3;
        }
        if (next_char < 0xe0) {
            length = 2;
        }

        while(--length > 0) {
            mbs += stream.get();
        }

        currentChar = StringValue::UTF8toUTF32(mbs)[0];
    }

    Token parseFloat() {
        if (currentChar == '.') {
            currentIdentifier += currentChar;
            getChar();

            if (currentIdentifier == U".") {
                /* must contain at least one digit */
                if (!is_numeral(currentChar)) {
                    return TOKEN_UNKNOWN;
                }
            }

            while (is_numeral(currentChar)) {
                currentIdentifier += currentChar;
                getChar();
            }

            if (currentChar == 'e' || currentChar == 'E') {
                return parseFloat();
            }
        } else {
            /* must be 'e' or 'E' */
            currentIdentifier += currentChar;
            getChar();

            /* optional '+' or '-' */
            if (currentChar == '+') {
                currentIdentifier += currentChar;
                getChar();
            } else if (currentChar == '-') {
                currentIdentifier += currentChar;
                getChar();
            }

            /* exponent must contain at least one digit */
            if (!is_numeral(currentChar)) {
                return TOKEN_UNKNOWN;
            }

            while (is_numeral(currentChar)) {
                currentIdentifier += currentChar;
                getChar();
            }
        }

        return TOKEN_FLOAT;
    }

    Token parseString() {
        /* dispose leading quotation mark */
        getChar();
        currentIdentifier.clear();

        /* while we haven't reached the end of the string... */
        while (currentChar != '\"') {

            /* premature EOF? */
            if (stream.eof()) {
                return TOKEN_UNKNOWN;
            }

            if (currentChar == '\\') {
                /* escaped character, dispose the backslash */
                getChar();

                switch (currentChar) {
                case 'u': {
                    int digits[4];
                    for (unsigned i = 0; i < 4; ++i) {
                        getChar();
                        digits[i] = value_hex(currentChar);
                        if (digits[i] < 0) {
                            return TOKEN_UNKNOWN;
                        }
                    }
                    int value = 0;
                    for (unsigned i = 0; i < 4; ++i) {
                        value = (value * 16) + digits[i];
                    }
                    currentIdentifier += value;
                    break;
                }
                case 'b':
                    currentIdentifier += '\b';
                    break;
                case 't':
                    currentIdentifier += '\t';
                    break;
                case 'n':
                    currentIdentifier += '\n';
                    break;
                case 'f':
                    currentIdentifier += '\f';
                    break;
                case 'r':
                    currentIdentifier += '\r';
                    break;
                case '\"':
                    currentIdentifier += '\"';
                    break;
                case '\'':
                    currentIdentifier += '\'';
                    break;
                case '\\':
                    currentIdentifier += '\\';
                    break;
                default:
                    return TOKEN_UNKNOWN;
                }
            } else {
                /* regular character, just insert */
                currentIdentifier += currentChar;
            }
            getChar();
        }

        /* dispose trailing quotation mark */
        getChar();
        return TOKEN_STRING;
    }

    char32_t currentChar;
    istream& stream;
};

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer), currentToken(TOKEN_UNKNOWN), init(false) {
    }

    shared_ptr<Statement> operator()() {
        if (!init) {
            init = true;
            eat(currentToken);
        }

        return parseStatement();
    }

private:
    Lexer& lexer;
    Token currentToken;
    bool init;

    void eat(const Token& token) {
        if (currentToken == token) {
            currentToken = lexer();
            return;
        }

        throwException(string("unexpected token \"") + token_name(currentToken) + "\" instead of \"" + token_name(token) + "\"");
    }

    void throwException(const string& message) {
        throw to_string(lexer.currentRow) + ":" + to_string(lexer.currentCol) + ": " + message;
    }

    u32string parseIdentifier() {
        u32string identifier = lexer.currentIdentifier;
        eat(TOKEN_IDENTIFIER);
        return identifier;
    }

    shared_ptr<Expression> parseExpression() {
        auto binary = make_shared<BinaryExpression>();
        binary->lhs = parseOperandExpression();

        switch (currentToken) {
        case TOKEN_EQUAL:
            eat(TOKEN_EQUAL);
            binary->oper = BinaryOperator::EQU;
            break;

        case TOKEN_NOTEQUAL:
            eat(TOKEN_NOTEQUAL);
            binary->oper = BinaryOperator::NEQ;
            break;

        case TOKEN_LESSTHAN:
            eat(TOKEN_LESSTHAN);
            binary->oper = BinaryOperator::LES;
            break;

        case TOKEN_LESSOREQUAL:
            eat(TOKEN_LESSOREQUAL);
            binary->oper = BinaryOperator::LEQ;
            break;

        case TOKEN_GREATERTHAN:
            eat(TOKEN_GREATERTHAN);
            binary->oper = BinaryOperator::GRT;
            break;

        case TOKEN_GREATEROREQUAL:
            eat(TOKEN_GREATEROREQUAL);
            binary->oper = BinaryOperator::GEQ;
            break;

        default:
            return binary->lhs;
        }

        binary->rhs = parseOperandExpression();
        return binary;
    }

    shared_ptr<Expression> parseOperandExpression() {
        auto binary = make_shared<BinaryExpression>();
        binary->lhs = parseTermExpression();

        switch (currentToken) {
        case TOKEN_PLUS:
            eat(TOKEN_PLUS);
            binary->oper = BinaryOperator::ADD;
            break;

        case TOKEN_MINUS:
            eat(TOKEN_MINUS);
            binary->oper = BinaryOperator::SUB;
            break;

        case TOKEN_OR:
            eat(TOKEN_OR);
            binary->oper = BinaryOperator::OR;
            break;

        default:
            return binary->lhs;
        }

        binary->rhs = parseTermExpression();
        return binary;
    }

    shared_ptr<Expression> parseTermExpression() {
        auto binary = make_shared<BinaryExpression>();
        binary->lhs = parseUnaryExpression();

        switch (currentToken) {
        case TOKEN_MULTIPLY:
            eat(TOKEN_MULTIPLY);
            binary->oper = BinaryOperator::MUL;
            break;

        case TOKEN_DIVIDE:
            eat(TOKEN_DIVIDE);
            binary->oper = BinaryOperator::DIV;
            break;

        case TOKEN_MODULO:
            eat(TOKEN_MODULO);
            binary->oper = BinaryOperator::MOD;
            break;

        case TOKEN_AND:
            eat(TOKEN_AND);
            binary->oper = BinaryOperator::AND;
            break;

        default:
            return binary->lhs;
        }

        binary->rhs = parseUnaryExpression();
        return binary;
    }

    shared_ptr<Expression> parseUnaryExpression() {
        auto unary = make_shared<UnaryExpression>();

        switch (currentToken) {
        case TOKEN_MINUS:
            eat(TOKEN_MINUS);
            unary->oper = UnaryOperator::NEG;
            break;

        case TOKEN_NOT:
            eat(TOKEN_NOT);
            unary->oper = UnaryOperator::NOT;
            break;

        default:
            return parseFactorExpression();
        }

        unary->rhs = parseFactorExpression();
        return unary;
    }

    shared_ptr<Expression> parseFactorExpression() {
        switch (currentToken) {
        case TOKEN_INTEGER: {
            auto value = make_shared<IntExpression>();
            try {
                value->value = stoll(StringValue::UTF32toUTF8(lexer.currentIdentifier));
            } catch (const std::logic_error& e) {
                throwException(string(e.what()));
            }
            eat(TOKEN_INTEGER);
            return value;
        }
        case TOKEN_FLOAT: {
            auto value = make_shared<FloatExpression>();
            try {
                value->value = stod(StringValue::UTF32toUTF8(lexer.currentIdentifier));
            } catch (const std::logic_error& e) {
                throwException(string(e.what()));
            }
            eat(TOKEN_FLOAT);
            return value;
        }
        case TOKEN_STRING: {
            auto value = make_shared<StringExpression>();
            value->value = lexer.currentIdentifier;
            eat(TOKEN_STRING);
            return value;
        }
        case KEYWORD_TRUE: {
            auto value = make_shared<BoolExpression>();
            value->value = true;
            eat(KEYWORD_TRUE);
            return value;
        }
        case KEYWORD_FALSE: {
            auto value = make_shared<BoolExpression>();
            value->value = false;
            eat(KEYWORD_FALSE);
            return value;
        }
        case KEYWORD_NULL: {
            auto value = make_shared<NullExpression>();
            eat(KEYWORD_NULL);
            return value;
        }
        case BRACKET_SQUARE_LEFT: {
            auto value = make_shared<ArrayExpression>();
            eat(BRACKET_SQUARE_LEFT);
            if (currentToken != BRACKET_SQUARE_RIGHT) {
                value->expressions.push_back(parseExpression());
                while (currentToken == TOKEN_COMMA) {
                    eat(TOKEN_COMMA);
                    value->expressions.push_back(parseExpression());
                }
            }
            eat(BRACKET_SQUARE_RIGHT);
            return value;
        }
        case BRACKET_CURLY_LEFT: {
            auto value = make_shared<ObjectExpression>();
            eat(BRACKET_CURLY_LEFT);
            if (currentToken != BRACKET_CURLY_RIGHT) {
                std::u32string key = parseIdentifier();
                eat(TOKEN_COLON);
                auto expr = parseExpression();
                value->values[key] = expr;

                while (currentToken == TOKEN_COMMA) {
                    eat(TOKEN_COMMA);
                    key = parseIdentifier();
                    eat(TOKEN_COLON);
                    expr = parseExpression();
                    value->values[key] = expr;
                }
            }
            eat(BRACKET_CURLY_RIGHT);
            return value;
        }
        case KEYWORD_FUNCTION: {
            auto value = make_shared<FunctionExpression>();
            eat(KEYWORD_FUNCTION);
            eat(BRACKET_ROUND_LEFT);
            if (currentToken != BRACKET_ROUND_RIGHT) {
                value->parameters.push_back(parseIdentifier());
                while (currentToken == TOKEN_COMMA) {
                    eat(TOKEN_COMMA);
                    value->parameters.push_back(parseIdentifier());
                }
            }
            eat(BRACKET_ROUND_RIGHT);

            eat(BRACKET_CURLY_LEFT);
            while (currentToken != BRACKET_CURLY_RIGHT) {
                value->statements.push_back(parseStatement());
            }
            eat(BRACKET_CURLY_RIGHT);

            return value;
        }
        case BRACKET_ROUND_LEFT: {
            eat(BRACKET_ROUND_LEFT);
            auto expression = parseExpression();
            eat(BRACKET_ROUND_RIGHT);
            return expression;
        }
        default:
            break;
        }

        auto variable = parseVariableExpression();
        if (currentToken == BRACKET_ROUND_LEFT) {
            auto value = make_shared<CallExpression>();
            value->function = variable;
            eat(BRACKET_ROUND_LEFT);
            if (currentToken != BRACKET_ROUND_RIGHT) {
                value->expressions.push_back(parseExpression());
                while (currentToken == TOKEN_COMMA) {
                    eat(TOKEN_COMMA);
                    value->expressions.push_back(parseExpression());
                }
            }
            eat(BRACKET_ROUND_RIGHT);
            return value;
        } else {
            return variable;
        }
    }

    shared_ptr<VariableExpression> parseVariableExpression() {
        auto node = make_shared<VariableExpression>();
        node->identifier = parseIdentifier();
        while (currentToken == BRACKET_SQUARE_LEFT) {
            eat(BRACKET_SQUARE_LEFT);
            node->expressions.push_back(parseExpression());
            eat(BRACKET_SQUARE_RIGHT);
        }

        return node;
    }

    shared_ptr<Statement> parseStatement() {
        switch (currentToken) {
        case TOKEN_EOF:
            return nullptr;
        case TOKEN_SEMICOLON:
            return parseEmptyStatement();
        case KEYWORD_IF:
            return parseIfStatement();
        case KEYWORD_FOR:
            return parseForStatement();
        case KEYWORD_RETURN:
            return parseReturnStatement();
        case KEYWORD_VAR:
            return parseVarStatement();
        case KEYWORD_WHILE:
            return parseWhileStatement();
        default:
            break;
        }

        auto variable = parseVariableExpression();

        if (currentToken == TOKEN_ASSIGNMENT) {
            return parseAssignmentStatement(variable);
        } else {
            return parseCallStatement(variable);
        }
    }

    shared_ptr<Statement> parseAssignmentStatement(shared_ptr<VariableExpression> variable) {
        auto node = make_shared<AssignmentStatement>();

        eat(TOKEN_ASSIGNMENT);
        node->variable = variable;
        node->expression = parseExpression();
        eat(TOKEN_SEMICOLON);

        return node;
    }

    shared_ptr<Statement> parseCallStatement(shared_ptr<VariableExpression> variable) {
        auto node = make_shared<CallStatement>();

        eat(BRACKET_ROUND_LEFT);
        node->function = variable;
        if (currentToken != BRACKET_ROUND_RIGHT) {
            node->expressions.push_back(parseExpression());
            while (currentToken == TOKEN_COMMA) {
                eat(TOKEN_COMMA);
                node->expressions.push_back(parseExpression());
            }
        }
        eat(BRACKET_ROUND_RIGHT);
        eat(TOKEN_SEMICOLON);

        return node;
    }

    shared_ptr<Statement> parseEmptyStatement() {
        eat(TOKEN_SEMICOLON);
        return make_shared<EmptyStatement>();
    }

    shared_ptr<Statement> parseForStatement() {
        auto node = make_shared<ForStatement>();

        eat(KEYWORD_FOR);
        eat(BRACKET_ROUND_LEFT);
        eat(KEYWORD_VAR);
        node->key = parseIdentifier();
        if (currentToken == TOKEN_COMMA) {
            eat(TOKEN_COMMA);
            node->value = parseIdentifier();
        } else {
            node->value = node->key;
            node->key = U"";
        }

        eat(TOKEN_COLON);
        node->expression = parseExpression();
        eat(BRACKET_ROUND_RIGHT);
        eat(BRACKET_CURLY_LEFT);

        while (currentToken != BRACKET_CURLY_RIGHT) {
            node->statements.push_back(parseStatement());
        }

        eat(BRACKET_CURLY_RIGHT);

        return node;
    }

    shared_ptr<Statement> parseIfStatement() {
        auto node = make_shared<IfStatement>();

        eat(KEYWORD_IF);
        eat(BRACKET_ROUND_LEFT);
        node->condition = parseExpression();
        eat(BRACKET_ROUND_RIGHT);
        eat(BRACKET_CURLY_LEFT);

        while (currentToken != BRACKET_CURLY_RIGHT) {
            node->statementsThen.push_back(parseStatement());
        }

        eat(BRACKET_CURLY_RIGHT);

        if (currentToken != KEYWORD_ELSE) {
            return node;
        }

        eat(KEYWORD_ELSE);

        if (currentToken == KEYWORD_IF) {
            node->statementsElse.push_back(parseIfStatement());
            return node;
        }

        eat(BRACKET_CURLY_LEFT);
        while (currentToken != BRACKET_CURLY_RIGHT) {
            node->statementsElse.push_back(parseStatement());
        }
        eat(BRACKET_CURLY_RIGHT);

        return node;
    }

    shared_ptr<Statement> parseReturnStatement() {
        auto node = make_shared<ReturnStatement>();

        eat(KEYWORD_RETURN);
        node->expression = parseExpression();
        eat(TOKEN_SEMICOLON);

        return node;
    }

    shared_ptr<Statement> parseVarStatement() {
        auto node = make_shared<VarStatement>();

        eat(KEYWORD_VAR);
        node->identifier = parseIdentifier();
        eat(TOKEN_ASSIGNMENT);
        node->expression = parseExpression();
        eat(TOKEN_SEMICOLON);

        return node;
    }

    shared_ptr<Statement> parseWhileStatement() {
        auto node = make_shared<WhileStatement>();

        eat(KEYWORD_WHILE);
        eat(BRACKET_ROUND_LEFT);
        node->condition = parseExpression();
        eat(BRACKET_ROUND_RIGHT);
        eat(BRACKET_CURLY_LEFT);

        while (currentToken != BRACKET_CURLY_RIGHT) {
            node->statements.push_back(parseStatement());
        }

        eat(BRACKET_CURLY_RIGHT);

        return node;
    }
};

shared_ptr<Value> Program::execute(Program& program, istream& stream) {
    noumenon::Lexer lexer(stream);
    noumenon::Parser parser(lexer);

    shared_ptr<noumenon::Statement> statement;
    while (true) {
        if (!(statement = parser())) {
            return make_shared<ObjectValue>();
        }

        const auto& returnValue = statement->walk(program);
        if (returnValue != nullptr) {
            return returnValue;
        }
    }
}

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
    map<u32string, shared_ptr<Value>> values;
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
        cerr << "no such variable: \"" + StringValue::UTF32toUTF8(variable.identifier) + "\"" << endl;
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
        cerr << "no such variable: \"" + StringValue::UTF32toUTF8(variable.identifier) + "\"" << endl;
    }
}

void Program::insertVariable(const u32string& identifier, shared_ptr<Value> value) {
    if (values.find(identifier) != values.end()) {
        if (!quiet) {
            cerr << "redefinition of variable: \"" + StringValue::UTF32toUTF8(identifier) + "\"" << endl;
        }
        return;
    }

    values[identifier] = value;
}

Program* Program::getParent() {
    return parent;
}

} /* namespace scriptlanguage */
