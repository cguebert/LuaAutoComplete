#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>

namespace lac ::helper
{
	template <class...>
	using my_void_t = void; // void_t is C++17, so not available on Linux yet

	template <class T, class V, class = void>
	struct has_find_for : public std::false_type
	{
	};

	template <class T, class V>
	struct has_find_for<T, V, my_void_t<decltype(std::declval<T>().find(std::declval<V>()))>> : public std::true_type
	{
	};

	// Test if type T has a member function find taking a type V
	template <class T, class V>
	constexpr bool has_find_for_v = has_find_for<T, V>::value;

	template <class Container, class Value>
	auto find(const Container& container, const Value& value)
	{
		return std::find(begin(container), end(container), value);
	}

	template <class Container, class Callback>
	auto find_if(Container& container, Callback&& callback)
	{
		return std::find_if(begin(container), end(container), std::forward<Callback>(callback));
	}

	template <class Container, class Value, std::enable_if_t<has_find_for_v<Container, Value>>* = nullptr>
	bool contains(const Container& container, const Value& value)
	{
		return container.find(value) != container.end();
	}

	template <class Container, class Value, std::enable_if_t<!has_find_for_v<Container, Value>>* = nullptr>
	bool contains(const Container& container, const Value& value)
	{
		return std::find(begin(container), end(container), value) != end(container);
	}

	template <class Container, class Callback>
	bool contains_if(const Container& container, Callback&& callback)
	{
		return find_if(container, std::forward<Callback>(callback)) != end(container);
	}

	template <class Container, class Value>
	auto remove(Container& container, const Value& value)
	{
		return std::remove(begin(container), end(container), value);
	}

	template <class Container, class Callback>
	auto remove_if(Container& container, Callback&& callback)
	{
		return std::remove_if(begin(container), end(container), std::forward<Callback>(callback));
	}

	template <class Container, class Value>
	auto erase_remove(Container& container, const Value& value)
	{
		const auto endIt = end(container);
		const auto remIt = remove(container, value);
		container.erase(remIt, endIt);
	}

	template <class Container, class Callback>
	void erase_remove_if(Container& container, Callback&& callback)
	{
		const auto endIt = end(container);
		const auto remIt = remove_if(container, std::forward<Callback>(callback));
		container.erase(remIt, endIt);
	}

	template <class Container, class Value>
	auto lower_bound(Container& container, const Value& value)
	{
		return std::lower_bound(begin(container), end(container), value);
	}

	template <class Container, class Value, class Callback>
	auto lower_bound(Container& container, const Value& value, Callback&& callback)
	{
		return std::lower_bound(begin(container), end(container), value, std::forward<Callback>(callback));
	}

	template <class Container, class Value>
	auto upper_bound(Container& container, const Value& value)
	{
		return std::upper_bound(begin(container), end(container), value);
	}

	template <class Container, class Value, class Callback>
	auto upper_bound(Container& container, const Value& value, Callback&& callback)
	{
		return std::upper_bound(begin(container), end(container), value, std::forward<Callback>(callback));
	}

	template <class Container>
	void sort(Container& container)
	{
		std::sort(begin(container), end(container));
	}

	template <class Container, class Callback>
	void sort(Container& container, Callback&& callback)
	{
		std::sort(begin(container), end(container), std::forward<Callback>(callback));
	}

	template <class Container, class OutputIt>
	void set_difference(const Container& container1, const Container& container2, OutputIt output)
	{
		std::set_difference(begin(container1), end(container1),
							begin(container2), end(container2),
							output);
	}

	template <class Container, class OutputIt, class Callback>
	void set_difference(const Container& container1, const Container& container2, OutputIt output, Callback&& callback)
	{
		std::set_difference(begin(container1), end(container1),
							begin(container2), end(container2),
							output,
							std::forward<Callback>(callback));
	}

	template <class Container, class OutputIt>
	void set_intersection(const Container& container1, const Container& container2, OutputIt output)
	{
		std::set_intersection(begin(container1), end(container1),
							  begin(container2), end(container2),
							  output);
	}

	template <class Container, class OutputIt, class Callback>
	void set_intersection(const Container& container1, const Container& container2, OutputIt output, Callback&& callback)
	{
		std::set_intersection(begin(container1), end(container1),
							  begin(container2), end(container2),
							  output,
							  std::forward<Callback>(callback));
	}

	template <class Container, class OutputIt>
	void set_union(const Container& container1, const Container& container2, OutputIt output)
	{
		std::set_union(begin(container1), end(container1),
					   begin(container2), end(container2),
					   output);
	}

	template <class Container, class OutputIt, class Callback>
	void set_union(const Container& container1, const Container& container2, OutputIt output, Callback&& callback)
	{
		std::set_union(begin(container1), end(container1),
					   begin(container2), end(container2),
					   output,
					   std::forward<Callback>(callback));
	}

	template <class Container, class Value>
	bool binary_search(const Container& container, const Value& value)
	{
		return std::binary_search(begin(container), end(container), value);
	}

	template <typename T>
	class reverse
	{
	public:
		explicit reverse(T& container)
			: m_container{container}
		{
		}
		auto begin() const { return std::rbegin(m_container); }
		auto end() const { return std::rend(m_container); }

	private:
		T& m_container;
	};
} // namespace lac::helper
