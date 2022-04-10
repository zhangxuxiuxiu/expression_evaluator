#pragma once

#include <boost/variant/apply_visitor.hpp> // apply_visitor

#include <type_traits>
#include "raw_ast.h"
#include "eval_fn.h"

namespace expr { namespace ast
{
	// non-variant version	
	template<class Evalee>
	class Op{
		public:		
			virtual float Eval(Evalee const&) const = 0;
			virtual ~Op() {}
	};
	
	template<class Evalee>
	class Value : public Op<Evalee>{
		public:			
			Value(float v) : val(v) {}

			float Eval(Evalee const&) const override{
				return val;
			} 

			~Value() override{}

		private:
			float val;
	};

	template<class Evalee>
	class SignedOp: public Op<Evalee>{
		public:			
			SignedOp(bool s, Op<Evalee>* p) : sign(s), op_ptr(p) {}

			float Eval(Evalee const& u) const override{
				return (sign?1:-1)*(op_ptr->Eval(u)); 
			} 

			~SignedOp() override { delete op_ptr; }

		private:
			bool sign;
			Op<Evalee>* op_ptr;
	};

	template<class Evalee>
	class BinaryOp: public Op<Evalee>{
		public:			
			BinaryOp(char s, Op<Evalee>* l, Op<Evalee>* r) : sign(s), lop_ptr(l), rop_ptr(r) {}

			float Eval(Evalee const& u) const override{
				float lhs = lop_ptr->Eval(u); 
				float rhs = rop_ptr->Eval(u); 
				switch (sign)
				{
					case '+': return lhs + rhs;
					case '-': return lhs - rhs;
					case '*': return lhs * rhs;
					case '/': return lhs / rhs;
				}
				return 0;
			} 

			~BinaryOp() override { delete lop_ptr; delete rop_ptr; }

		private:
			char sign;
			Op<Evalee>* lop_ptr;
			Op<Evalee>* rop_ptr;
	};
	
	template<class Functor, class Evalee>
	class FnOp : public Op<Evalee>{
		public:
			FnOp(ScoreFn  f) : fn(boost::any_cast<Functor>(f)) {}

			float Eval(Evalee const& u) const override{
				return EvalFn(fn, const_cast<Evalee*>(&u));
			}

			~FnOp() override {}

		private:
			Functor fn;
	};

	} // end of ns ast

	///////////////////////////////////////////////////////////////////////////
	//  The AST evaluator
	///////////////////////////////////////////////////////////////////////////
	template<class Functor, class Evalee>
	struct AstTransformer 
	{
		typedef ast::Op<Evalee>* result_type;

		result_type operator()(ast::Nil) const { BOOST_ASSERT(0); return nullptr; }

		result_type operator()(float n) const { return new ast::Value<Evalee>(n); }

		result_type operator()(ast::ScoreFn const& fn) const{
			return new ast::FnOp<Functor,Evalee>(fn); 
		}

		result_type operator()(ast::Operand const& op) const{
			return boost::apply_visitor(*this,op);
		}

		result_type operator()(ast::Signed const& x) const{
			return new ast::SignedOp<Evalee>(x.sign=='+', (*this)(x.operand));
		}

		result_type operator()(ast::Program const& x) const{
			auto lop_ptr = (*this)(x.first);				
			for(ast::Operation const& oper : x.rest){
				lop_ptr = new ast::BinaryOp<Evalee>(oper.sign, lop_ptr, (*this)(oper.operand)); 
			}
			return lop_ptr;
		}
	};

	template<class Functor, class Evalee>
	class AstEvaluator{
		public:
			using element_type = Evalee; 
			AstEvaluator(ast::Program const& prog) : op_ptr(AstTransformer<Functor, Evalee>()(prog)){}

			float operator()(element_type const& e) const{
				return op_ptr->Eval(e);
			}

			~AstEvaluator() { delete op_ptr; }

		private:
			typename AstTransformer<Functor, Evalee>::result_type op_ptr;
			
	};

}
