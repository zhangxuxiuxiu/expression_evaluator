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
	template <typename Iterator>
		struct CalcGrammer : qi::grammar<Iterator, ast::Program(), ascii::space_type>
	{
		template<class Symbols, class FnRange>
		CalcGrammer(Symbols&& symbols, FnRange&& fns) : CalcGrammer::base_type(expression),
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

	template<class Keywords, class FnRange>
	CalcGrammer<typename IteratorFromKeywords<Keywords>::type> MakeGrammer(Keywords const& keywords, FnRange const& fnList){
		return {keywords, fnList};
	}

}
