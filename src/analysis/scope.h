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

		// This class cannot be copied
		Scope(const Scope&) = delete;
		Scope& operator=(const Scope&) = delete;

		// But it can be moved
		Scope(Scope&&) noexcept = default;
		Scope& operator=(Scope&&) noexcept = default;

		void addVariable(std::string_view name, TypeInfo type);
		TypeInfo getVariableType(std::string_view name) const;

		void addLabel(std::string_view name);
		bool hasLabel(std::string_view name) const;

		Scope& getGlobalScope();
		void addChildScope(Scope&& scope);

	private:
		Scope* m_parent = nullptr;

		struct VariableInfo
		{
			std::string name;
			TypeInfo type;
		};

		struct LabelInfo
		{
			std::string name;
		};

		std::vector<Scope> m_childs;
		std::vector<VariableInfo> m_variables;
		std::vector<LabelInfo> m_labels;
	};
} // namespace lac::an
