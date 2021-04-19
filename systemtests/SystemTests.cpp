#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "..\XmlParserWrapper.h"
#include "..\Util.h"
using namespace OTInterviewExercise1;

struct CTestFailureDescr
{
    CTestFailureDescr(const std::string& descr,	const std::string& testName, unsigned int lineNo) :
    mDescr(descr),
    mTestName(testName),
    mLineNo(lineNo)
    {}

    std::string mDescr;
    std::string mTestName;
    unsigned int mLineNo;
};

static unsigned int numTestsRan;
static unsigned int numTestsSucceeded;
static unsigned int numTestsFailed;
static std::vector<CTestFailureDescr> testFailures;

#define SYSTEST_ENTER() bool status = true;
#define SYSTEST_RETURN() return status;
#define SYSTEST_ASSERT(x)	\
{															\
    if (!(x))												\
    {														\
        CTestFailureDescr descr(#x, __FUNCTION__, __LINE__);\
        testFailures.push_back(descr);						\
        status = false;										\
    }														\
}															

bool Test_CException()
{
    SYSTEST_ENTER();

    CException ex;
    SYSTEST_ASSERT(ex.mCode == CException::ErrorCode::NoError);
    SYSTEST_ASSERT(ex.mInternalErrorCode == 0);
    SYSTEST_ASSERT(ex.mLineNo == 0);
    SYSTEST_ASSERT(ex.mFunctionName.empty());
    SYSTEST_ASSERT(ex.mErrorDescription.empty());

    CException ex2("SomeFunc", 111, CException::ErrorCode::Error, 121, L"Some error");
    SYSTEST_ASSERT(ex2.mCode == CException::ErrorCode::Error);
    SYSTEST_ASSERT(ex2.mInternalErrorCode == 121);
    SYSTEST_ASSERT(ex2.mLineNo == 111);
    SYSTEST_ASSERT(ex2.mFunctionName == "SomeFunc");
    SYSTEST_ASSERT(ex2.mErrorDescription == L"Some error");
    SYSTEST_RETURN();
}

#ifdef _WIN32
bool Test_OsInitialization()
{
    SYSTEST_ENTER();

    SYSTEST_ASSERT(S_OK == ::CoInitialize(nullptr));
    ::CoUninitialize();

    auto osInit = std::make_unique<COsInitialization>();
    SYSTEST_ASSERT(S_FALSE == ::CoInitialize(nullptr));
    ::CoUninitialize();
    osInit.reset();

    SYSTEST_ASSERT(S_OK == ::CoInitialize(nullptr));
    ::CoUninitialize();
    SYSTEST_RETURN();
}

bool Test_TextFileReader()
{
    SYSTEST_ENTER();

    // Should fail with ACCESS_DENIED
    std::wstring errorMsg;
    std::wstring fileData;
    CTextFileReader reader(L"c:\\windows\\system32");
    SYSTEST_ASSERT(reader.Exists(errorMsg));
    SYSTEST_ASSERT(!reader.GetContents(fileData,errorMsg));
    SYSTEST_ASSERT(!errorMsg.empty());
    SYSTEST_ASSERT(fileData.empty());

    std::wstring errorMsg2;
    std::wstring fileData2;
    CTextFileReader reader2(L"c:\\windows\\system32\\kernel32.dll");
    SYSTEST_ASSERT(reader2.Exists(errorMsg2));
    SYSTEST_ASSERT(!reader2.GetContents(fileData2, errorMsg2));
    SYSTEST_ASSERT(!errorMsg2.empty());
    SYSTEST_ASSERT(fileData2.empty());

    std::wstring errorMsg3;
    std::wstring fileData3;
    CTextFileReader reader3(L"c:\\nonexisting_file.txt");
    SYSTEST_ASSERT(!reader3.Exists(errorMsg3));
    SYSTEST_ASSERT(!reader3.GetContents(fileData3, errorMsg3));
    SYSTEST_ASSERT(!errorMsg3.empty());
    SYSTEST_ASSERT(fileData3.empty());

    SYSTEST_RETURN();
}
#endif

bool Test_RAIICleanup()
{
    SYSTEST_ENTER();

    int var = 0;
    {
        auto cleanup = MakeRAIICleanup([&var]() {
            var = 10;
            });
    }

    SYSTEST_ASSERT(var == 10);

    SYSTEST_RETURN();
}

bool Test_XmlParserWrapper()
{
    SYSTEST_ENTER();

    COsInitialization init;

    CXmlParserWrapper* pParser = new CXmlParserWrapper(CXmlParserWrapper::EMXSLTFile::None);
    std::wstring sHTML;
    std::wstring sError;
    SYSTEST_ASSERT(!pParser->Parse(L"<some>ttt</some>", sHTML, sError));
    SYSTEST_ASSERT(sHTML.empty());
    SYSTEST_ASSERT(!sError.empty());
    delete pParser;
    pParser = nullptr;

    pParser = new CXmlParserWrapper(CXmlParserWrapper::EMXSLTFile::CatalogResources);
    std::wstring sHTML2;
    std::wstring sError2;
    SYSTEST_ASSERT(!pParser->Parse(L"<some>ttt", sHTML2, sError2));
    SYSTEST_ASSERT(sHTML2.empty());
    SYSTEST_ASSERT(!sError2.empty());
    delete pParser;
    pParser = nullptr;

    pParser = new CXmlParserWrapper(CXmlParserWrapper::EMXSLTFile::CatalogResources);
    std::wstring sHTML3;
    std::wstring sError3;
    SYSTEST_ASSERT(pParser->Parse(L"<some>ttt</some>", sHTML3, sError3));
    SYSTEST_ASSERT(!sHTML3.empty());
    SYSTEST_ASSERT(sError3.empty());
    delete pParser;
    pParser = nullptr;

    SYSTEST_RETURN();
}

int main()
{
    std::vector<std::function<bool()>> v = {
    Test_CException,
    Test_OsInitialization,
    Test_TextFileReader,
    Test_RAIICleanup,
    Test_XmlParserWrapper
    };

    for (auto f : v)
    {
        bool ret = f();
        numTestsRan++;
        if (ret)
        {
            numTestsSucceeded++;
        }
        else
        {
            numTestsFailed++;
        }
    }
    std::cout << "Tests ran: " << numTestsRan <<
        ", Succeeded: " << numTestsSucceeded << ", Failed: " << numTestsFailed << std::endl;
    if (numTestsFailed)
    {
        std::cout << "Failures are: \n";
        for (auto errRec : testFailures)
        {
            std::cout << "TestName: " << errRec.mTestName << ", LineNo: " << errRec.mLineNo <<
                ", Statement: " << errRec.mDescr << std::endl;
        }
    }
    assert(numTestsRan == numTestsSucceeded + numTestsFailed);

    return ((numTestsRan > 0 && numTestsFailed == 0) ? 0 : 1);
}