#pragma once

#include "ast_evaluator.h"

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

	template <typename Iterator, typename Functor, class Item>
		struct CalcGrammar : qi::grammar<Iterator, ast::Program(), ascii::space_type>
	{
		using func_type = Functor;
		using string_type = std::basic_string<typename std::iterator_traits<Iterator>::value_type>;

		template<class Symbols, class Functors>
		CalcGrammar(Symbols&& symbols, Functors&& fns) : CalcGrammar::base_type(expression),
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

		AstEvaluator<Functor, Item> Parse(string_type const& statement) const{
			return doParse(statement);
		}

		template<template<class, class> class Evaluator>
		Evaluator<Functor, Item> Parse(string_type const& statement) const{
			return doParse(statement);
		}

		private:
			expr::ast::Program doParse(string_type const& statement) const{
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

	template<class Keywords, class Functors, 
		class KeyIterator=typename IteratorFromKeywords<Keywords>::type, 
		class Functor=typename ContainedType<Functors>::type>
	auto MakeGrammar(Keywords const& keywords, Functors const& fnList) 
	-> typename boost::enable_if<arg1_type<Functor>,CalcGrammar<KeyIterator, Functor,typename arg1_type<Functor>::type >>::type{
		return {keywords, fnList};
	}

	template<class Item>
	struct TypeHint{
		template<class Keywords, class Functors, 
			class KeyIterator=typename IteratorFromKeywords<Keywords>::type, 
			class Functor=typename ContainedType<Functors>::type>
		static CalcGrammar<KeyIterator, Functor, Item> MakeGrammar(Keywords const& keywords, Functors const& fnList){
			return {keywords, fnList};
		}

	};

}
