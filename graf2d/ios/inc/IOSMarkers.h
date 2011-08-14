#ifndef ROOT_IOSMarkers
#define ROOT_IOSMarkers

#include <vector>

#include <CoreGraphics/CGContext.h>

#ifndef ROOT_TPoint
#include "TPoint.h"
#endif

namespace ROOT_iOS {
namespace GraphicUtils {
//Draw a polymarker, style is specified by gVirtualX.
void DrawPolyMarker(CGContextRef ctx, const std::vector<TPoint> &marker);

}//namespace GraphicUtils
}//namespace ROOT_iOS

#endif
