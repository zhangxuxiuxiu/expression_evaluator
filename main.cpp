/*=============================================================================
  Copyright (c) 2001-2011 Joel de Guzman

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
  =============================================================================*/
///////////////////////////////////////////////////////////////////////////////
//
//  A Calculator example demonstrating generation of AST. The AST,
//  once created, is traversed, 1) To print its contents and
//  2) To evaluate the result.
//
//  [ JDG April 28, 2008 ]      For BoostCon 2008
//  [ JDG February 18, 2011 ]   Pure attributes. No semantic actions.
//
///////////////////////////////////////////////////////////////////////////////

// Spirit v2.5 allows you to suppress automatic generation
// of predefined terminals to speed up complation. With
// BOOST_SPIRIT_NO_PREDEFINED_TERMINALS defined, you are
// responsible in creating instances of the terminals that
// you need (e.g. see qi::uint_type uint_ below).
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#if defined(_MSC_VER)
# pragma warning(disable: 4345)
#endif

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <string>
#include <functional>

namespace client { namespace ast
{
	///////////////////////////////////////////////////////////////////////////
	//  The AST
	///////////////////////////////////////////////////////////////////////////
	struct Nil {};
	struct Signed;
	struct Program;

	struct UserScore{ float like; float follow; float comment;};

	using ScoreFn = std::function<float(UserScore const&)>;

	typedef boost::variant<
		Nil
		, float 
		, ScoreFn
		, boost::recursive_wrapper<Signed>
		, boost::recursive_wrapper<Program>
		>
		Operand;

	struct Signed
	{
		char sign;
		Operand operand_;
	};

	struct Operation
	{
		char operator_;
		Operand operand_;
	};

	struct Program
	{
		Operand first;
		std::list<Operation> rest;
	};
}}

BOOST_FUSION_ADAPT_STRUCT(
		client::ast::Signed,
		(char, sign)
		(client::ast::Operand, operand_)
		)

BOOST_FUSION_ADAPT_STRUCT(
		client::ast::Operation,
		(char, operator_)
		(client::ast::Operand, operand_)
		)

BOOST_FUSION_ADAPT_STRUCT(
		client::ast::Program,
		(client::ast::Operand, first)
		(std::list<client::ast::Operation>, rest)
		)

namespace client { namespace ast
{
	///////////////////////////////////////////////////////////////////////////
	//  The AST Printer
	///////////////////////////////////////////////////////////////////////////
	struct Printer
	{
		typedef void result_type;

		void operator()(Nil) const {}
		void operator()(float n) const { std::cout << n; }
		void operator()(ScoreFn const& fn ) const { std::cout << "score fn"; }

		void operator()(Operation const& x) const
		{
			boost::apply_visitor(*this, x.operand_);
			switch (x.operator_)
			{
				case '+': std::cout << " add"; break;
				case '-': std::cout << " subt"; break;
				case '*': std::cout << " mult"; break;
				case '/': std::cout << " div"; break;
			}
		}

		void operator()(Signed const& x) const
		{
			boost::apply_visitor(*this, x.operand_);
			switch (x.sign)
			{
				case '-': std::cout << " neg"; break;
				case '+': std::cout << " pos"; break;
			}
		}

		void operator()(Program const& x) const
		{
			boost::apply_visitor(*this, x.first);
			BOOST_FOREACH(Operation const& oper, x.rest)
			{
				std::cout << ' ';
				(*this)(oper);
			}
		}
	};

	///////////////////////////////////////////////////////////////////////////
	//  The AST evaluator
	///////////////////////////////////////////////////////////////////////////
	struct Evaluator
	{
		mutable client::ast::UserScore const* user_ptr=nullptr;

		typedef float result_type;

		result_type operator()(Nil) const { BOOST_ASSERT(0); return 0; }
		result_type operator()(float n) const { return n; }
		result_type operator()(ScoreFn const& fn) const { return fn(*user_ptr); }

		result_type operator()(Operation const& x, float lhs) const
		{
			result_type rhs = boost::apply_visitor(*this, x.operand_);
			switch (x.operator_)
			{
				case '+': return lhs + rhs;
				case '-': return lhs - rhs;
				case '*': return lhs * rhs;
				case '/': return lhs / rhs;
			}
			BOOST_ASSERT(0);
			return 0;
		}

		result_type operator()(Signed const& x) const
		{
			result_type rhs = boost::apply_visitor(*this, x.operand_);
			switch (x.sign)
			{
				case '-': return -rhs;
				case '+': return +rhs;
			}
			BOOST_ASSERT(0);
			return 0;
		}

		result_type operator()(Program const& x) const
		{
			result_type state = boost::apply_visitor(*this, x.first);
			BOOST_FOREACH(Operation const& oper, x.rest)
			{
				state = (*this)(oper, state);
			}
			return state;
		}
	};
}}

namespace client
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
//		using string_type = typename std::conditional<std::is_pointer<value_type>::value,
//			std::basic_string<typename std::remove_const<typename std::pointer_traits<value_type>::element_type>::type>, value_type>::type; 
		using string_type = std::basic_string<typename StringTraits<value_type>::char_type>;
		using type = typename string_type::const_iterator;
	};

	template<class Keywords, class FnRange>
	CalcGrammer<typename IteratorFromKeywords<Keywords>::type> MakeGrammer(Keywords const& keywords, FnRange const& fnList){
		return {keywords, fnList};
	}

	class UserEvaluator{
		public:
			float operator()(ast::UserScore const& user) const{
				eval.user_ptr = &user;
				return eval(program);
			}

		private:
			client::ast::Program program;
			client::ast::Evaluator eval;
			
		public:
			template<class StrType>
			static UserEvaluator  Parse(StrType const& statement, CalcGrammer<typename StrType::const_iterator> const& gram){
				UserEvaluator evaluator;
				auto iter = statement.begin();
				auto end = statement.end();
				boost::spirit::ascii::space_type space;
				bool r = phrase_parse(iter, end, gram, space, evaluator.program);
				if (r && iter == end){
					return evaluator; 
				}
				throw std::invalid_argument("invalid statement:"+statement);
			}
	};

			
}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main()
{
	std::cout << "/////////////////////////////////////////////////////////\n\n";
	std::cout << "Expression parser...\n\n";
	std::cout << "/////////////////////////////////////////////////////////\n\n";
	std::cout << "Type an expression...or [q or Q] to quit\n\n";

	auto symbols = {"like","follow", "comment"};
	auto fnList  = {std::mem_fn(&client::ast::UserScore::like),
		std::mem_fn(&client::ast::UserScore::follow),
		std::mem_fn(&client::ast::UserScore::comment)
	};
	auto&& gram = client::MakeGrammer(symbols, fnList);

	std::string str;
	while (std::getline(std::cin, str))
	{
		if (str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;

		auto user_eval = client::UserEvaluator::Parse(str, gram);	

		client::ast::UserScore user1 ={1,2,3}, user2={2,3,4};
		for(auto& user : {user1, user2}){
			std::cout << "user score: like->" << user.like << ", follow->" << user.follow << ", comment->" << user.comment << '\n';
			std::cout << "weighted score is : " << user_eval(user) << '\n' << std::endl;
		}
		std::cout << "-------------------------\n";
	}

	std::cout << "Bye... :-) \n\n";
	return 0;
}

