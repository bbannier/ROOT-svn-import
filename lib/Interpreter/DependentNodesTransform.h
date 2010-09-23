#include "clang/Sema/SemaConsumer.h"

class DependentNodesTransform {
public:
   DependentNodesTransform();
   ~DependentNodesTransform();

   void TransformNodes(clang::Sema* Sema);
};

