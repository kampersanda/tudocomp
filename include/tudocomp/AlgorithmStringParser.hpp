#ifndef TUDOCOMP_DRIVER_ALGORITHM_PARSER
#define TUDOCOMP_DRIVER_ALGORITHM_PARSER

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <streambuf>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>

#include <glog/logging.h>

#include <tudocomp/Env.hpp>
#include <tudocomp/util.h>

namespace tudocomp {
using namespace tudocomp;
    namespace ast {
        /*
            Base Grammar of an Algorithm string as used on the command
            line, in the regstriy for declaring, and in the registry for
            giving implementations.

            Value ::= IDENT ['(' [Arg,]* [Arg] ')']
                    | '"' STRING_'"'
            Arg   ::= [IDENT [':' ['static'] IDENT] '='] Value

        */
        class Value;
        class Arg;
        class Value {
            bool m_is_invokation;
            std::string m_invokation_name_or_value;
            std::vector<Arg> m_invokation_arguments;
        public:
            inline Value():
                m_is_invokation(false),
                m_invokation_name_or_value("") {}
            inline Value(std::string&& value):
                m_is_invokation(false),
                m_invokation_name_or_value(std::move(value)) {}
            inline Value(std::string&& name, std::vector<Arg>&& args):
                m_is_invokation(true),
                m_invokation_name_or_value(std::move(name)),
                m_invokation_arguments(std::move(args)) {}

            inline bool is_invokation() {
                return m_is_invokation;
            }

            inline std::string& invokation_name() {
                CHECK(is_invokation());
                return m_invokation_name_or_value;
            }

            inline std::vector<Arg>& invokation_arguments() {
                CHECK(is_invokation());
                return m_invokation_arguments;
            }

            inline std::string& string_value() {
                CHECK(!is_invokation());
                return m_invokation_name_or_value;
            }

            inline std::string to_string();

            inline bool is_empty() {
                return m_invokation_name_or_value == "";
            }

            friend inline bool operator==(const Value &lhs, const Value &rhs);
        };
        class Arg {
            bool m_has_keyword;
            bool m_has_type;
            bool m_type_is_static;
            Value m_value;
            std::string m_keyword;
            std::string m_type;
        public:
            inline Arg(Value&& value):
                m_has_keyword(false),
                m_has_type(false),
                m_type_is_static(false),
                m_value(std::move(value)) {}
            inline Arg(View keyword, Value&& value):
                m_has_keyword(true),
                m_has_type(false),
                m_type_is_static(false),
                m_value(std::move(value)),
                m_keyword(keyword)
                {}
            inline Arg(View keyword, bool is_static, View type, Value&& value):
                m_has_keyword(true),
                m_has_type(true),
                m_type_is_static(is_static),
                m_value(std::move(value)),
                m_keyword(keyword),
                m_type(type)
                {}
            inline Arg(bool is_static, View type, Value&& value):
                m_has_keyword(false),
                m_has_type(true),
                m_type_is_static(is_static),
                m_value(std::move(value)),
                m_type(type)
                {}

            inline bool has_keyword() {
                return m_has_keyword;
            }
            inline bool has_type() {
                return m_has_type;
            }
            inline bool type_is_static() {
                return m_type_is_static;
            }
            inline std::string& keyword() {
                return m_keyword;
            }
            inline std::string& type() {
                return m_type;
            }
            inline Value& value() {
                return m_value;
            }

            inline std::string to_string();

            friend inline bool operator==(const Arg &lhs, const Arg &rhs);
        };

        inline std::ostream& operator<<(std::ostream& os,
                                        Value& x) {
            os << x.to_string();
            return os;
        }
        inline std::ostream& operator<<(std::ostream& os,
                                        Arg& x) {
            os << x.to_string();
            return os;
        }

        inline bool operator==(const Arg &lhs, const Arg &rhs);
        inline bool operator==(const Value &lhs, const Value &rhs);

        inline bool operator!=(const Arg &lhs, const Arg &rhs) {
            return !(lhs == rhs);
        }
        inline bool operator!=(const Value &lhs, const Value &rhs) {
            return !(lhs == rhs);
        }

        inline bool operator==(const Arg &lhs, const Arg &rhs) {
            if (lhs.m_has_keyword != rhs.m_has_keyword) return false;
            if (lhs.m_has_type != rhs.m_has_type) return false;
            if (lhs.m_type_is_static != rhs.m_type_is_static) return false;
            if (lhs.m_keyword != rhs.m_keyword) return false;
            if (lhs.m_type != rhs.m_type) return false;
            if (lhs.m_value != rhs.m_value) return false;
            return true;
        }

        inline bool operator==(const Value &lhs, const Value &rhs) {
            if (lhs.m_is_invokation != rhs.m_is_invokation) return false;
            if (lhs.m_invokation_name_or_value != rhs.m_invokation_name_or_value) return false;
            if (lhs.m_invokation_arguments != rhs.m_invokation_arguments) return false;
            return true;
        }

        inline std::string Value::to_string() {
            std::stringstream ss;
            if (is_invokation()) {
                ss << invokation_name();
                if (invokation_arguments().size() > 0) {
                    ss << "(";
                    bool first = true;
                    for (auto& a: invokation_arguments()) {
                        if (!first) {
                            ss << ", ";
                        }
                        ss << a;
                        first = false;
                    }
                    ss << ")";
                }
            } else {
                ss << "\"" << string_value() << "\"";
            }
            return ss.str();
        }

        inline std::string Arg::to_string() {
            std::stringstream ss;
            if (has_keyword()) {
                ss << keyword();
            } else {
                ss << value();
            }
            if (has_type()) {
                ss << ": ";
                if (type_is_static()) {
                    ss << "static ";
                }
                ss << type();
            }
            if (has_keyword()) {
                ss << " = ";
                ss << value();
            }
            return ss.str();
        }


        class ParseError: public std::runtime_error {
        public:
            inline ParseError(const std::string& cause, size_t pos, std::string input):
                std::runtime_error(cause
                    + ", found "
                    + input.substr(pos)) {}
        };

        // A simple recursive parser
        class Parser {
            View m_text;
            size_t m_cursor;

            inline void error(const std::string& cause) {
                throw ParseError(cause, m_cursor, m_text);
            }
            inline void error_ident() {
                error("Expected an identifier");
            }
        public:
            inline Parser(View text): m_text(text), m_cursor(0) {}

            inline bool has_next();
            inline Value parse_value(View already_parsed_ident = View(""));
            inline Arg parse_arg();
            inline View parse_ident();
            inline void parse_whitespace();
            inline bool parse_char(char c);
            inline bool peek_char(char c);
            inline std::string parse_string();
            inline bool parse_keyword(View keyword);
            inline View expect_ident();
        };

        inline bool Parser::has_next() {
            return m_cursor < m_text.size();
        }

        inline Value Parser::parse_value(View already_parsed_ident) {
            parse_whitespace();

            if (already_parsed_ident.size() == 0 && peek_char('"')) {
                return Value(parse_string());
            }

            View value_name("");

            if (already_parsed_ident.size() > 0) {
                value_name = already_parsed_ident;
            } else {
                value_name = expect_ident();
            }

            std::vector<Arg> args;
            bool first = true;
            parse_whitespace();
            if (parse_char('(')) {
                while(true) {
                    parse_whitespace();
                    if (parse_char(')')) {
                        break;
                    } else if (first || parse_char(',')) {
                        first = false;
                        args.push_back(parse_arg());
                    } else {
                        error("Expected ) or ,");
                    }
                }
            }

            return Value(value_name, std::move(args));

        }
        inline Arg Parser::parse_arg() {
            //return Arg(Value("test"));

            auto ident = parse_ident();

            bool has_type = false;
            bool is_static = false;
            View type_ident("");

            if (ident.size() > 0 && parse_char(':')) {
                is_static = parse_keyword("static");
                type_ident = expect_ident();
                has_type = true;
            }

            bool has_keyword = false;
            View keyword_ident("");

            if (ident.size() > 0 && parse_char('=')) {
                keyword_ident = ident;
                ident.clear();
                has_keyword = true;
            }

            auto value = parse_value(ident);

            if (has_keyword && has_type) {
                return Arg(keyword_ident,
                           is_static,
                           type_ident,
                           std::move(value));
            } else if (!has_keyword && has_type) {
                return Arg(is_static,
                           type_ident,
                           std::move(value));
            } else if (has_keyword && !has_type) {
                return Arg(keyword_ident,
                           std::move(value));
            } else {
                return Arg(std::move(value));
            }
        }
        inline View Parser::parse_ident() {
            parse_whitespace();
            size_t ident_start = m_cursor;
            if (m_cursor < m_text.size()) {
                auto c = m_text[m_cursor];
                if (c == '_'
                    || (c >= 'a' && c <= 'z')
                    || (c >= 'A' && c <= 'Z')
                ) {
                    // char is valid in an IDENT
                    m_cursor++;
                } else {
                    return m_text.substr(ident_start, m_cursor);
                }
            }
            for (; m_cursor < m_text.size(); m_cursor++) {
                auto c = m_text[m_cursor];
                if (c == '_'
                    || (c >= 'a' && c <= 'z')
                    || (c >= 'A' && c <= 'Z')
                    || (c >= '0' && c <= '9')
                ) {
                    // char is valid in an IDENT
                } else {
                    break;
                }
            }
            return m_text.substr(ident_start, m_cursor);
        }
        inline void Parser::parse_whitespace() {
            if (!has_next()) {
                return;
            }
            while (m_text[m_cursor] == ' '
                || m_text[m_cursor] == '\n'
                || m_text[m_cursor] == '\r'
                || m_text[m_cursor] == '\t')
            {
                m_cursor++;
            }
        }
        inline bool Parser::parse_char(char c) {
            bool r = peek_char(c);
            if (r) {
                m_cursor++;
            }
            return r;
        }
        inline bool Parser::peek_char(char c) {
            if (!has_next()) {
                return false;
            }
            parse_whitespace();
            if (m_text[m_cursor] == c) {
                return true;
            }
            return false;
        }
        inline std::string Parser::parse_string() {
            size_t start;
            size_t end;
            if (parse_char('"')) {
                start = m_cursor;
                end = start - 1;
                while(has_next()) {
                    if (parse_char('"')) {
                        end = m_cursor - 1;
                        break;
                    }
                    m_cursor++;
                }
                if (end >= start) {
                    return m_text.substr(start, end);
                }
            }
            error("Expected \"");
            return "";
        }
        inline bool Parser::parse_keyword(View keyword) {
            parse_whitespace();
            if (m_text.substr(m_cursor).starts_with(keyword)) {
                m_cursor += keyword.size();
                return true;
            }
            return false;
        }
        inline View Parser::expect_ident() {
            View ident = parse_ident();
            if (ident.size() == 0) {
                error_ident();
            }
            return ident;
        }
    }


    namespace decl {
        /*
            Representation of the declaration in the Registry.
            It cares about the exact types and signatures of the invokations,
            and about the documentation for the help printout
        */
        class Algorithm;
        class Arg;
        class Algorithm {
            std::string m_name;
            std::vector<Arg> m_arguments;
            std::string m_doc;

        public:

            inline Algorithm(std::string&& name,
                             std::vector<Arg>&& args,
                             std::string&& doc):
                m_name(std::move(name)),
                m_arguments(std::move(args)),
                m_doc(std::move(doc)) {}

            inline std::string& name() {
                return m_name;
            }
            inline std::vector<Arg>& arguments() {
                return m_arguments;
            }
            inline std::string& doc() {
                return m_doc;
            }
            inline std::string to_string(bool omit_type = false);

            friend inline bool operator==(const Algorithm &lhs, const Algorithm &rhs);
        };
        class Arg {
            std::string m_name;
            bool m_is_static;
            std::string m_type;
            bool m_has_default;
            ast::Value m_default;

        public:

            inline Arg(std::string&& name,
                       bool is_static,
                       std::string&& type):
                m_name(std::move(name)),
                m_is_static(is_static),
                m_type(std::move(type)),
                m_has_default(false) {}
            inline Arg(std::string&& name,
                       bool is_static,
                       std::string&& type,
                       ast::Value&& default_value):
                m_name(std::move(name)),
                m_is_static(is_static),
                m_type(std::move(type)),
                m_has_default(true),
                m_default(std::move(default_value)) {}

            inline std::string& name() {
                return m_name;
            }
            inline bool is_static() {
                return m_is_static;
            }
            inline std::string& type() {
                return m_type;
            }
            inline bool has_default() {
                return m_has_default;
            }
            inline ast::Value& default_value() {
                return m_default;
            }
            inline std::string to_string(bool omit_type = false);

            friend inline bool operator==(const Arg &lhs, const Arg &rhs);
        };

        inline std::ostream& operator<<(std::ostream& os,
                                        Algorithm& x) {
            os << x.to_string();
            return os;
        }
        inline std::ostream& operator<<(std::ostream& os,
                                        Arg& x) {
            os << x.to_string();
            return os;
        }

        inline std::string Algorithm::to_string(bool omit_type) {
            std::stringstream ss;
            ss << name();
            if (arguments().size() > 0) {
                ss << "(";
                bool first = true;
                for (auto& a: arguments()) {
                    if (!first) {
                        ss << ", ";
                    }
                    ss << a.to_string(omit_type);
                    first = false;
                }
                ss << ")";
            }
            return ss.str();
        }

        inline bool operator==(const Arg &lhs, const Arg &rhs);
        inline bool operator==(const Algorithm &lhs, const Algorithm &rhs);

        inline bool operator!=(const Arg &lhs, const Arg &rhs) {
            return !(lhs == rhs);
        }
        inline bool operator!=(const Algorithm &lhs, const Algorithm &rhs) {
            return !(lhs == rhs);
        }

        inline bool operator==(const Arg &lhs, const Arg &rhs) {
            if (lhs.m_name != rhs.m_name) return false;
            if (lhs.m_is_static != rhs.m_is_static) return false;
            if (lhs.m_type != rhs.m_type) return false;
            if (lhs.m_has_default != rhs.m_has_default) return false;
            if (lhs.m_default != rhs.m_default) return false;
            return true;
        }

        inline bool operator==(const Algorithm &lhs, const Algorithm &rhs) {
            if (lhs.m_name != rhs.m_name) return false;
            if (lhs.m_arguments != rhs.m_arguments) return false;
            if (lhs.m_doc != rhs.m_doc) return false;
            return true;
        }

        inline std::string Arg::to_string(bool omit_type) {
            std::stringstream ss;
            ss << name();
            if (!omit_type) {
                ss << ": ";
                if (is_static()) {
                    ss << "static ";
                }
                ss << type();
            }
            if (has_default()) {
                ss << " = " << default_value();
            }
            return ss.str();
        }

        inline Algorithm from_ast(ast::Value&& v, std::string&& doc) {
            CHECK(v.is_invokation());
            std::vector<Arg> args;
            for (auto& arg : v.invokation_arguments()) {
                // Two cases:

                if (arg.has_type() && arg.has_keyword()) {
                    args.push_back(Arg(
                        std::move(arg.keyword()),
                        arg.type_is_static(),
                        std::move(arg.type()),
                        std::move(arg.value())
                    ));
                } else if (arg.has_type()
                    && arg.value().is_invokation()
                    && arg.value().invokation_arguments().size() == 0)
                {
                    args.push_back(Arg(
                        std::move(arg.value().invokation_name()),
                        arg.type_is_static(),
                        std::move(arg.type())
                    ));
                } else {
                    throw std::runtime_error("Declaration needs to be of the form ident: type = default or ident: type");
                }

                // value: type

                // ident: type = value

                //arg.push_back(Arg);
                // TODO
            }

            return Algorithm(std::move(v.invokation_name()),
                             std::move(args),
                             std::move(doc)
            );
        }

    }

    namespace pattern {
        /*
            Representation of an algorithm consisting of only those
            options that are relevant for selecting a statically
            defined implementation
        */
        class Algorithm;
        class Arg;
        class Algorithm {
            std::string m_name;
            std::vector<Arg> m_arguments;
        public:
            inline Algorithm() {}
            inline Algorithm(std::string&& name, std::vector<Arg>&& args):
                m_name(std::move(name)),
                m_arguments(std::move(args)) {}
            inline std::string& name() {
                return m_name;
            }
            inline const std::string& name() const {
                return m_name;
            }
            inline std::vector<Arg>& arguments() {
                return m_arguments;
            }
            inline const std::vector<Arg>& arguments() const {
                return m_arguments;
            }
            inline std::string to_string(bool omit_keyword = false) const;
        };
        class Arg {
            std::string m_name;
            Algorithm m_algorithm;
        public:
            inline Arg(std::string&& name, Algorithm&& alg):
                m_name(std::move(name)),
                m_algorithm(std::move(alg)) {}
            inline std::string& name() {
                return m_name;
            }
            inline const std::string& name() const {
                return m_name;
            }
            inline Algorithm& algorithm() {
                return m_algorithm;
            }
            inline const Algorithm& algorithm() const {
                return m_algorithm;
            }
            inline std::string to_string(bool omit_keyword = false) const;
        };

        inline std::ostream& operator<<(std::ostream& os,
                                        const Algorithm& x) {
            os << x.to_string();
            return os;
        }
        inline std::ostream& operator<<(std::ostream& os,
                                        const Arg& x) {
            os << x.to_string();
            return os;
        }

        inline std::string Algorithm::to_string(bool omit_keyword) const {
            std::stringstream ss;
            ss << name();
            if (arguments().size() > 0) {
                ss << "(";
                bool first = true;
                for (auto& a: arguments()) {
                    if (!first) {
                        ss << ", ";
                    }
                    ss << a.to_string(omit_keyword);
                    first = false;
                }
                ss << ")";
            }
            return ss.str();
        }

        inline std::string Arg::to_string(bool omit_keyword) const {
            std::stringstream ss;
            if (omit_keyword) {
                ss << algorithm().to_string(omit_keyword);
            } else {
                ss << name() << " = " << algorithm().to_string(omit_keyword);
            }
            return ss.str();
        }

        inline bool operator==(const Algorithm &lhs, const Algorithm &rhs);
        inline bool operator<(const Algorithm &lhs, const Algorithm &rhs);
        inline bool operator==(const Arg &lhs, const Arg &rhs);
        inline bool operator<(const Arg &lhs, const Arg &rhs);

        inline bool operator!=(const Algorithm &lhs, const Algorithm &rhs) {
            return !(lhs == rhs);
        }
        inline bool operator!=(const Arg &lhs, const Arg &rhs) {
            return !(lhs == rhs);
        }

        inline bool operator==(const Algorithm &lhs, const Algorithm &rhs) {
            if (lhs.name() != rhs.name()) return false;
            if (lhs.arguments() != rhs.arguments()) return false;
            return true;
        }
        inline bool operator<(const Algorithm &lhs, const Algorithm &rhs) {
            if (lhs.name() != rhs.name()) return lhs.name() < rhs.name();
            if (lhs.arguments() != rhs.arguments()) return lhs.arguments() < rhs.arguments();
            return false;
        }
        inline bool operator==(const Arg &lhs, const Arg &rhs) {
            if (lhs.name() != rhs.name()) return false;
            if (lhs.algorithm() != rhs.algorithm()) return false;
            return true;
        }
        inline bool operator<(const Arg &lhs, const Arg &rhs) {
            if (lhs.name() != rhs.name()) return lhs.name() < rhs.name();
            if (lhs.algorithm() != rhs.algorithm()) return lhs.algorithm() < rhs.algorithm();
            return false;
        }
    }

    namespace eval {
        using AlgorithmTypes = std::unordered_map<
            std::string, std::vector<decl::Algorithm>>;

        struct Evaluated {
            pattern::Algorithm static_selection;
            OptionValue options;
        };

        inline Evaluated eval(ast::Value&& v,
                              View type,
                              AlgorithmTypes& types,
                              bool pattern,
                              ast::Value&& fixed_static_args = ast::Value())
        {
            // Check for build-in types
            if (type == "string") {
                // TODO: Nice error
                CHECK(!v.is_invokation());
                return Evaluated {
                    pattern::Algorithm(), // No static algorithm here to return
                    OptionValue(std::move(v.string_value()))
                };
            }

            bool has_fixed_static_args = !fixed_static_args.is_empty();

            if (has_fixed_static_args) {
                // TODO: Nice error
                // Check that the static "overrides" fit the
                // to-be-evaluated algorithm
                CHECK(fixed_static_args.is_invokation());
                CHECK(fixed_static_args.invokation_name() == v.invokation_name());
                auto& a = fixed_static_args.invokation_arguments();
                std::reverse(a.begin(), a.end());
            }

            // Find the exact signature of the algorithm
            // TODO: Nice error
            CHECK(types.count(type) > 0);
            // TODO: Nice error
            CHECK(v.is_invokation());
            auto& candidates = types[type];
            decl::Algorithm* found = nullptr;
            for (auto& candidate : candidates) {
                if (candidate.name() == v.invokation_name()) {
                   found = &candidate;
                }
            }
            // TODO: Nice error
            CHECK(found != nullptr);

            // Signature found, evaluate by walking v and signature
            auto& v_signature = *found;

            // Prepare return value
            std::string r_name = v_signature.name();
            std::vector<pattern::Arg> r_static_args;
            AlgorithmValue::ArgumentMap r_dynamic_args;

            // Step 1: Find argument name of each option in the invokation's
            // signature

            std::vector<View> v_argument_names;
            {
                // no positional allowed after the first keyword arg
                bool positional_ok = true;
                size_t i = 0;
                for (auto& unevaluated_arg : v.invokation_arguments()) {
                    // TODO: Nice error
                    CHECK(!unevaluated_arg.has_type());

                    // find argument name for each position in the
                    // unevaluated argument list
                    View argument_name("");
                    if (!unevaluated_arg.has_keyword()) {
                        // postional arg
                        // TODO: Nice error
                        CHECK(positional_ok);
                        // assume argument name from current position

                        // TODO: Nice error
                        argument_name = v_signature.arguments().at(i).name();
                        i++;
                    } else {
                        // keyword arg
                        positional_ok = false;
                        // assume argument name from keyword

                        argument_name = unevaluated_arg.keyword();
                    }

                    v_argument_names.push_back(argument_name);
                }
            }

            // Step 2: Walk signature's arguments and produce evaluated
            //         data for each one

            for (auto& signature_arg : v_signature.arguments()) {
                if (pattern && !signature_arg.is_static()) {
                    // If we are evaluateing patterns, ignore dynamic arguments
                    continue;
                }

                int found = -1;
                for (size_t i = 0; i < v_argument_names.size(); i++) {
                    if (v_argument_names[i] == signature_arg.name()) {
                        found = i;
                    }
                }

                // logic here: if override by static,
                // try to use static
                // error on non-default given that mismatches
                // important!
                // merge, not use either-or!

                ast::Value arg_value;
                ast::Value arg_fixed_static_value;

                if (found != -1) {
                    // use
                    arg_value = v.invokation_arguments()[found].value();
                } else if (signature_arg.has_default()) {
                    arg_value = signature_arg.default_value();
                }

                // after value has been selected, override parts if static;

                if (has_fixed_static_args) {
                    if (signature_arg.is_static()) {
                        // TODO: Nice error
                        CHECK(fixed_static_args
                              .invokation_arguments()
                              .size() > 0);
                        {
                            auto& current_fixed_static = fixed_static_args
                                .invokation_arguments().back();

                            // TODO: Nice error
                            CHECK(current_fixed_static.keyword()
                                == signature_arg.name());

                            arg_fixed_static_value = current_fixed_static.value();

                            if (found == -1) {
                                arg_value = std::move(current_fixed_static.value());
                            }
                        }

                        fixed_static_args
                              .invokation_arguments()
                              .pop_back();
                    }
                }

                // TODO: Nice error
                CHECK(!arg_value.is_empty());

                // Recursivly evaluate the argument
                Evaluated arg_evaluated
                    = eval(std::move(arg_value),
                           signature_arg.type(),
                           types,
                           pattern,
                           std::move(arg_fixed_static_value));

                if (signature_arg.is_static()
                    && arg_evaluated.static_selection.name() != "")
                {
                    r_static_args.push_back(
                        pattern::Arg(
                            std::string(signature_arg.name()),
                            std::move(arg_evaluated.static_selection)));
                }

                r_dynamic_args[signature_arg.name()]
                    = std::move(arg_evaluated.options);

            }

            // Step 2.5: Check that all fixed static args have been used

            if (has_fixed_static_args) {
                CHECK(fixed_static_args.invokation_arguments().size() == 0);
            }
            // Step 3: Return

            return Evaluated {
                pattern::Algorithm(
                    std::string(r_name),
                    std::move(r_static_args)),
                OptionValue(AlgorithmValue(
                    std::move(r_name),
                    std::move(r_dynamic_args)))
            };

        }

        inline Evaluated cl_eval(ast::Value&& v,
                                 View type,
                                 AlgorithmTypes& types,
                                 ast::Value&& fixed_static_args = ast::Value()) {
            return eval(std::move(v),
                        type,
                        types,
                        false,
                        std::move(fixed_static_args));
        }
        inline pattern::Algorithm pattern_eval(ast::Value&& v,
                                               View type,
                                               AlgorithmTypes& types) {
            return std::move(eval(std::move(v),
                                  type,
                                  types,
                                  true).static_selection);
        }
    }

}

#endif