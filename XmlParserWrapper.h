// Contains OS-independent declaration of XML parser class that converts an XML file
// into HTML format.
#ifndef OT_PARSERWRAPPER_H__
#define OT_PARSERWRAPPER_H__

#include <string>
#include <memory>

namespace OTInterviewExercise1
{
    class CXmlParserWrapper
    {
    public:
        // Type of XSLT style sheet (used by XML->HTML transformation)
        // Depending on implementation XSLT style sheet might or
        // might not be used.
        enum class EMXSLTFile
        {
            None, // No XSLT style sheet used
            CatalogResources, // Our XSLT style sheet is contained inside our EXE (as resource)
            File // Our XSLT style sheet is contained in a separate file
        };
        // Methods
        CXmlParserWrapper(EMXSLTFile xsltFileId, const wchar_t *sXSLTFilePathName = nullptr);

        ~CXmlParserWrapper();

        bool Parse(const std::wstring& sXML, std::wstring& o_sHTML, std::wstring& o_sError) noexcept;
    private:
        class CXmlParserWrapperImpl;
        std::unique_ptr<CXmlParserWrapperImpl> mImpl;
        std::wstring mError;
    };
}
#endif