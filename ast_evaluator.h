#pragma once

#include <boost/variant/apply_visitor.hpp> // apply_visitor

#include <type_traits>
#include "raw_ast.h"

namespace expr { namespace ast
{
	// non-variant version	
	template<class Item>
	class Op{
		public:		
			virtual float Eval(Item const&) const = 0;
			virtual ~Op() {}
	};
	
	template<class Item>
	class Value : public Op<Item>{
		public:			
			Value(float v) : val(v) {}

			float Eval(Item const&) const override{
				return val;
			} 

			~Value() override{}

		private:
			float val;
	};

	template<class Item>
	class SignedOp: public Op<Item>{
		public:			
			SignedOp(bool s, Op<Item>* p) : sign(s), op_ptr(p) {}

			float Eval(Item const& u) const override{
				return (sign?1:-1)*(op_ptr->Eval(u)); 
			} 

			~SignedOp() override { delete op_ptr; }

		private:
			bool sign;
			Op<Item>* op_ptr;
	};

	template<class Item>
	class BinaryOp: public Op<Item>{
		public:			
			BinaryOp(char s, Op<Item>* l, Op<Item>* r) : sign(s), lop_ptr(l), rop_ptr(r) {}

			float Eval(Item const& u) const override{
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
			Op<Item>* lop_ptr;
			Op<Item>* rop_ptr;
	};
	
	template<class Functor, class Item>
	class FnOp : public Op<Item>{
		public:
			FnOp(ScoreFn  f) : fn(boost::any_cast<Functor>(f)) {}

			float Eval(Item const& u) const override{
				return (*this)(const_cast<Item*>(&u));
			}

			~FnOp() override {}

		private:
			template<class F=Functor>
			auto operator()(Item*  u) const 
			->typename std::enable_if<std::is_member_object_pointer<F>::value, float>::type { 
				return u->*fn; 
			}
	
			template<class F=Functor>
			auto operator()(Item*  u) const 
			->typename std::enable_if<std::is_member_function_pointer<F>::value, float>::type  { 
				return (u->*fn)(); 
			}
	
			template<class F=Functor>
			auto operator()(Item*  u) const 
			->typename std::enable_if<!std::is_member_pointer<F>::value, float>::type  { 
				return fn(*u); 
			}

		private:
			Functor fn;
	};

	} // end of ns ast

	///////////////////////////////////////////////////////////////////////////
	//  The AST evaluator
	///////////////////////////////////////////////////////////////////////////
	template<class Functor, class Item>
	struct AstTransformer 
	{
		typedef ast::Op<Item>* result_type;

		result_type operator()(ast::Nil) const { BOOST_ASSERT(0); return nullptr; }

		result_type operator()(float n) const { return new ast::Value<Item>(n); }

		result_type operator()(ast::ScoreFn const& fn) const{
			return new ast::FnOp<Functor,Item>(fn); 
		}

		result_type operator()(ast::Operand const& op) const{
			return boost::apply_visitor(*this,op);
		}

		result_type operator()(ast::Signed const& x) const{
			return new ast::SignedOp<Item>(x.sign=='+', (*this)(x.operand));
		}

		result_type operator()(ast::Program const& x) const{
			auto lop_ptr = (*this)(x.first);				
			for(ast::Operation const& oper : x.rest){
				lop_ptr = new ast::BinaryOp<Item>(oper.sign, lop_ptr, (*this)(oper.operand)); 
			}
			return lop_ptr;
		}
	};

	template<class Functor, class Item>
	class AstEvaluator{
		public:
			using element_type = Item; 
			AstEvaluator(ast::Program const& prog) : op_ptr(AstTransformer<Functor, Item>()(prog)){}

			float operator()(element_type const& item) const{
				return op_ptr->Eval(item);
			}

			~AstEvaluator() { delete op_ptr; }

		private:
			typename AstTransformer<Functor, Item>::result_type op_ptr;
			
	};

}
