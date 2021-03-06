#pragma once

#include <lac/parser/ast.h>
#include <lac/helper/algorithm.h>

#include <boost/spirit/home/x3/support/context.hpp>

#include <iostream>
#include <vector>

namespace lac
{
	namespace pos
	{
		namespace details
		{
			namespace x3 = boost::spirit::x3;

			template <typename Context, typename Tag>
			struct HasTag
			{
			};

			template <typename ID, typename T, typename Next, typename Tag>
			struct HasTag<x3::context<ID, T, Next>, Tag> : HasTag<Next, Tag>
			{
			};

			template <typename ID, typename T, typename Next, typename Tag>
			struct HasTag<const x3::context<ID, T, Next>&, Tag> : HasTag<Next, Tag>
			{
			};

			template <typename ID, typename T, typename Next>
			struct HasTag<x3::context<ID, T, Next>, ID> : std::true_type
			{
			};

			template <typename ID, typename T, typename Next>
			struct HasTag<const x3::context<ID, T, Next>&, ID> : std::true_type
			{
			};

			template <typename ID, typename T, typename Tag>
			struct HasTag<x3::context<ID, T, x3::unused_type>, Tag> : std::false_type
			{
			};

			template <typename ID, typename T, typename Tag>
			struct HasTag<const x3::context<ID, T, x3::unused_type>&, Tag> : std::false_type
			{
			};

			template <typename ID, typename T>
			struct HasTag<x3::context<ID, T, x3::unused_type>, ID> : std::true_type
			{
			};

			template <typename ID, typename T>
			struct HasTag<const x3::context<ID, T, x3::unused_type>&, ID> : std::true_type
			{
			};
		} // namespace details

		template <class T>
		inline constexpr bool is_position_annotated = std::is_base_of<ast::PositionAnnotated, T>::value;

		struct CORE_API Element
		{
			Element() = default;
			Element(ast::ElementType type)
				: type(type)
			{
			}

			size_t begin = 0, end = 0;
			ast::ElementType type = ast::ElementType::not_defined;
		};
		using Elements = std::vector<Element>;

		template <typename Iterator>
		class Positions
		{
		public:
			Positions() {}

			Positions(Iterator begin, Iterator end)
				: m_begin(begin)
				, m_end(end)
			{
			}

			template <class AST>
			std::enable_if_t<!is_position_annotated<AST>> annotate(AST& /*ast*/, Iterator /*begin*/, Iterator /*end*/)
			{
				// Do nothing here
			}

			void annotate(ast::PositionAnnotated& ast, Iterator begin, Iterator end)
			{
				ast.begin = begin - m_begin;
				ast.end = end - m_begin - 1;
			}

			template <ast::ElementType E>
			void annotate(ast::ElementAnnotated<E>& ast, Iterator begin, Iterator end)
			{
				ast.begin = begin - m_begin;
				ast.end = end - m_begin;

				Element elt;
				elt.begin = ast.begin;
				elt.end = ast.end;
				elt.type = E;
				addElement(elt);
			}

			void addElement(Element element)
			{
				for (auto& elt : helper::reverse{m_elements})
				{
					if (elt.end < element.begin)
						break; // We can stop the search

					if (elt.begin == element.begin && elt.end == element.end)
					{
						elt = element;
						return;
					}
				}

				m_elements.push_back(element);
			}

			const Elements& elements() const
			{
				return m_elements;
			}

			size_t pos(const Iterator it) const
			{
				return it - m_begin;
			}

		private:
			Iterator m_begin, m_end;
			Elements m_elements;
		};

		template <typename Context, typename Tag>
		inline constexpr bool has_tag = details::HasTag<Context, Tag>::value;

		struct position_tag
		{
		};

		struct annotate_position
		{
			template <typename T, typename Iterator, typename Context>
			inline void on_success(const Iterator& begin, const Iterator& end, T& ast, const Context& context)
			{
				if constexpr (has_tag<Context, position_tag>)
				{
					auto& positions = boost::spirit::x3::get<position_tag>(context).get();
					positions.annotate(ast, begin, end);
				}
			}
		};
	} // namespace pos
} // namespace lac
