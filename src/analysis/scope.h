#pragma once

#include <analysis/type_info.h>

#include <string>
#include <string_view>
#include <vector>

namespace lac::an
{
	class UserDefined;

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

		void addFunction(std::string_view name, TypeInfo type);
		TypeInfo getFunctionType(std::string_view name) const;

		Scope& getGlobalScope();
		void addChildScope(Scope&& scope);

		void setUserDefined(UserDefined* userDefined);

	private:
		UserDefined* getUserDefined() const;

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

		std::vector<Scope> m_childs;
		std::vector<VariableInfo> m_variables;
		std::vector<LabelInfo> m_labels;
		std::vector<FunctionInfo> m_functions;
	};
} // namespace lac::an
