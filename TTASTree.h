#ifndef _TTASTree_H_
#define _TTASTree_H_

#include "ToutenCommon.h"
#include "TTASTNode.h"
#include "TTType.h"

#define ASTNODE_VISIT_FUNC_DEC void visit( ASTNodeVisitor* visitor);

namespace TT
{


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
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNodeList::Ptr defs;
	};

	class VarNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		Char name[MAX_VAR_NAME_LEN];
		VariableType type;		

		ASTNode::Ptr index;//可以是下标可以是map的key
		ASTNode::Ptr member;
	};

	class AssginNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNodeList::Ptr left;
		ASTNode::Ptr right;
	};

	class VarListNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNodeList::Ptr vars;
	};

	class FunctionNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		Char name[MAX_VAR_NAME_LEN];
		ASTNodeList::Ptr paras;
		ASTNode::Ptr body;
	};

	class FieldNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		Char name[MAX_VAR_NAME_LEN];
		ASTNode::Ptr body;
	};

	class BlockNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNodeList::Ptr stats;
	};

	class ConstNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ConstantType type;
		Char value[MAX_VAR_NAME_LEN];
	};

	class FuncCallNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public:
		ASTNode::Ptr var;
		ASTNodeList::Ptr paras;
	};

	class OperatorNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNode::Ptr exp1;
		ASTNode::Ptr exp2;

		OperatorType opt;
	};

	class LoopNode:public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNode::Ptr expr;
		ASTNode::Ptr block;
	};

	class CondIFNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public:
		ASTNode::Ptr expr;
		ASTNode::Ptr block;

		ASTNode::Ptr elseif;
	};

	class ReturnNode: public ASTNode
	{
		ASTNODE_VISIT_FUNC_DEC;
	public :
		ASTNode::Ptr expr;
	};

#define REGISTOR_NODE_VISIT_FUNC(t) virtual void visit(t *) = 0;

	class ASTNodeVisitor
	{
	public :
		REGISTOR_NODE_VISIT_FUNC(FileNode);
		REGISTOR_NODE_VISIT_FUNC(VarNode);
		REGISTOR_NODE_VISIT_FUNC(AssginNode);
		REGISTOR_NODE_VISIT_FUNC(VarListNode);
		REGISTOR_NODE_VISIT_FUNC(FunctionNode);
		REGISTOR_NODE_VISIT_FUNC(FieldNode);
		REGISTOR_NODE_VISIT_FUNC(BlockNode);
		REGISTOR_NODE_VISIT_FUNC(ConstNode);
		REGISTOR_NODE_VISIT_FUNC(FuncCallNode);
		REGISTOR_NODE_VISIT_FUNC(OperatorNode);
		REGISTOR_NODE_VISIT_FUNC(LoopNode);
		REGISTOR_NODE_VISIT_FUNC(CondIFNode);
		REGISTOR_NODE_VISIT_FUNC(ReturnNode);
	};
}

#endif