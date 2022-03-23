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

#include <string>
#include <functional> //mem_fn
//#include "user_score.h"
#include "grammar.h"
#include "item_evaluator.h"

namespace biz{
	struct UserScore{ float like; float follow; float comment;};
}

float like(biz::UserScore const& user){ return user.like; };
float follow(biz::UserScore const& user){ return user.follow; };
float comment(biz::UserScore const& user){ return user.comment; };

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
	auto fnList  = {std::mem_fn(&biz::UserScore::like),
		std::mem_fn(&biz::UserScore::follow),
		std::mem_fn(&biz::UserScore::comment)
	};
	auto&& gram = expr::MakeGrammar(symbols, fnList);

	auto fnList2  = {&like,&follow,&comment};
	auto&& gram2 = expr::MakeGrammar(symbols, fnList2);

	std::string str;
	while (std::getline(std::cin, str))
	{
		if (str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;

		auto user_eval = expr::Parse<biz::UserScore>(str, gram);	
		auto user_eval2 = expr::Parse(str, gram2);	

		biz::UserScore user1 ={1,2,3}, user2={2,3,4};
		for(auto& user : {user1, user2}){
			std::cout << "user score: like->" << user.like << ", follow->" << user.follow << ", comment->" << user.comment << '\n';
			std::cout << "weighted score is : " << user_eval(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval2(user) << '\n' << std::endl;
		}
		std::cout << "-------------------------\n";
	}

	std::cout << "Bye... :-) \n\n";
	return 0;
}

