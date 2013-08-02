#include "TTASTree.h"

using namespace TT;

#define ASTNODE_VISIT_FUNC_DEF(t) void t##::visit(ASTNodeVisitor* visitor) {visitor->visit(this);}

ASTNODE_VISIT_FUNC_DEF(FileNode);
ASTNODE_VISIT_FUNC_DEF(VarNode);
ASTNODE_VISIT_FUNC_DEF(AssginNode);
ASTNODE_VISIT_FUNC_DEF(VarListNode);
ASTNODE_VISIT_FUNC_DEF(FunctionNode);
ASTNODE_VISIT_FUNC_DEF(FieldNode);
ASTNODE_VISIT_FUNC_DEF(BlockNode);
ASTNODE_VISIT_FUNC_DEF(ConstNode);
ASTNODE_VISIT_FUNC_DEF(FuncCallNode);
ASTNODE_VISIT_FUNC_DEF(OperatorNode);
ASTNODE_VISIT_FUNC_DEF(LoopNode);
ASTNODE_VISIT_FUNC_DEF(CondIFNode);