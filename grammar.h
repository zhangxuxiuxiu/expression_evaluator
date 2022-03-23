#pragma once

#include "ast.h"
#include <boost/spirit/include/qi.hpp> // qi::xxx 

namespace expr
{

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	///////////////////////////////////////////////////////////////////////////////
	//  The calculator grammar
	///////////////////////////////////////////////////////////////////////////////
	template <typename Iterator, typename FuncType>
		struct CalcGrammar : qi::grammar<Iterator, ast::Program(), ascii::space_type>
	{
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

		qi::symbols<char, ast::ScoreFn>  symbol2fn;
		qi::rule<Iterator, ast::Program(), ascii::space_type> expression;
		qi::rule<Iterator, ast::Program(), ascii::space_type> term;
		qi::rule<Iterator, ast::Operand(), ascii::space_type> factor;
	};

	template<class Grammar>
	struct GrammarTraits{};
	template<class Iterator, class FuncType>
	struct GrammarTraits<CalcGrammar<Iterator,FuncType>>{
		using iterator = Iterator;
		using func_type = FuncType;
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

	template<class Keywords, class FnRange>
	CalcGrammar<typename IteratorFromKeywords<Keywords>::type, typename ContainedType<FnRange>::type> MakeGrammar(Keywords const& keywords, FnRange const& fnList){
		return {keywords, fnList};
	}

}
