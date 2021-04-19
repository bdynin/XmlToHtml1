<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:template match="/">
    <html>
      <body>
        <h2>CD Catalog</h2>
        <table border="1">
          <tr bgcolor="#9acd32">
            <th>Title</th>
            <th>Artist</th>
            <th>Country</th>
            <th>Company</th>
            <th>Price</th>
            <th>Year</th>
           </tr>
          <xsl:for-each select="CATALOG/CD">
            <xsl:sort select="ARTIST"/>
            <tr>
              <td>
                <xsl:if test="TITLE">
                  <xsl:value-of select="TITLE"/>
                </xsl:if>                  
              </td>
              <td>
                <xsl:if test="ARTIST">
                  <xsl:value-of select="ARTIST"/>
                </xsl:if>
              </td>
              <td>
                <xsl:if test="COUNTRY">
                  <xsl:value-of select="COUNTRY"/>
                </xsl:if>
              </td>
              <td>
                <xsl:if test="COMPANY">
                  <xsl:value-of select="COMPANY"/>
                </xsl:if>
              </td>
              <td>
                <xsl:if test="PRICE">
                  <xsl:value-of select="PRICE"/>
                </xsl:if>
              </td>
              <td>
                <xsl:if test="YEAR">
                  <xsl:value-of select="YEAR"/>
                </xsl:if>
              </td>
            </tr>
          </xsl:for-each>
        </table>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>