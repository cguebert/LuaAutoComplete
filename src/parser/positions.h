#pragma once

#include <parser/ast.h>

#include <boost/spirit/home/x3/support/context.hpp>

#include <iostream>

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

			template <typename ID, typename T, typename Next>
			struct HasTag<x3::context<ID, T, Next>, ID> : std::true_type
			{
			};

			template <typename ID, typename T, typename Tag>
			struct HasTag<x3::context<ID, T, x3::unused_type>, Tag> : std::false_type
			{
			};

			template <typename ID, typename T>
			struct HasTag<x3::context<ID, T, x3::unused_type>, ID> : std::true_type
			{
			};
		}

		template <class T>
		inline constexpr bool is_position_annotated = std::is_base_of<ast::PositionAnnotated, T>::value;

		template<typename Iterator>
		struct Positions
		{
			Positions(Iterator begin, Iterator end)
				: m_begin(begin)
				, m_end(end)
			{
			}

			template <class AST>
			std::enable_if_t<!is_position_annotated<AST>> annotate(AST& ast, Iterator begin, Iterator end)
			{
				// Do nothing here
				size_t b = begin - m_begin, e = end - m_begin;
				std::cout << "parsed non-ast: " << b << " - " << e << std::endl;
				std::cout << typeid(ast).name() << std::endl;
			}

			void annotate(ast::PositionAnnotated& ast, Iterator begin, Iterator end)
			{
				ast.begin = begin - m_begin;
				ast.end = end - m_begin;

				std::cout << "parsed ast: " << ast.begin << " - " << ast.end << std::endl;
			}

		private:
			Iterator m_begin, m_end;
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
