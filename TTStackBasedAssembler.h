#ifndef _TTStackBasedAssembler_H_
#define _TTStackBasedAssembler_H_

#include <vector>
#include "TTASTree.h"
#include "TTAssembler.h"
#include "TTScope.h"
#include "TTConstantPool.h"

namespace TT
{
	class StackBasedAssembler: public ASTNodeVisitor
	{
		enum Instruction
		{
			//var
			LOAD,
			LOAD_CONST,
			STORE,
			ADDR,

			//func
			CALL,

			//opt
			ASSGIN,
			ASSGIN_ARRAY,

			RETURN,
			RETURN_VALUE,
			RETURN_ARRAY,


			//binop
			DAND,
			DOR,
			AND,
			OR,
			XOR,
			EQ,
			NE,
			GREAT,
			LESS,
			GE,
			LE,
			ADD,
			SUB,
			MUL,
			DIV,
			MOD,

			//unop
			EXC,
			NS,


			VOID
		};

		typedef __int32 Operand;
	public :
		typedef std::vector<char> Codes;

	public:
		StackBasedAssembler(StackBasedAssembler::Codes& c, 
			ScopeManager& sm, ConstantPool& cp);

		void assemble(ASTNode::Ptr ast);
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
			size_t head = mInstruction;
			char* code = (char*)&c;
			for (int i = 0; i < sizeof(T); ++i, ++mInstruction)
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

		typedef TTMap<Symbol*, std::vector<size_t>> BackFill;
		BackFill mBackFill;

		void addbackfill(Symbol*, size_t instr);
		
	};
}

#endif