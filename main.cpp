// Spirit v2.5 allows you to suppress automatic generation
// of predefined terminals to speed up complation. With
// BOOST_SPIRIT_NO_PREDEFINED_TERMINALS defined, you are
// responsible in creating instances of the terminals that
// you need (e.g. see qi::uint_type uint_ below).
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#include <string>
#include <vector>
#include <functional> //mem_fn

#include "grammar.h"
#include "raw_evaluator.h"

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

template<class MF>
struct UserOp{
	MF mf;
	UserOp(MF f=nullptr) : mf(f){}

	float operator()(biz::UserScore const& user) const{
		return (user.*mf)();	
	}
};

struct UserOp2{
	std::function<float(biz::UserScore const&)> mf;

	// default constructible required
	UserOp2(){}

	template<class F>
	UserOp2(F f) : mf(f){}

	float operator()(biz::UserScore const& user) const{
		return mf(user);	
	}
};

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

	// case 0: for each Functor<Sig> like (std|boost)::function, result of std::mem_fn 
	using Uop = UserOp<float(biz::UserScore::*)() const>;
	std::vector<Uop> fnList_4  = {Uop{&biz::UserScore::lk},
		Uop{&biz::UserScore::fw},
		Uop{&biz::UserScore::cmt}
	};
	auto&& gram_4= expr::MakeGrammar(symbols, fnList_4);

	std::vector<std::function<float(biz::UserScore const&)>> fnList_3  = {std::mem_fn(&biz::UserScore::like),
		std::mem_fn(&biz::UserScore::follow),
		std::mem_fn(&biz::UserScore::comment)
	};
	auto&& gram_3= expr::MakeGrammar(symbols, fnList_3);

	std::vector<boost::function<float(biz::UserScore const&)>> fnList_2  = {std::mem_fn(&biz::UserScore::like),
		std::mem_fn(&biz::UserScore::follow),
		std::mem_fn(&biz::UserScore::comment)
	};
	auto&& gram_2= expr::MakeGrammar(symbols, fnList_2);

	// for Functor<R, A...> like result of boost::mem_fn
	auto fnList_1  = {boost::mem_fn(&biz::UserScore::like),
		boost::mem_fn(&biz::UserScore::follow),
		boost::mem_fn(&biz::UserScore::comment)
	};
	auto&& gram_1= expr::MakeGrammar(symbols, fnList_1);

	auto fnList0  = {std::mem_fn(&biz::UserScore::like),
		std::mem_fn(&biz::UserScore::follow),
		std::mem_fn(&biz::UserScore::comment)
	};
	auto&& gram0= expr::MakeGrammar(symbols, fnList0);

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

	// case 5: if not any function above or error in inferring biz type, use TypeHint to specify biz type explicitly
	std::vector<UserOp2> fnList5  = {&biz::UserScore::lk,
		&biz::UserScore::fw,
		&biz::UserScore::cmt
	};
	auto&& gram5 = expr::TypeHint<biz::UserScore>::MakeGrammar(symbols, fnList5);
	//auto&& gram5 = expr::MakeGrammar(symbols, fnList5); // disable by enable_if<arg1_type<F>>

	std::string str;
	while (std::getline(std::cin, str)) 
	{
		if (str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;

		//auto user_eval0 = gram0.Parse<biz::UserScore>(str); 
		auto user_eval0 = gram0.Parse(str); 
		auto user_eval1 = gram1.Parse(str);	
		auto user_eval2 = gram2.Parse(str);	
		auto user_eval3 = gram3.Parse(str);	
		auto user_eval4 = gram4.Parse(str);	
		auto user_eval5 = gram5.Parse<expr::RawEvaluator>(str);	

		biz::UserScore user1 ={1,2,3}, user2={2,3,4};
		// NOTE initializer_list<UserScore> won't work for user_eval4 here, b' initializer_list only return const iterator while fnList4 has non-const function
		for(auto& user : std::vector<biz::UserScore>{user1, user2}){
			std::cout << "user score: like->" << user.like << ", follow->" << user.follow << ", comment->" << user.comment << '\n';
			std::cout << "weighted score is : " << user_eval0(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval1(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval2(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval3(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval4(user) << '\n' << std::endl;
			std::cout << "weighted score is : " << user_eval5(user) << '\n' << std::endl;
		}
		std::cout << "-------------------------\n";
	}

	std::cout << "Bye... :-) \n\n";
	return 0;
}

