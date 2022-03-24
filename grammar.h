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

	///////////////////////////////////////////////////////////////////////////////
	//  The calculator grammar
	///////////////////////////////////////////////////////////////////////////////
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	template <typename Iterator, typename FuncType>
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


		// specify ItemType
		template<class ItemType, class StrType>
		ItemEvaluator<FuncType, ItemType> Parse(StrType const& statement) const
		{
			expr::ast::Program program;
			auto iter = statement.begin();
			auto end = statement.end();
			boost::spirit::ascii::space_type space;
			bool r = boost::spirit::qi::phrase_parse(iter, end, *this, space, program);
			if (r && iter == end){
				return {program}; 
			}
			throw std::invalid_argument("invalid statement:"+statement);
		}

		// infer ItemType
		template<class StrType, class F=FuncType>
		ItemEvaluator<F, typename arg1_type<F>::type> Parse(StrType const& statement) const {
			static_assert(arg1_type<F>::value, "specify biz type for non function/member pointer like Parse<BizType>(statement)");
			return Parse<typename arg1_type<F>::type>(statement);
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

	template<class Keywords, class FnRange, class KeyIterator=typename IteratorFromKeywords<Keywords>::type, 
		class FuncType=typename ContainedType<FnRange>::type>
	CalcGrammar<KeyIterator, FuncType> MakeGrammar(Keywords const& keywords, FnRange const& fnList){
		return {keywords, fnList};
	}

	// infer ItemType
	template<class StrType, class FuncType, class ItemType=typename arg1_type<FuncType>::type>
	ItemEvaluator<FuncType, ItemType>  Parse(StrType const& statement, CalcGrammar<typename StrType::const_iterator, FuncType> const& gram){
		static_assert(arg1_type<FuncType>::value, "specify biz type for non function/member pointer like Parse<BizType>(statement)");
		return gram.Parse(statement);
	}

	// specify ItemType
	template<class ItemType, class StrType, class Grammar>
	auto Parse(StrType const& statement, Grammar const& gram)
	//-> decltype(Parse<StrType, typename Grammar::func_type, ItemType>(statement, gram)){
	-> decltype(gram. template Parse<ItemType>(statement)){
		return gram. template Parse<ItemType>(statement);
		//return Parse<StrType, typename Grammar::func_type, ItemType>(statement, gram);
	}
}
