#pragma once

#include <iostream> //cout

#include <boost/variant/apply_visitor.hpp> // apply_visitor

#include "ast.h"

namespace expr { namespace ast
{

	///////////////////////////////////////////////////////////////////////////
	//  The AST evaluator
	///////////////////////////////////////////////////////////////////////////
	template<class FuncType, class ItemType>
	struct ExprEvaluator
	{
		mutable  ItemType const* item_ptr=nullptr;

		typedef float result_type;

		result_type operator()(Nil) const { BOOST_ASSERT(0); return 0; }
		result_type operator()(float n) const { return n; }
		result_type operator()(ScoreFn const& fn) const { return boost::any_cast<FuncType>(fn)(*item_ptr); }

		result_type operator()(Operation const& x, float lhs) const
		{
			result_type rhs = boost::apply_visitor(*this, x.operand);
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
			result_type rhs = boost::apply_visitor(*this, x.operand);
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
			for(Operation const& oper : x.rest)
			{
				state = (*this)(oper, state);
			}
			return state;
		}
	};
}}
