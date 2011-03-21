<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:d="http://docbook.org/ns/docbook"
    xmlns:fo="http://www.w3.org/1999/XSL/Format"
    version="1.0">
    <xsl:import href="docbook.xsl"/>
    <!-- Apply XHLTHL extension. -->
    <xsl:import href="highlight.xsl"/>
    <xsl:import href="../oxygen_custom.xsl"/>
    
    <!-- oXygen patch: the template was copied from the graphics.xsl and changed to consider also SVG -->
    <xsl:template match="d:imagedata">
        <xsl:choose>
            <xsl:when test="svg:*" xmlns:svg="http://www.w3.org/2000/svg">
                <fo:instream-foreign-object>
                    <xsl:apply-templates mode="copy-all" select="svg:*"/>
                </fo:instream-foreign-object>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-imports/>
            </xsl:otherwise>
        </xsl:choose>        
    </xsl:template>    
    <!-- end oXygen patch. -->
</xsl:stylesheet>