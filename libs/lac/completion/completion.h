#pragma once

#include <lac/parser/ast.h>
#include <lac/parser/positions.h>
#include <lac/analysis/scope.h>
#include <lac/analysis/user_defined.h>

#include <boost/optional.hpp>

namespace lac
{
	namespace comp
	{
		enum class CompletionFilter
		{
			none,
			variables,
			methods
		};

		class CORE_API Completion
		{
		public:
			void setUserDefined(lac::an::UserDefined userDefined);
			lac::an::UserDefined userDefined() const;

			bool updateProgram(std::string_view str, size_t currentPosition = std::string_view::npos);
			an::ElementsMap getVariableCompletionList(std::string_view str, size_t pos = std::string_view::npos);
			an::ElementsMap getArgumentCompletionList(std::string_view str, size_t pos = std::string_view::npos);
			an::TypeInfo getTypeAtPos(std::string_view str, size_t pos);
			std::string getVariableNameAtPos(std::string_view str, size_t pos); // Returns empty string if it is not a name at this position
			std::vector<std::string> getTypeHierarchyAtPos(std::string_view str, size_t pos);

		private:
			boost::optional<lac::an::UserDefined> m_userDefined;
			ast::Block m_rootBlock;
			an::Scope m_rootScope;
			pos::Positions<std::string_view::const_iterator> m_positions;
		};

		// Remove the last member of the variable. If not possible, return empty.
		boost::optional<ast::VariableOrFunction> removeLastPart(ast::VariableOrFunction var);

		// Return the "parent" variable at the current position
		boost::optional<ast::VariableOrFunction> getContext(std::string_view str, size_t pos = std::string_view::npos);

		// Return a list of possibilities for auto-completion
		an::ElementsMap getAutoCompletionList(const an::Scope& rootScope, std::string_view str, size_t pos = std::string_view::npos);
		an::ElementsMap getAutoCompletionList(const an::Scope& localScope, const boost::optional<ast::VariableOrFunction>& var, CompletionFilter filter = CompletionFilter::none);

		// Extend the block in the scope until the following keyword (and recurse over children)
		void extendBlock(const an::Scope& scope, const pos::Elements& elements);
	} // namespace comp
} // namespace lac
