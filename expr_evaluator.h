#pragma once

#include <iostream> //cout
#include <type_traits> //is_member_xxx

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
		// ItemType can't be qualified with const here in case non-const object or function
		mutable  ItemType * item_ptr=nullptr;

		typedef float result_type;

		result_type operator()(Nil) const { BOOST_ASSERT(0); return 0; }

		result_type operator()(float n) const { return n; }

		template<class F=FuncType>
		auto operator()(ScoreFn const& fn) const
		->typename std::enable_if<std::is_member_object_pointer<F>::value, result_type>::type { 
			return item_ptr->*boost::any_cast<FuncType>(fn); 
		}

		template<class F=FuncType>
		auto operator()(ScoreFn const& fn) const
		->typename std::enable_if<std::is_member_function_pointer<F>::value, result_type>::type  { 
			return (item_ptr->*boost::any_cast<FuncType>(fn))(); 
		}

		template<class F=FuncType>
		auto operator()(ScoreFn const& fn) const
		->typename std::enable_if<!std::is_member_pointer<F>::value, result_type>::type  { 
			return boost::any_cast<FuncType>(fn)(*item_ptr); 
		}

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
