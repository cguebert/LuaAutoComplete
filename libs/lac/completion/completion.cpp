#include <lac/analysis/visitor.h>
#include <lac/completion/completion.h>
#include <lac/completion/get_block.h>
#include <lac/completion/type_at_pos.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/parser/chunk.h>
#include <lac/parser/parser.h>
#include <lac/helper/algorithm.h>

#include <doctest/doctest.h>
#include <cctype>

namespace lac::comp
{
	void Completion::setUserDefined(lac::an::UserDefined userDefined)
	{
		m_userDefined = std::move(userDefined);
	}

	bool Completion::updateProgram(std::string_view view, size_t currentPosition)
	{
		if (view.empty())
			return false;

		if (currentPosition == std::string_view::npos)
			currentPosition = view.size() - 1;

		auto ret = lac::parser::parseBlock(view);
		if (!ret.parsed)
		{
			// Remove the current line and retry
			std::string str{view};
			auto start = str.find_last_of("\t\r\n;", currentPosition);
			auto end = str.find_first_of("\t\r\n;", currentPosition);
			if (start != std::string::npos || end != std::string::npos)
			{
				if (start == std::string::npos)
					start = 0;
				if (end == std::string::npos)
					end = view.size() - 1;
				++start; // Start after the white space
				if (start < end)
					str.replace(start, end - start, end - start, ' '); // Replace the current line with spaces

				// Retry
				ret = lac::parser::parseBlock(str);
			}
		}

		if (ret.parsed)
		{
			std::swap(m_rootBlock, ret.block);
			std::swap(m_positions, ret.positions);

			m_rootScope = an::Scope{m_rootBlock};
			if (m_userDefined)
				m_rootScope.setUserDefined(&m_userDefined.get());
			an::analyseBlock(m_rootScope, m_rootBlock);

			// Extend each block until the following keyword
			extendBlock(m_rootScope, m_positions.elements());
		}

		// Always update the boundary of the root block
		m_rootBlock.end = view.size();

		return ret.parsed;
	}

	an::ElementsMap Completion::getAutoCompletionList(std::string_view str, size_t pos)
	{
		return comp::getAutoCompletionList(m_rootScope, str, pos);
	}

	an::TypeInfo Completion::getTypeAtPos(std::string_view str, size_t pos)
	{
		return comp::getTypeAtPos(m_rootScope, str, pos);
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
		/*	
		if (str[pos] == '.')
			filter = CompletionFilter::variables;
		else */
		if (str[pos] == ':')
			filter = CompletionFilter::functions;

		bool membersOnly = (str[pos] == '.' || str[pos] == ':');
		if (membersOnly)
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
					 : CompletionFilter::none; // Not only variables, but all members
		return getAutoCompletionList(*scope, removeLastPart(*var), filter);
	}

	an::ElementsMap getAutoCompletionList(const an::Scope& localScope, const boost::optional<ast::Variable>& var, CompletionFilter filter)
	{
		if (!var)
			return localScope.getElements(false);

		const auto info = getVariableType(localScope, *var);
		if (info.type == an::Type::nil)
			return {}; // Cannot return members as this is not a known table

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

	void extendBlock(const an::Scope& scope, const pos::Elements& elements)
	{
		auto block = scope.block();
		if (!block)
			return;

		// Filter only the keyword in the list of elements
		pos::Elements keywords;
		std::copy_if(elements.begin(), elements.end(), std::back_inserter(keywords), [](const pos::Element& elt) {
			return elt.type == ast::ElementType::keyword;
		});

		// Find the keyword just before the start of the block
		auto reversed = helper::reverse{elements};
		auto itStart = helper::upper_bound(reversed, block->begin, [](size_t pos, const pos::Element& elt) {
			return pos > elt.end;
		});
		if (itStart != reversed.end())
			block->begin = itStart->end;

		// Find the keyword just after the end of the block
		auto itEnd = helper::upper_bound(keywords, block->end, [](size_t pos, const pos::Element& elt) {
			return pos < elt.begin;
		});
		if (itEnd != keywords.end())
			block->end = itEnd->begin;

		// Process the children blocks
		for (const auto& child : scope.children())
			extendBlock(child, elements);
	}
} // namespace lac::comp
