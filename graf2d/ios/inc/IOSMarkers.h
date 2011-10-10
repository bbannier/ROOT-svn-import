#ifndef ROOT_IOSMarkers
#define ROOT_IOSMarkers

#include <vector>

#include <CoreGraphics/CGContext.h>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TPoint
#include "TPoint.h"
#endif

namespace ROOT {
namespace iOS {
namespace GraphicUtils {

void DrawPolyMarker(CGContextRef ctx, const std::vector<TPoint> &marker, Size_t markerSize, Style_t markerStyle);
void DrawPolyMarker(CGContextRef ctx, unsigned nPoints, const TPoint *marker, Size_t markerSize, Style_t markerStyle);

}
}
}

#endif
