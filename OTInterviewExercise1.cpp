#include <iostream>
#include "XmlParserWrapper.h"
#include "Util.h"

// Possible exit codes (of this application) - with 0 for success - and specific
// non-0 codes for errors.
enum class OTInterviewExercise1ExitCode
{
    SUCCESS = 0,
    INVALID_CMD_LINE,
    XML_FILE_NOT_FOUND,
    COULDNT_READ_XML_FILE,
    XML_FILE_IS_EMPTY,
    INIT_ERROR,
    XML_PARSER_ERROR
};

int wmain(int argc, wchar_t **argv)
{
    if (argc > 2)
    {
        std::wcerr << L"Invalid command-line params.\n"
            L"The only cmd-line parameter should be pathname of input XML file.\n"
            L"E.g. OTInterviewExercise1.exe c:\\temp\\catalog.xml\n"
            L"Or invoke without parameters to see help page\n";
        
        return (int)OTInterviewExercise1ExitCode::INVALID_CMD_LINE;
    }
    if (argc == 1 || wcslen(argv[1]) >= 2 && argv[1][0] == L'-' && towlower(argv[1][1]) == L'h')
    {
        std::wcerr << L"Usage: {EXE-path-name} {input-xml-file-pathname}\n"
            L"E.g. OTInterviewExercise1.exe c:\\temp\\catalog.xml\n"
            L"Output HTML will be written to stdout\n"
            L"Any error messages will be written to stderr\n"
            L"Exit codes are:\n"
            L"\t0 - success\n"
            L"\t1 - invalid command line\n"
            L"\t2 - XML file not found\n"
            L"\t3 - Couldn't read XML file\n"
            L"\t4 - XML file is empty\n"
            L"\t5 - initialization error\n"
            L"\t6 - parsing error\n";

        return (int)OTInterviewExercise1ExitCode::SUCCESS;
    }

    std::wstring sXml;
    std::wstring sErrorMsg;
    wchar_t* xmlFilePathName = argv[1];

    auto xmlFileReader = std::make_unique<OTInterviewExercise1::CTextFileReader>(xmlFilePathName);
    if (!xmlFileReader->Exists(sErrorMsg))
    {
        std::wcerr << L"File: " << xmlFilePathName << L" couldn't be opened. " << sErrorMsg << std::endl;
        return (int)OTInterviewExercise1ExitCode::XML_FILE_NOT_FOUND;
    }
    else if (!xmlFileReader->GetContents(sXml, sErrorMsg))
    {
        std::wcerr << L"Error reading contents of file: " << xmlFilePathName << L" " << sErrorMsg << std::endl;
        return (int)OTInterviewExercise1ExitCode::COULDNT_READ_XML_FILE;
    }
    else if (sXml.empty())
    {
        std::wcerr << L"File: " << xmlFilePathName << L" doesn't contain any XML." << std::endl;
        return (int)OTInterviewExercise1ExitCode::XML_FILE_IS_EMPTY;
    }
    // Close XML reader - it's not needed anymore
    xmlFileReader.reset();

    // Perform OS-specific initialization. Dtor will perform cleanup (if necessary).
    OTInterviewExercise1::COsInitialization init;
    if (!init.IsOk(sErrorMsg))
    {
        std::wcerr << L"Initialization error encountered. " << sErrorMsg << std::endl;
        return (int)OTInterviewExercise1ExitCode::INIT_ERROR;
    }

    // Create XML parser object using XSLT style-sheet in resources (stored in our EXE)
    OTInterviewExercise1::CXmlParserWrapper xmlParser(OTInterviewExercise1::CXmlParserWrapper::EMXSLTFile::CatalogResources);

    // Call XML parser to produce HTML output
    std::wstring sHtml;
    if (!xmlParser.Parse(
        sXml,
        sHtml,
        sErrorMsg))
    {
        std::wcerr << L"Xml parser error encountered. " << sErrorMsg << std::endl;
        return (int)OTInterviewExercise1ExitCode::XML_PARSER_ERROR;
    }

    // Write HTML to stdout
    std::wcout << sHtml << std::endl;
    return 0;
}