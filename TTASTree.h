#ifndef _TTASTree_H_
#define _TTASTree_H_

#include <malloc.h>
#include "SharedPtr.h"
#include "TTLexer.h"

#define ASTNODE_VISIT_FUNC void visit( ASTNodeVisitor* visitor) {visitor->visit(this);}

namespace TT
{
	class ASTNode
	{
	public :
		typedef SharedPtr<ASTNode> Ptr;

	public :
		
		void* operator new(size_t size)
		{
			static size_t memcost = 0;
			memcost += size;
			return malloc(size);
		}
	};


	struct ASTNodeList
	{
		typedef SharedPtr<ASTNodeList> Ptr;

		ASTNode::Ptr obj;
		Ptr next;

		Ptr setAndNext(ASTNode::Ptr o)
		{
			ASTNodeList* t = new ASTNodeList();
			obj = o;
			return t;
		}
	};



	class FileNode :public ASTNode
	{
	public :
		ASTNodeList::Ptr defs;
	};

	class VarNode: public ASTNode
	{
	public :
		Char name[MAX_VAR_NAME_LEN];
		TokenType type;		

		ASTNode::Ptr index;//可以是下标可以是map的key
		ASTNode::Ptr member;
	};

	class AssginNode: public ASTNode
	{
	public :
		ASTNodeList::Ptr left;
		ASTNode::Ptr right;
	};

	class VarListNode: public ASTNode
	{
	public :
		ASTNodeList::Ptr vars;
	};

	class FunctionNode: public ASTNode
	{
	public :
		Char name[MAX_VAR_NAME_LEN];
		ASTNodeList::Ptr paras;
		ASTNode::Ptr body;
	};

	class FieldNode: public ASTNode
	{
	public :
		Char name[MAX_VAR_NAME_LEN];
		ASTNode::Ptr body;
	};

	class BlockNode: public ASTNode
	{
	public :
		ASTNodeList::Ptr stats;
	};

	class ConstNode: public ASTNode
	{
	public :
		TokenType type;
		Char value[MAX_VAR_NAME_LEN];
	};

	class FuncCallNode: public ASTNode
	{
	public:
		ASTNode::Ptr var;
		ASTNodeList::Ptr paras;
	};

	class BinopNode: public ASTNode
	{
	public :
		ASTNode::Ptr exp1;
		ASTNode::Ptr exp2;

		static const int OP_MAX_LEN = 4;
		Char opt[OP_MAX_LEN];
	};
}

#endif