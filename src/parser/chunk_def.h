#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

#include <parser/ast.h>
#include <parser/ast_adapted.h>
#include <parser/chunk.h>

namespace lac
{
	namespace parser
	{
		namespace x3 = boost::spirit::x3;
		namespace ascii = x3::ascii;
		using ascii::char_;
		using ascii::string;
		using x3::_attr;
		using x3::_pass;
		using x3::alnum;
		using x3::alpha;
		using x3::double_;
		using x3::eol;
		using x3::eps;
		using x3::get;
		using x3::hex;
		using x3::lexeme;
		using x3::lit;
		using x3::omit;
		using x3::raw;
		using x3::with;

		// clang-format off
		struct keyword_ : x3::symbols<std::string>
		{
			keyword_()
			{
				add
					("and", "and")
					("break", "break")
					("do", "do")
					("else", "else")
					("elseif", "elseif")
					("end", "end")
					("false", "false")
					("for", "for")
					("function", "function")
					("goto", "goto")
					("if", "if")
					("in", "in")
					("local", "local")
					("nil", "nil")
					("not", "not")
					("or", "or")
					("repeat", "repeat")
					("return", "return")
					("then", "then")
					("true", "true")
					("until", "until")
					("while", "while");
			}
		} keyword;

		struct binaryOperator_ : x3::symbols<std::string>
		{
			binaryOperator_()
			{
				add
					("+", "+")
					("-", "-")
					("*", "*")
					("/", "/")
					("//", "//")
					("%", "%")
					("^", "^")
					("&", "&")
					("|", "|")
					("~", "~")
					(">>", ">>")
					("<<", "<<")
					("..", "..")
					("<", "<")
					("<=", "<=")
					(">", ">")
					(">=", ">=")
					("==", "==")
					("~=", "~=")
					("and", "and")
					("or", "or");
			}
		} binaryOperator;

		struct unaryOperator_ : x3::symbols<std::string>
		{
			unaryOperator_()
			{
				add
					("-", "-")
					("not", "not")
					("#", "#")
					("~", "~");
			}
		} unaryOperator;

		struct fieldSeparator_ : x3::symbols<std::string>
		{
			fieldSeparator_()
			{
				add
					(",", ",")
					(";", ";");
			}
		} fieldSeparator;
		// clang-format on

		const x3::rule<class name, std::string> name = "name";
		const x3::rule<class namesList, std::string> namesList = "namesList";

		const x3::rule<class openLongBracket> openLongBracket = "openLongBracket";
		const x3::rule<class closeLongBacket> closeLongBacket = "closeLongBacket";
		const x3::rule<class longLiteralString, std::string> longLiteralString = "longLiteralString";
		const x3::rule<class literalString, std::string> literalString = "literalString";

		const x3::rule<class numeral, double> numeral = "numeral";
		const x3::rule<class numeralAsString, std::string> numeralAsString = "numeralAsString";

		const x3::rule<class shortComment, std::string> shortComment = "shortComment";
		const x3::rule<class longComment, std::string> longComment = "longComment";
		const x3::rule<class comment, std::string> comment = "comment";

		const x3::rule<class field, std::string> field = "field";
		const x3::rule<class fieldsList, std::string> fieldsList = "fieldsList";
		const x3::rule<class tableConstructor, std::string> tableConstructor = "tableConstructor";

		const x3::rule<class parametersList, std::string> parametersList = "parametersList";
		const x3::rule<class arguments, std::string> arguments = "arguments";
		const x3::rule<class functionBody, std::string> functionBody = "functionBody";
		const x3::rule<class functionCall, std::string> functionCall = "functionCall";
		const x3::rule<class functionCallEnd, std::string> functionCallEnd = "functionCallEnd";
		const x3::rule<class functionDefinition, std::string> functionDefinition = "functionDefinition";
		const x3::rule<class functionName, std::string> functionName = "functionName";

		const x3::rule<class prefixExpression, std::string> prefixExpression = "prefixExpression";
		const x3::rule<class postPrefix, std::string> postPrefix = "postPrefix";
		const x3::rule<class variable, std::string> variable = "variable";
		const x3::rule<class variablePostfix, std::string> variablePostfix = "variablePostfix";
		const x3::rule<class variablesList, std::string> variablesList = "variablesList";

		const x3::rule<class simpleExpression, std::string> simpleExpression = "simpleExpression";
		const x3::rule<class expression, std::string> expression = "expression";
		const x3::rule<class expressionsList, std::string> expressionsList = "expressionsList";

		const x3::rule<class label, std::string> label = "label";
		const x3::rule<class statement, std::string> statement = "statement";
		const x3::rule<class returnStatement, std::string> returnStatement = "returnStatement";

		const x3::rule<class block, std::string> block = "block";
		const x3::rule<class chunk, std::string> chunk = "chunk";

		// Names
		const auto name_def = lexeme[((alpha | char_('_'))
									  >> *(alnum | char_('_')))
									 - keyword];

		const auto namesList_def = name % ',';

		// Short literal strings
		auto quotedString = [](char c) {
			return lit(c)
				   >> *(('\\' >> char_(c)) // Escaped quote
						| (char_ - c))     // Any character but the quote
				   >> c;
		};

		// Long literal strings
		struct long_bracket_tag
		{
		};
		auto set_bracket = [](auto& ctx) { get<long_bracket_tag>(ctx) = _attr(ctx); };
		auto is_bracket = [](auto& ctx) { _pass(ctx) = (x3::get<long_bracket_tag>(ctx) == _attr(ctx)); };
		const auto openLongBracket_def = '[' >> (*char_('='))[set_bracket] >> '[';
		const auto closeLongBacket_def = ']' >> (*char_('='))[is_bracket] >> ']';
		const auto longLiteralString_def = with<long_bracket_tag>(std::string())
			[omit[openLongBracket]
			 >> *(char_ - closeLongBacket)
			 >> omit[closeLongBacket]];

		// Literal strings
		const auto literalString_def = lexeme[quotedString('\'')
											  | quotedString('"')
											  | longLiteralString];

		// Numerals
		const auto numeral_def = (lit("0x") >> hex)
								 | (lit("0X") >> hex)
								 | double_;
		const auto numeralAsString_def = raw[numeral];

		// Comments
		const auto shortComment_def = "--" >> lexeme[*(char_ - eol)] >> -eol;
		const auto longComment_def = with<long_bracket_tag>(std::string())
			["--"
			 >> omit[openLongBracket]
			 >> lexeme[*(char_ - closeLongBacket)]
			 >> omit[closeLongBacket]];
		const auto comment_def = longComment | shortComment;

		// A skipper that ignore whitespace and comments
		const x3::rule<class skipper> skipper = "skipper";

		const auto skipper_def = ascii::space
								 | omit[shortComment]
								 | omit[longComment];

		//*** Complete syntax of Lua ***
		// Table and fields
		const auto field_def = ('[' >> expression >> lit(']') >> lit('=') >> expression)
							   | (name >> '=' >> expression)
							   | expression;

		const auto fieldsList_def = field >> *(fieldSeparator >> field) >> -fieldSeparator;

		const auto tableConstructor_def = '{' >> -fieldsList >> '}';

		// Functions
		const auto parametersList_def = (namesList >> -(lit(',') >> lit("...")))
										| lit("...");

		const auto arguments_def = ('(' >> -expressionsList >> ')')
								   | tableConstructor
								   | literalString;

		const auto functionBody_def = '(' >> -parametersList >> ')' >> block >> lit("end");

		const auto functionDefinition_def = lit("function") >> functionBody;

		const auto functionCall_def = variable >> functionCallEnd;

		const auto functionCallEnd_def = -(':' >> name) >> arguments;

		const auto functionName_def = name >> *('.' >> name) >> -(':' >> name);

		// Variables
		const auto prefixExpression_def = (('(' >> expression >> ')')
										   | name)
										  >> *postPrefix;

		const auto postPrefix_def = ('[' >> expression >> ']')
									| ('.' >> name)
									| functionCallEnd;

		const auto variable_def = (('(' >> expression >> ')')
								   | name)
								  >> *variablePostfix;

		const auto variablePostfix_def = ('[' >> expression >> ']')
										 | ('.' >> name)
										 | (functionCallEnd >> variablePostfix);

		const auto variablesList_def = variable % ',';

		// Expressions
		const auto simpleExpression_def = lit("nil")
										  | lit("false")
										  | lit("true")
										  | numeralAsString
										  | literalString
										  | lit("...")
										  | functionDefinition
										  | (unaryOperator >> expression)
										  | tableConstructor
										  | prefixExpression;
		const auto expression_def = simpleExpression >> -(binaryOperator >> expression);

		const auto expressionsList_def = expression >> *(',' >> expression);

		// Statements
		const auto label_def = "::" >> name >> "::";

		const auto returnStatement_def = "return" >> -expressionsList >> -lit(';');

		const auto statement_def = ';'
								   | (variablesList >> '=' >> expressionsList)
								   | functionCall
								   | label
								   | "break"
								   | ("goto" >> name)
								   | ("do" >> block >> "end")
								   | ("while" >> expression >> "do" >> block >> "end")
								   | ("repeat" >> block >> "until" >> expression)
								   | ("if" >> expression >> "then" >> block >> *("elseif" >> expression >> "then" >> block) >> -("else" >> block) >> "end")
								   | ("for" >> name >> '=' >> expression >> ',' >> expression >> -(',' >> expression) >> "do" >> block >> "end")
								   | ("for" >> namesList >> "in" >> expressionsList >> "do" >> block >> "end")
								   | ("function" >> functionName >> functionBody)
								   | ("local" >> lit("function") >> name >> functionBody)
								   | ("local" >> namesList >> -('=' >> expressionsList));

		// Blocks
		const auto block_def = *statement >> -returnStatement;
		const auto chunk_def = block;

		BOOST_SPIRIT_DEFINE(name, namesList,
							openLongBracket, closeLongBacket,
							longLiteralString, literalString,
							numeral, numeralAsString,
							shortComment, longComment, comment,
							skipper,
							field, fieldsList, tableConstructor,
							parametersList, arguments,
							functionBody, functionCall, functionCallEnd,
							functionDefinition, functionName,
							prefixExpression, postPrefix,
							variable, variablePostfix, variablesList,
							simpleExpression, expression, expressionsList,
							label, returnStatement, statement,
							block, chunk);
	} // namespace parser

	parser::chunk_type chunkRule()
	{
		return parser::chunk;
	}
} // namespace lac
