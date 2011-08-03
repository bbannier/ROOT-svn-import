#ifndef ROOT_FillPatterns
#define ROOT_FillPatterns

#include <CoreGraphics/CGPattern.h>

namespace ROOT_iOS {
namespace GraphicUtils {

//
//Predefined fill styles (patterns).
//Must be 25, now only 10. To be added.
//

//TODO: remaining patterns are required.
enum {
   kPredefinedFillPatterns = 10
};

typedef CGPatternRef (*PatternGenerator_t)();
//Array of pointers to functions, generating patterns.
extern PatternGenerator_t gPatternGenerators[kPredefinedFillPatterns];

}//namespace GraphicUtils
}//namespace ROOT_iOS

#endif
