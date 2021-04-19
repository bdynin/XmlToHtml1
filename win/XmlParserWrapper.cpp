// Contains OS-specific (Windows) implementation of XML parser class that converts
// an XML file into HTML format.

#include <windows.h>
#include "resource.h"
#include "..\XmlParserWrapper.h"
#include "..\Util.h"
#include "WinUtil.h"
#import <msxml6.dll>
#include <sstream>

namespace OTInterviewExercise1
{
    // Uses MSXML6.DLL XSLT engine to generate HTML from XML (by using appropriate
    // XSLT files).
    class CXmlParserWrapper::CXmlParserWrapperImpl
    {
    public:
        // Methods
        CXmlParserWrapperImpl(CXmlParserWrapper::EMXSLTFile xsltFileId, const wchar_t* sXSLTFilePathName);
        ~CXmlParserWrapperImpl() = default;

        void Parse(const std::wstring& sXML, std::wstring& o_sHTML);
    private:
        void GetItemsXSLTObj(MSXML2::IXMLDOMNode*& xslItemsPage);
        // Retrieve XSLT stylesheet from file. Not implemented currently
        void ReadXSLTFile(const wchar_t* strFileFullPath);
        // Retrieve XSLT stylesheet from resources
        void ReadXSLTFromResources(DWORD resId);

        // Data
        MSXML2::IXMLDOMDocumentPtr mXslItemsObj;
        std::wstring msXslt;
    };

    CXmlParserWrapper::CXmlParserWrapper(EMXSLTFile xsltFileId, const wchar_t* sXSLTFilePathName)
    {
        std::string functionName;
        unsigned int lineNo = 0;
        try
        {
            if (xsltFileId == CXmlParserWrapper::EMXSLTFile::None || 
                (xsltFileId == CXmlParserWrapper::EMXSLTFile::CatalogResources && sXSLTFilePathName != nullptr) ||
                (xsltFileId == CXmlParserWrapper::EMXSLTFile::File && sXSLTFilePathName == nullptr))
            {
                THROW_ERROR(L"Invalid combination of command-line parameters");
            }
            // Ctor of CXmlParserWrapperImpl class will read XSLT stylesheet from resources
            mImpl = std::make_unique<CXmlParserWrapper::CXmlParserWrapperImpl>(xsltFileId, sXSLTFilePathName);
            return;
        }
        catch (const CException& ex)
        {
            functionName = ex.mFunctionName;
            lineNo = ex.mLineNo;
            std::wostringstream ss;
            ss << L"Exception caught. ";
            if (!ex.mErrorDescription.empty())
            {
                ss << L"System error: " << ex.mErrorDescription;
            }
            mError = ss.str();
        }
        catch (const std::bad_alloc& /*ex*/)
        {
            functionName = __FUNCTION__;
            lineNo = __LINE__;
            mError = L"Memory allocation error.";
        }
        catch (const std::exception& ex)
        {
            std::wostringstream ss;
            ss << L"C++ exception caught. ";
            if (ex.what())
            {
                std::vector<wchar_t> what(strlen(ex.what()) + 1, 0);
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, &what[0], what.size(), ex.what(), strlen(ex.what()));
                assert(numConverted);
                ss.write(static_cast<wchar_t*>(what.data()), wcslen(what.data()));
            }
            functionName = __FUNCTION__;
            lineNo = __LINE__;
            mError = ss.str();
        }
        catch (...)
        {
            functionName = __FUNCTION__;
            lineNo = __LINE__;
            mError = L"Unknown exception caught.";
        }
        LogError(functionName.c_str(), lineNo, mError);;
    }

    CXmlParserWrapper::~CXmlParserWrapper()
    {}

    bool CXmlParserWrapper::Parse(const std::wstring& sXML, std::wstring& o_sHTML, std::wstring& o_sError) noexcept
    {
        std::string functionName;
        unsigned int lineNo = 0;
        try
        {
            o_sError.clear();
            // Object wasn't initialized properly - so copy init error descr into o_sError and return false
            if (mImpl == nullptr)
            {
                o_sError = mError;
                return false;
            }
            mImpl->Parse(sXML, o_sHTML);
            return true;
        }
        catch (const CException& ex)
        {
            std::wostringstream ss;
            ss << L"Exception caught. ";
            if (!ex.mErrorDescription.empty())
            {
                ss << L"System error: " << ex.mErrorDescription;
            }
            o_sError = ss.str();
            functionName = ex.mFunctionName;
            lineNo = ex.mLineNo;
        }
        catch (const std::bad_alloc& /*ex*/)
        {
            o_sError = L"Memory allocation error.";
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        catch (const std::exception& ex)
        {
            std::wostringstream ss;
            ss << L"C++ exception caught. ";
            if (ex.what())
            {
                std::vector<wchar_t> what(strlen(ex.what()) + 1, 0);
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, &what[0], what.size(), ex.what(), strlen(ex.what()));
                assert(numConverted);
                ss.write(static_cast<wchar_t*>(what.data()), wcslen(what.data()));
            }
            o_sError = ss.str();
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        catch (...)
        {
            o_sError = L"Unknown exception caught.";
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        LogError(functionName.c_str(), lineNo, o_sError);;
        
        return false;
    }

    CXmlParserWrapper::CXmlParserWrapperImpl::CXmlParserWrapperImpl(
        CXmlParserWrapper::EMXSLTFile xsltFileId,
        const wchar_t* sXSLTFilePathName)
    {
        assert(xsltFileId == CXmlParserWrapper::EMXSLTFile::CatalogResources);
        if (xsltFileId == CXmlParserWrapper::EMXSLTFile::CatalogResources)
        {
            ReadXSLTFromResources(IDR_RCDATA_CAT_XSLT);
        }
        if (xsltFileId == CXmlParserWrapper::EMXSLTFile::File)
        {
            ReadXSLTFile(sXSLTFilePathName);
        }
    }

    void CXmlParserWrapper::CXmlParserWrapperImpl::Parse(
        const std::wstring& sXML, std::wstring& o_sHTML)
    {
        o_sHTML.clear();

        MSXML2::IXMLDOMNode* xSLTInterface = nullptr;

        bool bGotXSLPage = false;

        bstr_t sXMLBstr(sXML.c_str());
        if (!sXMLBstr)
        {
            THROW_ERROR(L"Memory allocation error");
        }
        GetItemsXSLTObj(xSLTInterface);
        MSXML2::IXMLDOMDocumentPtr xmlObj;
        HRESULT hr = xmlObj.CreateInstance(__uuidof(MSXML2::DOMDocument60));
        assert(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            std::wostringstream ss;
            ss << L"MSXML2::DOMDocument60::CreateInstance failed. Error code: " << std::hex << hr;
            THROW_ERROR(ss.str().c_str());
        }
        VARIANT_BOOL vLoadStatus = xmlObj->loadXML(sXMLBstr);
        if (VARIANT_TRUE != vLoadStatus)
        {
            THROW_ERROR(L"MSXML2::DOMDocument60::loadXML failed");
        }
        bstr_t sHTMLBstr = xmlObj->transformNode(xSLTInterface);
        if (!sHTMLBstr)
        {
            THROW_ERROR(L"MSXML2::DOMDocument60::transformNode failed");
        }
        o_sHTML = sHTMLBstr.GetBSTR();
    }

    void CXmlParserWrapper::CXmlParserWrapperImpl::ReadXSLTFile(const wchar_t* strFileFullPath)
    {
        assert(false);
        THROW_ERROR(L"This functionality isn't implemented");
    }

    void CXmlParserWrapper::CXmlParserWrapperImpl::ReadXSLTFromResources(DWORD resId)
    {
        assert(resId != 0);

        msXslt.clear();

        HMODULE hModule = ::GetModuleHandle(nullptr);
        HRSRC hRes = ::FindResource(hModule, MAKEINTRESOURCE(resId), RT_RCDATA);
        if (!hRes)
        {
            DWORD lastErr = ::GetLastError();
            std::wostringstream ss;
            ss << L"FindResource failed. Error code: " << std::hex << lastErr;
            THROW_ERROR(ss.str().c_str());
        }
        HGLOBAL hResourceLoaded = LoadResource(hModule, hRes);
        if (!hResourceLoaded)
        {
            DWORD lastErr = ::GetLastError();
            std::wostringstream ss;
            ss << L"LoadResource failed. Error code: " << std::hex << lastErr;
            THROW_ERROR(ss.str().c_str());
        }
        char* lpResLock = static_cast<char*>(LockResource(hResourceLoaded));
        DWORD dwSizeRes = SizeofResource(hModule, hRes);
        if (!lpResLock || !dwSizeRes)
        {
            THROW_ERROR(L"Resource is null or 0 size.");
        }
        size_t bufSize = dwSizeRes;
        std::vector<wchar_t> buf(bufSize + 1, 0);
        size_t numConverted = 0;

        mbstowcs_s(&numConverted, buf.data(), buf.size(),
            lpResLock, dwSizeRes);
        assert(numConverted);

        msXslt.assign(buf.data(), wcslen(buf.data()));
    }

    void CXmlParserWrapper::CXmlParserWrapperImpl::GetItemsXSLTObj
    (
        MSXML2::IXMLDOMNode*& xslItemsObj
    )
    {
        xslItemsObj = nullptr;
        if (mXslItemsObj == nullptr)
        {
            bstr_t bstrItemsXSLT(msXslt.c_str());
            if (!bstrItemsXSLT)
            {
                THROW_ERROR(L"BSTR memory allocation error");
            }
            HRESULT hr = mXslItemsObj.CreateInstance(__uuidof(MSXML2::DOMDocument60));
            if (FAILED(hr))
            {
                std::wostringstream ss;
                ss << L"CreateInstance failed. Error code: " << std::hex << hr;
                THROW_ERROR(ss.str().c_str());
            }
            if (VARIANT_TRUE != mXslItemsObj->loadXML(bstrItemsXSLT))
            {
                THROW_ERROR(L"MSXML2::IXMLDOMDocumentPtr::loadXML failed");
            }
        }
        xslItemsObj = mXslItemsObj;
    }
}
