#include <lac/analysis/visitor.h>
#include <lac/completion/completion.h>
#include <lac/completion/get_block.h>
#include <lac/completion/type_at_pos.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/parser/chunk.h>
#include <lac/parser/parser.h>

#include <doctest/doctest.h>
#include <cctype>

namespace lac::comp
{
	bool Completion::updateProgram(std::string_view view, size_t currentPosition)
	{
		if (view.empty())
			return false;

		if (currentPosition == std::string_view::npos)
			currentPosition = view.size() - 1;

		auto ret = lac::parser::parseBlock(view);

		if (ret.parsed)
		{
			std::swap(m_rootBlock, ret.block);
			std::swap(m_positions, ret.positions);

			m_rootScope = an::analyseBlock(m_rootBlock);
		}

		// Always update the boundary of the root block
		m_rootBlock.end = view.size();

		return ret.parsed;
	}

	an::ElementsMap Completion::getAutoCompletionList(std::string_view str, size_t pos)
	{
		return comp::getAutoCompletionList(m_rootScope, str, pos);
	}

	boost::optional<ast::Variable> removeLastPart(ast::VariableOrFunction var)
	{
		// TODO: what can we do if the start is a bracketed expression?
		if (var.variable.start.get().type() != typeid(std::string))
			return {};

		// If there is a member function, remove it and return the rest as is
		if (var.member)
			return var.variable;

		// Else we must remove the last part of the variable
		if (var.variable.rest.empty())
			return {}; // There is nothing left

		var.variable.rest.pop_back();
		return var.variable;
	}

	an::ElementsMap getAutoCompletionList(const an::Scope& rootScope, std::string_view str, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = str.size() - 1;
		if (pos >= str.size())
			return rootScope.getElements();

		// Get the scope under the cursor
		auto scope = pos::getScopeAtPos(rootScope, pos);
		if (!scope)
			return rootScope.getElements();

		CompletionFilter filter = CompletionFilter::none;
		if (str[pos] == '.')
			filter = CompletionFilter::variables;
		else if (str[pos] == ':')
			filter = CompletionFilter::functions;

		if (filter != CompletionFilter::none)
		{
			if (!pos)
				return rootScope.getElements();

			--pos;
			// Ignore the whitespace
			while (pos != 0 && std::isspace(str[pos]))
				--pos;

			auto var = parseVariableAtPos(str, pos); // Do not remove the last part, as it does not exist
			if (!var)
				return {}; // Return an empty map here

			return getAutoCompletionList(*scope, var->variable, filter);
		}

		auto var = parseVariableAtPos(str, pos);
		if (!var)
			return scope->getElements(false);

		// Deduct the filter from the syntax used
		filter = var->member
					 ? CompletionFilter::functions
					 : CompletionFilter::variables;
		return getAutoCompletionList(*scope, removeLastPart(*var), filter);
	}

	an::ElementsMap getAutoCompletionList(const an::Scope& localScope, const boost::optional<ast::Variable>& var, CompletionFilter filter)
	{
		if (!var)
			return localScope.getElements(false);

		const auto info = getVariableType(localScope, *var);
		if (info.type == an::Type::nil)
			return localScope.getElements(false);

		if (filter == CompletionFilter::none)
			return getElements(info);

		return getElements(info,
						   filter == CompletionFilter::variables
							   ? an::ElementType::variable
							   : an::ElementType::function);
	}

	boost::optional<ast::Variable> getContext(std::string_view str, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = str.size() - 1;
		if (pos >= str.size())
			return {};
		if (str[pos] == '.' || str[pos] == ':')
		{
			auto var = parseVariableAtPos(str, pos - 1); // Do not remove the last part, as it does not exist
			if (var)
				return var->variable;
			return {};
		}

		auto var = parseVariableAtPos(str, pos);
		if (!var)
			return {};
		return removeLastPart(*var);
	};
} // namespace lac::comp