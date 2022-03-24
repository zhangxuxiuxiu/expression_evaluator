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
#include <vector>
#include <functional> //mem_fn

#include "grammar2.h"

namespace biz{
	struct UserScore{ float like; float follow; float comment;
		float lk() const{ return like;}
		float fw() const{ return follow;}
		float cmt() const{ return comment;}
		float lk2() { return like;}
		float fw2() { return follow;}
		float cmt2() { return comment;}
	};
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
	// case 0: any callable object
	auto fnList0  = {std::mem_fn(&biz::UserScore::like),
		std::mem_fn(&biz::UserScore::follow),
		std::mem_fn(&biz::UserScore::comment)
	};
	// if not function or member pointer, specify biz type
	auto&& gram0= expr::MakeGrammar(symbols, fnList0, boost::mpl::identity<biz::UserScore>{});

	// case 1: member object pointer
	auto fnList1  = {&biz::UserScore::like,
		&biz::UserScore::follow,
		&biz::UserScore::comment
	};
	auto&& gram1 = expr::MakeGrammar(symbols, fnList1);

	// case 2:  function pointer
	auto fnList2  = {&like,&follow,&comment};
	auto&& gram2 = expr::MakeGrammar(symbols, fnList2);

	// case 3:  const member function pointer
	auto fnList3  = {&biz::UserScore::lk,
		&biz::UserScore::fw,
		&biz::UserScore::cmt
	};
	auto&& gram3 = expr::MakeGrammar(symbols, fnList3);

	// case 4:  non-const member function pointer
	auto fnList4  = {&biz::UserScore::lk2,
		&biz::UserScore::fw2,
		&biz::UserScore::cmt2
	};
	auto&& gram4 = expr::MakeGrammar(symbols, fnList4);

	std::string str;
	while (std::getline(std::cin, str))
	{
		if (str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;

		auto user_eval0 = gram0.Parse(str); 
		auto user_eval1 = gram1.Parse(str);	
		auto user_eval2 = gram2.Parse(str);	
		auto user_eval3 = gram3.Parse(str);	
		auto user_eval4 = gram4.Parse(str);	

		biz::UserScore user1 ={1,2,3}, user2={2,3,4};
		// NOTE initializer_list<UserScore> won't work for user_eval4 here, b' initializer_list only return const iterator while fnList4 has non-const function
		for(auto& user : std::vector<biz::UserScore>{user1, user2}){
			std::cout << "user score: like->" << user.like << ", follow->" << user.follow << ", comment->" << user.comment << '\n';
			std::cout << "weighted score is : " << user_eval0(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval1(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval2(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval3(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval4(user) << '\n' << std::endl;
		}
		std::cout << "-------------------------\n";
	}

	std::cout << "Bye... :-) \n\n";
	return 0;
}

