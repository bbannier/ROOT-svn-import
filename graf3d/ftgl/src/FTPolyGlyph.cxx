#include "FTPolyGlyph.h"
#include "FTVectoriser.h"


FTPolyGlyph::FTPolyGlyph( FT_GlyphSlot glyph, bool useDisplayList1)
:   FTGlyph( glyph),
    glList(0), mesh(0)
{
    if( ft_glyph_format_outline != glyph->format)
    {
        err = 0x14; // Invalid_Outline
        return;
    }

    horizontalTextureScale = glyph->face->size->metrics.x_ppem * 64;
    verticalTextureScale   = glyph->face->size->metrics.y_ppem * 64;        

    FTVectoriser vectoriser( glyph);

    if(( vectoriser.ContourCount() < 1) || ( vectoriser.PointCount() < 3))
    {
        return;
    }
    
    vectoriser.MakeMesh( 1.0);
    if (useDisplayList1)
    {
        glList = glGenLists( 1);
        glNewList( glList, GL_COMPILE);
        mesh = vectoriser.GetMesh();
        RenderMesh();
        glEndList();
        mesh = 0;
    }
    else
    {
       mesh = vectoriser.DisownMesh();
    }
}


FTPolyGlyph::~FTPolyGlyph()
{
    glDeleteLists( glList, 1);
    delete mesh;
}


const FTPoint& FTPolyGlyph::Render( const FTPoint& pen)
{
    glTranslatef(  pen.X(),  pen.Y(), 0.0f);

    if (glList)
    {
        glCallList( glList);    
    }
    else if (mesh)
    {
       RenderMesh();
    }
    
    return advance;
}

void FTPolyGlyph::RenderMesh()
{
    for( unsigned int index = 0; index < mesh->TesselationCount(); ++index)
    {
        const FTTesselation* subMesh = mesh->Tesselation( index);
        unsigned int polyonType = subMesh->PolygonType();

        glBegin( polyonType);
            for( unsigned int pointIndex = 0; pointIndex < subMesh->PointCount(); ++pointIndex)
            {
                FTPoint point = subMesh->Point(pointIndex);
                
                glTexCoord2f( point.X() / horizontalTextureScale,
                              point.Y() / verticalTextureScale);
                
                glVertex3f( point.X() / 64.0f,
                            point.Y() / 64.0f,
                            0.0f);
            }
        glEnd();
    }
}
