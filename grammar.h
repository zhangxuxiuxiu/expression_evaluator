#pragma once

#include "item_evaluator.h"

#include <boost/spirit/include/qi.hpp> // qi::xxx 
#include <boost/type_traits.hpp> //  function_traits 

#include <exception> //invalid_argument

namespace expr
{
	// support function pointer, member object/function pointer
	template<class F>
	struct arg1_type : boost::false_type{};
	template<class R, class T, class... U>
	struct arg1_type<R(T,U...)> : boost::true_type{
		using type = typename boost::remove_cv<
					typename boost::remove_reference<T>::type
				>::type;
		
	};
	template<class R, class... A>
	struct arg1_type<R(*)(A...)> : arg1_type<R(A...)>{};
	template<class R, class... A>
	struct arg1_type<R(&)(A...)> : arg1_type<R(A...)>{};
	template<class R, class T>
	struct arg1_type<R T::*> : arg1_type<void(T)>{};
	// for (boost|std)::function<Sig> or std::mem_fn alike
	template<class Sig, template<class> class Functor>
	struct arg1_type<Functor<Sig>> : arg1_type<Sig>{};
	// for boost::mem_fn alike
	template<class R, class... A, template<class...> class Functor>
	struct arg1_type<Functor<R,A...>> : arg1_type<R(A...)>{};

	///////////////////////////////////////////////////////////////////////////////
	//  The calculator grammar
	///////////////////////////////////////////////////////////////////////////////
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	template <typename Iterator, typename FuncType, class ItemType>
		struct CalcGrammar : qi::grammar<Iterator, ast::Program(), ascii::space_type>
	{
		using func_type = FuncType;

		template<class Symbols, class FnRange>
		CalcGrammar(Symbols&& symbols, FnRange&& fns) : CalcGrammar::base_type(expression),
			symbol2fn(symbols, fns)
		{
			qi::float_type float_;
			qi::char_type char_;

			expression =
				term
				>> *(   (char_('+') >> term)
					|   (char_('-') >> term)
				    )
				;

			term =
				factor
				>> *(   (char_('*') >> factor)
					|   (char_('/') >> factor)
				    )
				;

			factor =
				float_
                                |   symbol2fn 
				|   '(' >> expression >> ')'
				|   (char_('-') >> factor)
				|   (char_('+') >> factor)
				;
		}

		template<class StrType>
		ItemEvaluator<FuncType, ItemType> Parse(StrType const& statement) const{
			return doParse(statement);
		}

		private:
			template<class StrType>
			expr::ast::Program doParse(StrType const& statement) const{
				expr::ast::Program program;
				auto iter = statement.begin();
				auto end = statement.end();
				boost::spirit::ascii::space_type space;
				bool r = boost::spirit::qi::phrase_parse(iter, end, *this, space, program);
				if (r && iter == end){
					return program; 
				}
				throw std::invalid_argument("invalid statement:"+statement);
			}

			qi::symbols<char, ast::ScoreFn>  symbol2fn;
			qi::rule<Iterator, ast::Program(), ascii::space_type> expression;
			qi::rule<Iterator, ast::Program(), ascii::space_type> term;
			qi::rule<Iterator, ast::Operand(), ascii::space_type> factor;
	};

	template<class StrType>
	struct StringTraits{};
	template<class CharType>
	struct StringTraits<std::basic_string<CharType>>{
		using char_type = CharType;
	};
	template<class CharType>
	struct StringTraits<CharType*>{
		using char_type = typename std::remove_const<CharType>::type;
	};

	template<class Keywords>
	struct IteratorFromKeywords{
		using value_type = typename Keywords::value_type; // basic_string or const char*
		using string_type = std::basic_string<typename StringTraits<value_type>::char_type>;
		using type = typename string_type::const_iterator;
	};

	template<class Container>
	struct ContainedType{
		using type = typename std::remove_const<
				typename std::remove_reference<
					typename Container::value_type				
				>::type
			>::type;
	};

	template<class Keywords, class FnRange, 
		class KeyIterator=typename IteratorFromKeywords<Keywords>::type, 
		class FuncType=typename ContainedType<FnRange>::type>
	auto MakeGrammar(Keywords const& keywords, FnRange const& fnList) 
	-> typename boost::enable_if<arg1_type<FuncType>,CalcGrammar<KeyIterator, FuncType,typename arg1_type<FuncType>::type >>::type{
		return {keywords, fnList};
	}

	template<class ItemType>
	struct TypeHint{
		template<class Keywords, class FnRange, 
			class KeyIterator=typename IteratorFromKeywords<Keywords>::type, 
			class FuncType=typename ContainedType<FnRange>::type>
		static CalcGrammar<KeyIterator, FuncType, ItemType> MakeGrammar(Keywords const& keywords, FnRange const& fnList){
			return {keywords, fnList};
		}

	};

}
