#ifndef _TTASTNode_H_
#define _TTASTNode_H_

#include "SharedPtr.h"
#include <malloc.h>

namespace TT
{
	class ASTNodeVisitor;
	class ASTNode
	{
	public :
		typedef SharedPtr<ASTNode> Ptr;

	public :
		virtual ~ASTNode()
		{
		}
		virtual void visit(ASTNodeVisitor* visitor) = 0;

	};

}

#endif