#ifndef _TTStackBasedAssembler_H_
#define _TTStackBasedAssembler_H_

#include "TTASTree.h"
#include "TTAssembler.h"
#include "TTScope.h"
#include "TTConstantPool.h"
#include "TTInterpreterCommon.h"
#include "TTStackBasedAssembler.h"

namespace TT
{
	static const Char* GLOBAL_INIT_FUNC = L"__initGlobal";

	class StackBasedAssembler: public ASTNodeVisitor
	{
	public:
		StackBasedAssembler(ScopeManager& sm, ConstantPool& cp);

		void assemble(ASTNode::Ptr ast);
	private:

		void visit(FileNode* node);
		void visit(VarNode* node);
		void visit(AssginNode* node);
		void visit(VarListNode* node);
		void visit(FunctionNode* node);
		void visit(BlockNode* node);
		void visit(ConstNode* node);
		void visit(FuncCallNode* node);
		void visit(OperatorNode* node);
		void visit(LoopNode* node);
		void visit(CondIFNode* node);
		void visit(ReturnNode* node);
		void visit(TT::NameNode * node);

		size_t visit(ASTNodeList::Ptr list);


		size_t addInstruction(Instruction instr);
		size_t addInstruction(Instruction instr, Operand opra);

		template <class T>
		size_t addCode(const T& c)
		{
			return mCurScope->writeCode(&c, sizeof(T));
		}

		template <class T>
		void fill(T value, char* addr)
		{
			*((T*) addr) = value;
		}

		Scope::Ptr enterScope(const Char* name = 0);
		Scope::Ptr leaveScope();

		size_t stringToConstPool(const Char* str, size_t size);

	private:
		ScopeManager& mScopeMgr;
		ConstantPool& mConstPool;

		//current parameters;

		String		mCurName;
		Scope::Ptr	mCurScope;
		Symbol*		mCurSymbol;
		bool		mIsFuncall;
		bool		mIsLeft;

		typedef TTMap<Symbol*, std::vector<size_t>> BackFill;
		BackFill mBackFill;

		void addbackfill(Symbol*, size_t instr);
		
	};
}

#endif