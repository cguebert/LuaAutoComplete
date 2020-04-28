#include <lac/analysis/user_defined.h>

namespace lac::an
{
	void UserDefined::addVariable(std::string_view name, TypeInfo type)
	{
		variables[std::string{name}] = type;
	}

	const TypeInfo* UserDefined::getVariable(std::string_view name) const
	{
		const auto it = variables.find(std::string{name});
		if (it != variables.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addScriptInput(std::string_view name, FunctionInfo func)
	{
		TypeInfo info = Type::function;
		info.function = std::move(func);
		scriptEntries[std::string{name}] = std::move(info);
	}

	const TypeInfo* UserDefined::getScriptInput(std::string_view name) const
	{
		const auto it = scriptEntries.find(std::string{name});
		if (it != scriptEntries.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addType(TypeInfo type)
	{
		types[type.name] = std::move(type);
	}

	const TypeInfo* UserDefined::getType(std::string_view name) const
	{
		const auto it = types.find(std::string{name});
		if (it != types.end())
			return &it->second;
		return nullptr;
	}

} // namespace lac::an
