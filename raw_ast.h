#pragma once

#include <boost/any.hpp> // any
#include <boost/variant/recursive_variant.hpp> //variant
#include <boost/fusion/include/adapt_struct.hpp> //BOOST_FUSION_ADAPT_STRUCT

#include <list> // std::list

namespace expr { namespace ast
{
	///////////////////////////////////////////////////////////////////////////
	//  The AST
	///////////////////////////////////////////////////////////////////////////
	struct Nil {};
	struct Signed;
	struct Program;

	using ScoreFn = boost::any;

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
		Operand operand;
	};

	struct Operation
	{
		char sign;
		Operand operand;
	};

	struct Program
	{
		Operand first;
		std::list<Operation> rest;
	};

	template<class Functor, class Item>
	auto EvalFn(Functor fn, Item*  u) 
	->typename std::enable_if<std::is_member_object_pointer<Functor>::value, float>::type { 
		return u->*fn; 
	}
	
	template<class Functor, class Item>
	auto EvalFn(Functor fn, Item*  u) 
	->typename std::enable_if<std::is_member_function_pointer<Functor>::value, float>::type  { 
		return (u->*fn)(); 
	}
	
	template<class Functor, class Item>
	auto EvalFn(Functor fn, Item*  u) 
	->typename std::enable_if<!std::is_member_pointer<Functor>::value, float>::type  { 
		return fn(*u); 
	}
}}

BOOST_FUSION_ADAPT_STRUCT(
		expr::ast::Signed,
		(char, sign)
		(expr::ast::Operand, operand)
		)

BOOST_FUSION_ADAPT_STRUCT(
		expr::ast::Operation,
		(char, sign)
		(expr::ast::Operand, operand)
		)

BOOST_FUSION_ADAPT_STRUCT(
		expr::ast::Program,
		(expr::ast::Operand, first)
		(std::list<expr::ast::Operation>, rest)
		)

