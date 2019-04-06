#pragma once

#include <analysis/type_info.h>

#include <string>
#include <string_view>
#include <vector>

namespace lac::an
{
	class Scope
	{
	public:
		Scope(Scope* parent = nullptr);

		void addVariable(std::string_view name, TypeInfo type);
		TypeInfo getVariableType(std::string_view name) const;

		void addChildScope(Scope scope);

	private:
		Scope* m_parent = nullptr;

		struct VariableInfo
		{
			std::string name;
			TypeInfo type;
		};

		std::vector<Scope> m_childs;
		std::vector<VariableInfo> m_variables;
	};
} // namespace lac::an
