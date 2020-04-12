#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

#include <lac/parser/ast.h>
#include <lac/parser/ast_adapted.h>
#include <lac/parser/chunk.h>
#include <lac/parser/positions.h>

/* From Lua 5.3 reference, 9 - The Complete Syntax of Lua:
	chunk ::= block
	block ::= {stat} [retstat]
	stat ::=  ';' | 
		varlist '=' explist | 
		functioncall | 
		label | 
		break | 
		goto Name | 
		do block end | 
		while exp do block end | 
		repeat block until exp | 
		if exp then block {elseif exp then block} [else block] end | 
		for Name '=' exp ',' exp [',' exp] do block end | 
		for namelist in explist do block end | 
		function funcname funcbody | 
		local function Name funcbody | 
		local namelist ['=' explist] 
	retstat ::= return [explist] [';']
	label ::= '::' Name '::'
	funcname ::= Name {'.' Name} [':' Name]
	varlist ::= var {',' var}
	var ::=  Name | prefixexp '[' exp ']' | prefixexp '.' Name 
	namelist ::= Name {',' Name}
	explist ::= exp {',' exp}
	exp ::=  nil | false | true | Numeral | LiteralString | '...' | functiondef | 
		prefixexp | tableconstructor | exp binop exp | unop exp 
	prefixexp ::= var | functioncall | '(' exp ')'
	functioncall ::=  prefixexp args | prefixexp ':' Name args 
	args ::=  '(' [explist] ')' | tableconstructor | LiteralString 
	functiondef ::= function funcbody
	funcbody ::= '(' [parlist] ')' block end
	parlist ::= namelist [',' '...'] | '...'
	tableconstructor ::= '{' [fieldlist] '}'
	fieldlist ::= field {fieldsep field} [fieldsep]
	field ::= '[' exp ']' '=' exp | Name '=' exp | exp
	fieldsep ::= ',' | ';'
	binop ::=  '+' | '-' | '*' | '/' | '//' | '^' | '%' | 
		'&' | '~' | '|' | '>>' | '<<' | '..' | 
		'<' | '<=' | '>' | '>=' | '==' | '~=' | 
		and | or
	unop ::= '-' | not | '#' | '~'
*/

namespace lac::parser
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
					("and", Op::land)
					("or", Op::lor);
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
					("not", Op::lnot);
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

#define RULE(name, type)                 \
	struct name : pos::annotate_position \
	{                                    \
	};                                   \
	const x3::rule<struct name, type> name = #name;

	RULE(name, std::string);
	RULE(namesList, std::vector<std::string>);

	const x3::rule<class openLongBracket> openLongBracket = "openLongBracket";
	const x3::rule<class closeLongBacket> closeLongBacket = "closeLongBacket";
	RULE(longLiteralString, std::string);
	RULE(literalStringValue, std::string);
	RULE(literalString, ast::LiteralString);

	RULE(numeralInt, int);
	RULE(numeralFloat, double);
	RULE(numeral, ast::Numeral);

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
	RULE(functionCallPostfix, ast::FunctionCallPostfix);
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

	RULE(variableOrFunction, ast::VariableOrFunction);

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

#undef RULE

	// To annotate rules with no struct attributes
	struct element_tag
	{
	};
	auto startElement = [](auto& ctx) {
		if constexpr (pos::has_tag<decltype(ctx), pos::position_tag>)
		{
			auto& positions = x3::get<pos::position_tag>(ctx).get();
			auto& elt = get<element_tag>(ctx);
			elt.begin = positions.pos(_where(ctx).begin());
		}
	};
	auto endElement = [](auto& ctx) {
		if constexpr (pos::has_tag<decltype(ctx), pos::position_tag>)
		{
			auto& positions = x3::get<pos::position_tag>(ctx).get();
			auto& elt = get<element_tag>(ctx);
			elt.end = positions.pos(_where(ctx).begin());
			positions.addElement(elt);
		}
	};

	const x3::rule<class elementStart> elementStart = "elementStart";
	const x3::rule<class elementEnd> elementEnd = "elementEnd";
	const auto elementStart_def = x3::eps[startElement];
	const auto elementEnd_def = x3::eps[endElement];

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
	const auto literalStringValue_def = lexeme[quotedString('\'')
											   | quotedString('"')
											   | longLiteralString];
	const auto literalString_def = literalStringValue;

	// Numerals
	const auto numeralInt_def = (lit("0x") >> hex)
								| (lit("0X") >> hex)
								| (int_ >> !lit('.'));
	const auto numeralFloat_def = double_;
	const auto numeral_def = numeralInt | numeralFloat;

	// Comments
	const auto shortComment_def = "--" >> lexeme[*(char_ - eol)] >> -eol;
	const auto longComment_def = with<long_bracket_tag>(std::string())
		["--"
		 >> omit[openLongBracket]
		 >> lexeme[*(char_ - closeLongBacket)]
		 >> omit[closeLongBacket]];
	const auto comment_def = longComment | shortComment;

	// Keywords
	auto kwd = [](const char* str) {
		return with<element_tag>(pos::Element{ast::ElementType::keyword})
			[omit[elementStart]
			 >> omit[x3::string(str)]
			 >> x3::no_skip[omit[elementEnd]]];
	};

	// A skipper that ignore whitespace and comments
	const x3::rule<struct skipper> skipper = "skipper";
	const auto skipper_def = ascii::space
							 | with<element_tag>(pos::Element{ast::ElementType::comment})
								 [x3::no_skip[omit[elementStart >> comment >> elementEnd]]];

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

	const auto functionBody_def = '(' >> -parametersList >> ')' >> block >> kwd("end");

	const auto functionDefinition_def = kwd("function") >> functionBody;

	const auto functionCallPostfix_def = -(tableIndexExpression
										   | tableIndexName)
										 >> functionCallEnd;

	const auto functionCall_def = (bracketedExpression
								   | name)
								  >> +functionCallPostfix;

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

	const auto variableOrFunction_def = ((variable >> !functionCallEnd) // Ensure there is no function call after the variable
										 | functionCall)
										>> -functionNameMember;

	// Expressions
	const auto simpleExpression_def = expressionConstant
									  | unaryOperation
									  | numeral
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
	const auto gotoStatement_def = kwd("goto") >> name;
	const auto breakStatement_def = kwd("break") >> x3::attr(ast::BreakStatement{});
	const auto doStatement_def = kwd("do") >> block >> kwd("end");
	const auto whileStatement_def = kwd("while") >> expression >> kwd("do") >> block >> kwd("end");
	const auto repeatStatement_def = kwd("repeat") >> block >> kwd("until") >> expression;
	const auto ifStatement_def = kwd("if") >> expression >> kwd("then") >> block;
	const auto elseIfStatement_def = kwd("elseif") >> expression >> kwd("then") >> block;
	const auto ifThenElseStatement_def = ifStatement
										 >> *(elseIfStatement)
										 >> -(kwd("else") >> block)
										 >> kwd("end");
	const auto numericalForStatement_def = kwd("for") >> name >> '=' >> expression
										   >> ',' >> expression >> -(',' >> expression)
										   >> kwd("do") >> block >> kwd("end");
	const auto genericForStatement_def = kwd("for") >> namesList >> kwd("in") >> expressionsList >> kwd("do") >> block >> kwd("end");
	const auto functionDeclarationStatement_def = kwd("function") >> functionName >> functionBody;
	const auto localFunctionDeclarationStatement_def = kwd("local") >> kwd("function") >> name >> functionBody;
	const auto localAssignmentStatement_def = kwd("local") >> namesList >> -('=' >> expressionsList);

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

	const auto returnStatement_def = kwd("return") >> -expressionsList >> -lit(';');

	// Blocks
	const auto block_def = *statement >> -returnStatement;
	const auto chunk_def = block;

	BOOST_SPIRIT_DEFINE(name, namesList,
						openLongBracket, closeLongBacket,
						longLiteralString, literalStringValue, literalString,
						numeralInt, numeralFloat, numeral,
						shortComment, longComment, comment,
						elementStart, elementEnd,
						skipper,
						fieldByExpression, fieldByAssignment, field, fieldsList, tableConstructor,
						parametersList, arguments,
						functionBody, functionCallPostfix, functionCall, functionCallEnd,
						functionDefinition, functionNameMember, functionName,
						bracketedExpression, tableIndexExpression, tableIndexName,
						prefixExpression, postPrefix,
						variable, variableFunctionCall, variablePostfix, variablesList,
						variableOrFunction,
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

	skipper_type skipperRule()
	{
		return skipper;
	}

	chunk_type chunkRule()
	{
		return chunk;
	}

	variable_or_function_type variableOrFunctionRule()
	{
		return variableOrFunction;
	}
} // namespace lac::parser
