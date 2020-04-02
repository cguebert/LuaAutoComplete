#include <lac/analysis/type_info.h>

#include <algorithm>

namespace lac::an
{
	VariableInfo::VariableInfo(const VariableInfo& other)
		: VariableInfo(other.name(), other.type())
	{
	}

	VariableInfo::VariableInfo(std::string_view name)
		: VariableInfo(name, Type::unknown)
	{
	}

	VariableInfo::VariableInfo(std::string_view name, const TypeInfo& type)
		: m_name(name)
		, m_type(std::make_unique<TypeInfo>(type))
	{
	}

	VariableInfo& VariableInfo::operator=(const VariableInfo& other)
	{
		m_name = other.name();
		m_type = std::make_unique<TypeInfo>(other.type());
		return *this;
	}

	const std::string& VariableInfo::name() const
	{
		return m_name;
	}

	const TypeInfo& VariableInfo::type() const
	{
		return *m_type;
	}

	/****************************************************************************/

	TypeInfo UserType::getVariableType(const std::string& name) const
	{
		return variables.count(name)
				   ? variables.at(name)
				   : TypeInfo{};
	}

	FunctionInfo UserType::getMethodType(const std::string& name) const
	{
		return methods.count(name)
				   ? methods.at(name)
				   : FunctionInfo{};
	}

	/****************************************************************************/

	TypeInfo::TypeInfo() = default;

	TypeInfo::TypeInfo(Type type)
		: type(type)
	{
	}

	TypeInfo TypeInfo::fromTypeName(std::string_view name)
	{
		TypeInfo type{Type::userdata};
		type.name = name;
		return type;
	}

	TypeInfo TypeInfo::fromName(std::string_view name)
	{
		TypeInfo type{Type::string};
		type.name = name;
		return type;
	}

	TypeInfo TypeInfo::convert(Type destination) const
	{
		if (type == destination)
			return destination;

		if (type == Type::error)
			return Type::error;
		if (type == Type::unknown)
			return destination;

		switch (destination)
		{
		case Type::boolean:
			return Type::boolean;

		case Type::number:
			if (type == Type::number || type == Type::string)
				return Type::number;
			return Type::error;

		case Type::string:
			if (type == Type::string || type == Type::number)
				return Type::string;
			return Type::error;

		default:
			return Type::error;
		}
	}

	TypeInfo::operator bool() const
	{
		return type != Type::error;
	}

	TypeInfo TypeInfo::member(const std::string& name) const
	{
		return members.count(name)
				   ? members.at(name)
				   : TypeInfo{};
	}

	std::string TypeInfo::typeName() const
	{
		if (!name.empty())
			return name;

		switch (type)
		{
		case Type::nil:
			return "nil";
		case Type::boolean:
			return "boolean";
		case Type::number:
			return "number";
		case Type::string:
			return "string";
		case Type::table:
			return "table";
		case Type::function:
			return "function";
		case Type::userdata:
			return "userdata";
		case Type::thread:
			return "thread";
		default:
		case Type::unknown:
			return "unknown";
		case Type::error:
			return "error";
		}
	}

} // namespace lac::an
