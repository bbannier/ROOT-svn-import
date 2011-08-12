#ifndef ROOT_FillPatterns
#define ROOT_FillPatterns

#include <CoreGraphics/CGPattern.h>

namespace ROOT_iOS {
namespace GraphicUtils {

//
//Predefined fill styles (patterns).
//Must be 25, now only 15. To be added.
//

//TODO: remaining patterns are required.
enum {
   kPredefinedFillPatterns = 18
};

//Pattern generator function type. Parameter of type float *
//is an rgb tuple. Attention! rgb pointer should be valid while
//you are using pattern - it will be passed into pattern drawing callback
//funciton.
typedef CGPatternRef (*PatternGenerator_t)(float *);
//Array of pointers to functions, generating patterns.
extern PatternGenerator_t gPatternGenerators[kPredefinedFillPatterns];

}//namespace GraphicUtils
}//namespace ROOT_iOS

#endif
