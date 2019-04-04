#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

#include <parser/ast.h>
#include <parser/ast_adapted.h>
#include <parser/chunk.h>
#include <parser/positions.h>

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
		using x3::get;
		using x3::hex;
		using x3::int_;
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

		struct binaryOperator_ : x3::symbols<ast::Operation>
		{
			binaryOperator_()
			{
				using Op = ast::Operation;
				add
					("+", Op::add)
					("-", Op::sub)
					("*", Op::mul)
					("/", Op::div)
					("//", Op::idiv)
					("%", Op::mod)
					("^", Op::pow)
					("&", Op::band)
					("|", Op::bor)
					("~", Op::bxor)
					("<<", Op::shl)
					(">>", Op::shr)
					("..", Op::concat)
					("<", Op::lt)
					("<=", Op::le)
					(">", Op::gt)
					(">=", Op::ge)
					("==", Op::eq)
					("~=", Op::ineq)
					("and", Op::band)
					("or", Op::bor);
			}
		} binaryOperator;

		struct unaryOperator_ : x3::symbols<ast::Operation>
		{
			using Op = ast::Operation;
			unaryOperator_()
			{
				add
					("-", Op::unm)
					("#", Op::len)
					("~", Op::bnot)
					("not", Op::bnot);
			}
		} unaryOperator;

		struct expressionConstant_ : x3::symbols<ast::ExpressionConstant>
		{
			expressionConstant_()
			{
				using EC = ast::ExpressionConstant;
				add
					("nil", EC::nil)
					("false", EC::False)
					("true", EC::True)
					("...", EC::dots);
			}
		} expressionConstant;
		// clang-format on

#define RULE(name, type) const x3::rule<struct name, type> name = #name;

		RULE(name, std::string);
		RULE(namesList, std::vector<std::string>);

		const x3::rule<class openLongBracket> openLongBracket = "openLongBracket";
		const x3::rule<class closeLongBacket> closeLongBacket = "closeLongBacket";
		RULE(longLiteralString, std::string);
		RULE(literalString, std::string);

		RULE(numeralInt, int);
		RULE(numeralFloat, double);

		RULE(shortComment, std::string);
		RULE(longComment, std::string);
		RULE(comment, std::string);

		RULE(fieldByExpression, ast::FieldByExpression);
		RULE(fieldByAssignment, ast::FieldByAssignment);
		RULE(field, ast::Field);
		RULE(fieldsList, ast::FieldsList);
		RULE(tableConstructor, ast::TableConstructor);

		RULE(parametersList, ast::ParametersList);
		RULE(arguments, ast::Arguments);
		RULE(functionBody, ast::FunctionBody);
		RULE(functionCall, ast::FunctionCall);
		RULE(functionCallEnd, ast::FunctionCallEnd);
		RULE(functionDefinition, ast::FunctionBody);
		RULE(functionNameMember, ast::FunctionNameMember);
		RULE(functionName, ast::FunctionName);

		RULE(bracketedExpression, ast::BracketedExpression);
		RULE(tableIndexExpression, ast::TableIndexExpression);
		RULE(tableIndexName, ast::TableIndexName);
		RULE(prefixExpression, ast::PrefixExpression);
		RULE(postPrefix, ast::PostPrefix);
		RULE(variable, ast::Variable);
		RULE(variableFunctionCall, ast::VariableFunctionCall);
		RULE(variablePostfix, ast::VariablePostfix);
		RULE(variablesList, ast::VariablesList);

		RULE(unaryOperation, ast::UnaryOperation);
		RULE(binaryOperation, ast::BinaryOperation);
		RULE(simpleExpression, ast::Operand);
		RULE(expression, ast::Expression);
		RULE(expressionsList, ast::ExpressionsList);

		RULE(assignmentStatement, ast::AssignmentStatement);
		RULE(labelStatement, ast::LabelStatement);
		RULE(gotoStatement, ast::GotoStatement);
		RULE(breakStatement, ast::BreakStatement);
		RULE(doStatement, ast::DoStatement);
		RULE(whileStatement, ast::WhileStatement);
		RULE(repeatStatement, ast::RepeatStatement);
		RULE(ifStatement, ast::IfStatement);
		RULE(elseIfStatement, ast::IfStatement);
		RULE(ifThenElseStatement, ast::IfThenElseStatement);
		RULE(numericalForStatement, ast::NumericalForStatement);
		RULE(genericForStatement, ast::GenericForStatement);
		RULE(functionDeclarationStatement, ast::FunctionDeclarationStatement);
		RULE(localFunctionDeclarationStatement, ast::LocalFunctionDeclarationStatement);
		RULE(localAssignmentStatement, ast::LocalAssignmentStatement);

		RULE(returnStatement, ast::ReturnStatement);
		RULE(statement, ast::Statement);

		RULE(block, ast::Block);
		RULE(chunk, ast::Block);

		struct simpleExpression : pos::annotate_position
		{
		};

		struct variable : pos::annotate_position
		{
		};

#undef RULE

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
		auto is_bracket = [](auto& ctx) { _pass(ctx) = (get<long_bracket_tag>(ctx) == _attr(ctx)); };
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
		const auto numeralInt_def = (lit("0x") >> hex)
									| (lit("0X") >> hex)
									| (int_ >> !lit('.'));
		const auto numeralFloat_def = double_;

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
		const auto fieldByExpression_def = '[' >> expression >> lit(']') >> lit('=') >> expression;
		const auto fieldByAssignment_def = name >> '=' >> expression;
		const auto field_def = fieldByExpression
							   | fieldByAssignment
							   | expression;

		const auto fieldSeparator = lit(',') | lit(';');
		const auto fieldsList_def = field >> *(fieldSeparator >> field) >> -fieldSeparator;

		const auto tableConstructor_def = '{' >> -fieldsList >> '}';

		// Functions
		const auto funcVarargs = lit("...") >> x3::attr(true);
		const auto parametersList_def = (namesList >> -(lit(',') >> funcVarargs))
										| (x3::attr(ast::NamesList{}) >> funcVarargs);

		const auto emptyArguments = lit('(') >> lit(')') >> x3::attr(ast::EmptyArguments());
		const auto argumentsExpressions = '(' >> expressionsList >> ')';
		const auto arguments_def = emptyArguments
								   | argumentsExpressions
								   | tableConstructor
								   | literalString;

		const auto functionBody_def = '(' >> -parametersList >> ')' >> block >> lit("end");

		const auto functionDefinition_def = lit("function") >> functionBody;

		const auto functionCall_def = variable >> functionCallEnd;

		const auto functionCallEnd_def = -(':' >> name) >> arguments;

		const auto functionNameMember_def = ':' >> name;

		const auto functionName_def = name >> *('.' >> name) >> -functionNameMember;

		// Variables
		const auto bracketedExpression_def = '(' >> expression >> ')';
		const auto tableIndexExpression_def = '[' >> expression >> ']';
		const auto tableIndexName_def = '.' >> name;
		const auto prefixExpression_def = (bracketedExpression
										   | name)
										  >> *postPrefix;

		const auto postPrefix_def = tableIndexExpression
									| tableIndexName
									| functionCallEnd;

		const auto variable_def = (bracketedExpression
								   | name)
								  >> *variablePostfix;

		const auto variableFunctionCall_def = functionCallEnd >> variablePostfix; // Should not stop with a function call

		const auto variablePostfix_def = tableIndexExpression
										 | tableIndexName
										 | variableFunctionCall;

		const auto variablesList_def = variable % ',';

		// Expressions
		const auto simpleExpression_def = expressionConstant
										  | unaryOperation
										  | numeralInt
										  | numeralFloat
										  | literalString
										  | tableConstructor
										  | functionDefinition
										  | prefixExpression;

		const auto unaryOperation_def = unaryOperator >> expression;
		const auto binaryOperation_def = binaryOperator >> expression;
		const auto expression_def = simpleExpression >> -binaryOperation;

		const auto expressionsList_def = expression >> *(',' >> expression);

		// Statements
		const auto emptyStatement = lit(';') >> x3::attr(ast::EmptyStatement{});
		const auto assignmentStatement_def = variablesList >> '=' >> expressionsList;
		const auto labelStatement_def = "::" >> name >> "::";
		const auto gotoStatement_def = "goto" >> name;
		const auto breakStatement_def = "break" >> x3::attr(ast::BreakStatement{});
		const auto doStatement_def = "do" >> block >> "end";
		const auto whileStatement_def = "while" >> expression >> "do" >> block >> "end";
		const auto repeatStatement_def = "repeat" >> block >> "until" >> expression;
		const auto ifStatement_def = "if" >> expression >> "then" >> block;
		const auto elseIfStatement_def = "elseif" >> expression >> "then" >> block;
		const auto ifThenElseStatement_def = ifStatement
											 >> *(elseIfStatement)
											 >> -("else" >> block)
											 >> "end";
		const auto numericalForStatement_def = "for" >> name >> '=' >> expression
											   >> ',' >> expression >> -(',' >> expression)
											   >> "do" >> block >> "end";
		const auto genericForStatement_def = "for" >> namesList >> "in" >> expressionsList >> "do" >> block >> "end";
		const auto functionDeclarationStatement_def = "function" >> functionName >> functionBody;
		const auto localFunctionDeclarationStatement_def = "local" >> lit("function") >> name >> functionBody;
		const auto localAssignmentStatement_def = "local" >> namesList >> -('=' >> expressionsList);

		const auto statement_def = emptyStatement
								   | assignmentStatement
								   | functionCall
								   | labelStatement
								   | gotoStatement
								   | breakStatement
								   | doStatement
								   | whileStatement
								   | repeatStatement
								   | ifThenElseStatement
								   | numericalForStatement
								   | genericForStatement
								   | functionDeclarationStatement
								   | localFunctionDeclarationStatement
								   | localAssignmentStatement;

		const auto returnStatement_def = "return" >> -expressionsList >> -lit(';');

		// Blocks
		const auto block_def = *statement >> -returnStatement;
		const auto chunk_def = block;

		BOOST_SPIRIT_DEFINE(name, namesList,
							openLongBracket, closeLongBacket,
							longLiteralString, literalString,
							numeralInt, numeralFloat,
							shortComment, longComment, comment,
							skipper,
							fieldByExpression, fieldByAssignment, field, fieldsList, tableConstructor,
							parametersList, arguments,
							functionBody, functionCall, functionCallEnd,
							functionDefinition, functionNameMember, functionName,
							bracketedExpression, tableIndexExpression, tableIndexName,
							prefixExpression, postPrefix,
							variable, variableFunctionCall, variablePostfix, variablesList,
							unaryOperation, binaryOperation,
							simpleExpression, expression, expressionsList,
							assignmentStatement, localAssignmentStatement,
							labelStatement, gotoStatement, breakStatement, doStatement,
							whileStatement, repeatStatement,
							ifStatement, elseIfStatement, ifThenElseStatement,
							numericalForStatement, genericForStatement,
							functionDeclarationStatement, localFunctionDeclarationStatement,
							returnStatement, statement,
							block, chunk);
	} // namespace parser

	parser::chunk_type chunkRule()
	{
		return parser::chunk;
	}
} // namespace lac
