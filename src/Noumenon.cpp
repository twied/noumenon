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
#include "Runtime.h"
#include "Statement.h"
#include "Value.h"

#include <fstream>
#include <iostream>
#include <memory>
#include "Program.h"

using namespace std;

static void usage() {
    cout << "Usage: noumenon [options] [FILE] [arguments]" << endl
        << endl
        << "Options:"
        << endl
        << "  --quiet, -q       Don't show intro" << endl
        << endl
        << "If FILE is not given or \"--\", use interactive mode." << endl;
}

int main(int, char* argv[], char** env) {
    struct {
        /* script file name */
        string file;

        /* parameter --quiet / -q */
        bool quiet;
    } options = {"", false};

    /* parse noumenon arguments */
    for(argv++; *argv; argv += 1) {
        string arg(*argv);

        if (arg == "--") {
            options.file = arg;
            argv += 1;
            break;
        }

        if (arg.size() > 0 && arg[0] != '-') {
            break;
        }

        if (arg == "--quiet" || arg == "-q") {
            options.quiet = true;
        } else {
            cout << "Unknown option '" << arg << "'" << endl << endl;
            usage();
            return 1;
        }
    }

    /* filename */
    if (*argv && options.file.empty()) {
        options.file = *argv;
        argv += 1;
    }

    /* program arguments */
    const auto& arguments = make_shared<noumenon::ArrayValue>();
    for(; *argv; argv += 1) {
        arguments->values.emplace_back(make_shared<noumenon::StringValue>(noumenon::StringValue::UTF8toUTF32(*argv)));
    }

    const auto& environment = make_shared<noumenon::ObjectValue>();
    for(;*env; ++env) {
        const string line(*env);
        const auto& pos = line.find('=');
        if(pos != line.npos) {
            environment->values[noumenon::StringValue::UTF8toUTF32(line.substr(0, pos))] = make_shared<noumenon::StringValue>(noumenon::StringValue::UTF8toUTF32(line.substr(pos + 1)));
        }
    }

    noumenon::Program program(options.quiet);
    program.insertVariable(U"arg", arguments);
    program.insertVariable(U"env", environment);

    program.insertVariable(U"typeof", make_shared<noumenon::rtl::Typeof>());
    program.insertVariable(U"print", make_shared<noumenon::rtl::Print>());
    program.insertVariable(U"println", make_shared<noumenon::rtl::Println>());
    program.insertVariable(U"range", make_shared<noumenon::rtl::Range>());
    program.insertVariable(U"length", make_shared<noumenon::rtl::Length>());
    program.insertVariable(U"require", make_shared<noumenon::rtl::Require>());

    if (options.file.empty() || options.file == "--") {
        program.insertVariable(U"list", make_shared<noumenon::rtl::List>());

        if (!options.quiet) {
            cout << "Noumenon 0.1" << endl
                << "Copyright (C) 2015 Tim Wiederhake" << endl
                << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << endl
                << "This is free software: you are free to change and redistribute it." << endl
                << "There is NO WARRANTY, to the extent permitted by law." << endl
                << "Report bugs to <https://github.com/twied/noumenon/issues>." << endl;
        }

        struct InteractiveBuf : public streambuf {
            InteractiveBuf() : semicolons(0), c(0) {
            }

            int_type underflow() {
                if (semicolons) {
                    semicolons -= 1;
                    c = ';';
                    setg(&c, &c, &c + 1);
                    return c;
                }

                int value = cin.get();
                c = value;

                if (value == '\n' || value == '\r') {
                    semicolons = 3;
                }

                setg(&c, &c, &c + 1);
                return value;
            }

            unsigned semicolons;
            char c;
        } buffer;
        istream input(&buffer);

        while(!cin.eof()) {
            try {
                const auto& returnValue = noumenon::Program::execute(program, input);
                noumenon::rtl::Println println;
                std::vector<std::shared_ptr<noumenon::Value>> arguments = {returnValue};
                println.doCall(program, arguments);
            } catch (const string& s) {
                cout << "driver: " << s << endl;
            }
        }

    } else {
        ifstream input(options.file);

        if (!input) {
            cout << "Unreadable file: " << options.file << endl << endl;
            usage();
            return 1;
        }

        try {
            const auto& returnValue = noumenon::Program::execute(program, input);

            struct Walker : public noumenon::DefaultValueWalker {
                Walker() : result(0), valid(false) {
                }

                shared_ptr<noumenon::Value> value(noumenon::IntValue& node) {
                    result = node.value;
                    valid = true;
                    return nullptr;
                }

                int result;
                bool valid;
            } walker;

            returnValue->walk(walker);
            if (walker.valid) {
                return walker.result;
            }
        } catch (const string& s) {
            cout << "driver: " << s << endl;
            return 1;
        }
    }

    return 0;
}
