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

/*
 * This file defines the grammar of Noumenon and is in ANTLR format.
 * See: http://www.antlr.org/
 */
grammar Noumenon;

script
    : statement* EOF
    ;

statement
    : assignment_statement
    | call_statement
    | empty_statement
    | for_statement
    | if_statement
    | return_statement
/*  | switch_statement // maybe later... */
    | variable_declaration
    | while_statement
    ;

assignment_statement
    : designator '=' expression ';'
    ;

call_statement
    : designator '(' ( expression ( ',' expression )* )? ')' ';'
    ;

empty_statement
    : ';'
    ;

for_statement
    : 'for' '(' 'var' ( IDENTIFIER ',' )? IDENTIFIER ':' expression ')' '{' statement* '}'
    ;

if_statement
    : 'if' '(' expression ')' '{' statement* '}'
    | 'if' '(' expression ')' '{' statement* '}' 'else' '{' statement* '}'
    | 'if' '(' expression ')' '{' statement* '}' 'else' if_statement
    ;

return_statement
    : 'return' expression ';'
    ;

variable_declaration
    : 'var' IDENTIFIER '=' expression ';'
    ;

while_statement
    : 'while' '(' expression ')' '{' statement* '}'
    ;

expression
    : operand ( ( '==' | '!=' | '<' | '<=' | '>' | '>=' ) operand )?
    ;

operand
    : term ( ( '+' | '-' | '||' ) term)?
    ;

term
    : unary ( ( '*' | '/' | '%' | '&&' ) unary )?
    ;

unary
    : ( '-' | '!' )? factor
    ;

factor
    : INT
    | FLOAT
    | STRING
    | 'true'
    | 'false'
    | 'null'
    | array
    | object
    | function
    | '(' expression ')'
    | call
    | designator
    ;

array
    : '[' ( expression ( ',' expression )* )? ']'
    ;

object
    : '{' ( IDENTIFIER ':' expression ( ',' IDENTIFIER ':' expression )* )? '}'
    ;

function
    : 'function' '(' ( IDENTIFIER ( ',' IDENTIFIER )* )? ')' '{' statement* '}'
    ;

call
    : designator '(' ( expression ( ',' expression )* )? ')'
    ;

designator
    : IDENTIFIER ( ( '[' expression ']' ) | ( '.' IDENTIFIER ) )*
    ;

IDENTIFIER
    : ( 'a' .. 'z' | 'A' .. 'Z' | '_' ) ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' )*
    ;

INT
    : '0' .. '9'+
    ;

FLOAT
    : ( '0' .. '9' )+ '.' ( '0' .. '9' )* EXPONENT?
    | '.' ( '0' .. '9' )+ EXPONENT?
    | ( '0' .. '9' )+ EXPONENT
    ;

COMMENT
    : '//' ~( '\n' | '\r' )* '\r'? '\n' { $channel = HIDDEN; }
    | '/*' ( options { greedy = false; } : . )* '*/' { $channel = HIDDEN; }
    ;

WS
    : ( ' ' | '\t' | '\r' | '\n' ) { $channel = HIDDEN; }
    ;

STRING
    :  '\"' ( ESC_SEQ | ~('\\' | '\"' ) )* '\"'
    ;

fragment
EXPONENT
    : ( 'e' | 'E' ) ( '+' | '-' )? ( '0' .. '9' )+
    ;

fragment
HEX_DIGIT
    : ( '0' .. '9' | 'a' .. 'f' | 'A' .. 'F' )
    ;

fragment
ESC_SEQ
    : '\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\'' | '\\' )
    | UNICODE_ESC
    | OCTAL_ESC
    ;

fragment
OCTAL_ESC
    : '\\' ( '0' .. '3' ) ( '0' .. '7' ) ( '0' .. '7' )
    | '\\' ( '0' .. '7' ) ( '0' .. '7' )
    | '\\' ( '0' .. '7' )
    ;

fragment
UNICODE_ESC
    : '\\' 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
    ;
