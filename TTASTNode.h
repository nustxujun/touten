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
		virtual void visit(ASTNodeVisitor* visitor) = 0;
		
		void* operator new(size_t size)
		{
			static size_t memcost = 0;
			memcost += size;
			return malloc(size);
		}
	};

}

#endif