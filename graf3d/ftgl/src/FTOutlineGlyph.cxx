#include    "FTOutlineGlyph.h"
#include    "FTVectoriser.h"


FTOutlineGlyph::FTOutlineGlyph( FT_GlyphSlot glyph, bool useDisplayList1)
:   FTGlyph( glyph),
    glList(0),
    vectoriser(0)
{
    if( ft_glyph_format_outline != glyph->format)
    {
        err = 0x14; // Invalid_Outline
        return;
    }

    vectoriser = new FTVectoriser(glyph);

    if (vectoriser->ContourCount() < 1 || vectoriser->PointCount() < 3)
    {
        delete vectoriser; vectoriser = 0;
        return;
    }

    if(useDisplayList1)
    {
        glList = glGenLists(1);
        glNewList( glList, GL_COMPILE);
        RenderContour();
        glEndList();
        delete vectoriser; vectoriser = 0;
    }
}


FTOutlineGlyph::~FTOutlineGlyph()
{
    glDeleteLists( glList, 1);
    delete vectoriser;
}


const FTPoint& FTOutlineGlyph::Render( const FTPoint& pen)
{
    glTranslatef( pen.X(), pen.Y(), 0.0f);

    if( glList)
    {
        glCallList( glList);
    }
    else if (vectoriser)
    {
       RenderContour();
    }
    
    return advance;
}

void FTOutlineGlyph::RenderContour()
{
    for( unsigned int c = 0; c < vectoriser->ContourCount(); ++c)
    {
        const FTContour* contour = vectoriser->Contour(c);
        
        glBegin( GL_LINE_LOOP);
            for( unsigned int pointIndex = 0; pointIndex < contour->PointCount(); ++pointIndex)
            {
                FTPoint point = contour->Point(pointIndex);
                glVertex2f( point.X() / 64.0f, point.Y() / 64.0f);
            }
        glEnd();
    }
}
