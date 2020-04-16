#pragma once

#include <lac/analysis/type_info.h>

#include <map>
#include <set>
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

		void addVariable(const std::string& name, TypeInfo type);
		TypeInfo getVariableType(const std::string& name) const;

		TypeInfo& modifyTable(const std::string& name);

		void addLabel(const std::string& name);
		bool hasLabel(const std::string& name) const;

		TypeInfo getUserType(std::string_view name) const;

		Scope& getGlobalScope();
		void addChildScope(Scope&& scope);

		void setUserDefined(UserDefined* userDefined);
		const UserDefined* getUserDefined() const;
		TypeInfo resolve(const TypeInfo& type) const; // If the given type is userdata, return the corresponding table, else no change

		const ast::Block* block() const;
		const std::vector<Scope>& children() const;

		ElementsMap getElements(bool localOnly = true) const;

	private:
		const ast::Block* m_block = nullptr;
		Scope* m_parent = nullptr;
		UserDefined* m_userDefined = nullptr;

		std::vector<Scope> m_children;
		std::map<std::string, TypeInfo> m_variables;
		std::set<std::string> m_labels;
	};

	ElementsMap getElements(const TypeInfo& type);
	ElementsMap getElements(const TypeInfo& type, ElementType filter);
} // namespace lac::an
