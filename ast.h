#pragma once

#include <boost/variant/recursive_variant.hpp> //variant
#include <boost/fusion/include/adapt_struct.hpp> //BOOST_FUSION_ADAPT_STRUCT

#include <functional> // std::function
#include <list> // std::list

#include "user_score.h"

namespace expr { namespace ast
{
	///////////////////////////////////////////////////////////////////////////
	//  The AST
	///////////////////////////////////////////////////////////////////////////
	struct Nil {};
	struct Signed;
	struct Program;

	using ScoreFn = std::function<float(biz::UserScore const&)>;

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
		char operator_;
		Operand operand;
	};

	struct Program
	{
		Operand first;
		std::list<Operation> rest;
	};
}}

BOOST_FUSION_ADAPT_STRUCT(
		expr::ast::Signed,
		(char, sign)
		(expr::ast::Operand, operand)
		)

BOOST_FUSION_ADAPT_STRUCT(
		expr::ast::Operation,
		(char, operator_)
		(expr::ast::Operand, operand)
		)

BOOST_FUSION_ADAPT_STRUCT(
		expr::ast::Program,
		(expr::ast::Operand, first)
		(std::list<expr::ast::Operation>, rest)
		)

