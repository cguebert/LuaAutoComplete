#pragma once

#include <lac/analysis/type_info.h>

#include <string>
#include <string_view>
#include <vector>

namespace lac::ast
{
	struct Block;
}

namespace lac::an
{
	class UserDefined;

	enum class ElementType
	{
		variable,
		function,
		label
	};

	struct Element
	{
		ElementType elementType = ElementType::variable;
		std::string name;
		TypeInfo typeInfo;
		bool local = true;
	};
	using ElementsMap = std::map<std::string, Element>;

	class Scope
	{
	public:
		Scope() = default;
		Scope(const ast::Block& block, Scope* parent = nullptr);

		// This class cannot be copied
		Scope(const Scope&) = delete;
		Scope& operator=(const Scope&) = delete;

		// But it can be moved
		Scope(Scope&&) noexcept = default;
		Scope& operator=(Scope&&) noexcept = default;

		void addVariable(std::string_view name, TypeInfo type);
		TypeInfo getVariableType(std::string_view name) const;

		TypeInfo& modifyTable(std::string_view name);

		void addLabel(std::string_view name);
		bool hasLabel(std::string_view name) const;

		void addFunction(std::string_view name, TypeInfo type);
		TypeInfo getFunctionType(std::string_view name) const;

		UserType getUserType(std::string_view name) const;

		Scope& getGlobalScope();
		void addChildScope(Scope&& scope);

		void setUserDefined(UserDefined* userDefined);
		const UserDefined* getUserDefined() const;

		const ast::Block* block() const;
		const std::vector<Scope>& children() const;

		ElementsMap getElements(bool localOnly = true) const;

	private:
		const ast::Block* m_block = nullptr;
		Scope* m_parent = nullptr;
		UserDefined* m_userDefined = nullptr;

		struct VariableInfo
		{
			std::string name;
			TypeInfo type;
		};

		struct LabelInfo
		{
			std::string name;
		};

		struct FunctionInfo
		{
			std::string name;
			TypeInfo type;
		};

		std::vector<Scope> m_children;
		std::vector<VariableInfo> m_variables;
		std::vector<LabelInfo> m_labels;
		std::vector<FunctionInfo> m_functions;
	};

	ElementsMap getElements(const TypeInfo& type);
	ElementsMap getElements(const TypeInfo& type, ElementType filter);
} // namespace lac::an
