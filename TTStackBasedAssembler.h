#ifndef _TTStackBasedAssembler_H_
#define _TTStackBasedAssembler_H_

#include "TTASTree.h"
#include "TTAssembler.h"
#include "TTScope.h"
#include "TTConstantPool.h"
#include "TTInterpreterCommon.h"
namespace TT
{
	class StackBasedAssembler: public ASTNodeVisitor
	{


	public:
		StackBasedAssembler(Codes& c, 
			ScopeManager& sm, ConstantPool& cp);

		void assemble(ASTNode::Ptr ast);
		bool hasMain()const ;
		size_t getMain()const;
	private:

		void visit(FileNode* node);
		void visit(VarNode* node);
		void visit(AssginNode* node);
		void visit(VarListNode* node);
		void visit(FunctionNode* node);
		void visit(FieldNode* node);
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
			return addCode(&c, sizeof(T));
		}

		size_t addCode(const void* b, size_t s)
		{
			size_t head = mInstruction;
			const char* code = (const char*)b;
			for (size_t i = 0; i < s; ++i, ++mInstruction)
				mCodes.push_back(*code++);
			return head;			
		}

		template <class T>
		void fill(T value, size_t offset)
		{
			*((T*) &mCodes[offset]) = value;
		}

		Scope::Ptr enterScope(const Char* name = 0);
		Scope::Ptr leaveScope();

		void clearParas();
	private:
		ScopeManager& mScopeMgr;
		Codes& mCodes;
		ConstantPool& mConstPool;

		size_t mInstruction;
		//current parameters;

		String		mCurName;
		Scope::Ptr	mCurScope;
		Symbol*		mCurSymbol;
		size_t		mCurConst;
		bool		mIsFuncall;

		typedef TTMap<Symbol*, std::vector<size_t>> BackFill;
		BackFill mBackFill;

		void addbackfill(Symbol*, size_t instr);
		
		size_t mMain;
		bool mHasMain;
	};
}

#endif