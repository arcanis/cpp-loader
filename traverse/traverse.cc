#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <list>

#include <clang-c/Index.h>

#include "./picojson.h"

struct FuncDef {

    std::string name;

    FuncDef(std::string const & name)
        : name(name)
    {}

};

struct ClassDef {

    std::string name;

    std::list<std::string> publicMethods;
    std::list<std::string> publicMembers;

    ClassDef(std::string const & name)
        : name(name), publicMethods(), publicMembers()
    {}

};

std::list<FuncDef> g_funcDefs;

std::list<ClassDef> g_classDefs;
std::list<ClassDef>::iterator g_currentClassDef;

picojson::value toPicoJson(std::string const & str)
{
    return picojson::value(str);
}

template <typename T> picojson::value toPicoJson(std::list<T> const & list)
{
    std::vector<picojson::value> transformed;
    transformed.reserve(list.size());

    for (auto const & entry : list)
        transformed.emplace_back(toPicoJson(entry));

    return picojson::value(transformed);
}

picojson::value toPicoJson(std::map<std::string, picojson::value> const & map)
{
    return picojson::value(map);
}

template <typename T> picojson::value toPicoJson(std::map<std::string, T> const & map)
{
    std::map<std::string, picojson::value> transformed;

    for (auto const & entry : map)
        transformed.emplace(entry.first, toPicoJson(entry.second));

    return picojson::value(transformed);
}

picojson::value toPicoJson(FuncDef const & funcDef)
{
    std::map<std::string, picojson::value> transformed;

    transformed.emplace("name", funcDef.name);

    return toPicoJson(transformed);
}

picojson::value toPicoJson(ClassDef const & classDef)
{
    std::map<std::string, picojson::value> transformed;

    transformed.emplace("name", classDef.name);

    transformed.emplace("publicMethods", toPicoJson(classDef.publicMethods));
    transformed.emplace("publicMembers", toPicoJson(classDef.publicMembers));

    return toPicoJson(transformed);
}

std::string getCursorKindName(CXCursorKind cursorKind)
{
    CXString kindName = clang_getCursorKindSpelling(cursorKind);
    std::string result = clang_getCString(kindName);

    clang_disposeString(kindName);

    return result;
}

std::string getCursorSpelling(CXCursor cursor)
{
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    std::string result = clang_getCString(cursorSpelling);

    clang_disposeString(cursorSpelling);

    return result;
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor /* parent */, CXClientData clientData)
{
    CXSourceLocation location = clang_getCursorLocation(cursor);

    if (!clang_Location_isFromMainFile(location))
        return CXChildVisit_Continue;

    CXCursorKind cursorKind = clang_getCursorKind(cursor);

    std::string cursorName = getCursorKindName(cursorKind);
    std::string cursorSpelling = getCursorSpelling(cursor);

    if (cursorName == "ClassDecl") {

        g_classDefs.emplace_back(cursorSpelling);
        g_currentClassDef = std::prev(g_classDefs.end());

    } else if (cursorName == "CXXMethod" || cursorName == "FieldDecl") {

        auto accessSpecifier = clang_getCXXAccessSpecifier(cursor);

        if (accessSpecifier == CX_CXXPublic) {

            if (cursorName == "CXXMethod") {
                g_currentClassDef->publicMethods.emplace_back(cursorSpelling);
            } else {
                g_currentClassDef->publicMembers.emplace_back(cursorSpelling);
            }

        }

    } else if (cursorName == "FunctionDecl") {

        auto linkage = clang_getCursorLinkage(cursor);

        if (linkage == CXLinkage_External) {
            g_funcDefs.emplace_back(cursorSpelling);
        }

    } else {

        //std::cout << cursorName << std::endl;

    }

    clang_visitChildren(cursor, visitor, nullptr);

    return CXChildVisit_Continue;
}

int main(int argc, char ** argv)
{
    if (argc < 2)
        return -1;

    CXIndex index = clang_createIndex(0, 1);

    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,                 // where to store the AST?
        argv[1],               // which file should be parsed?
        argv + 2, argc - 2,    // what are the Clang command-line flags?
        nullptr, 0,            // what are the in-memory files?
        CXTranslationUnit_None // what are the parsing flags?
    );

    if (!tu) {
        std::cerr << "Failed to parse the specified file." << std::endl;
        return -1;
    }

    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(rootCursor, visitor, nullptr);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    std::map<std::string, picojson::value> output {
        { "functions", toPicoJson(g_funcDefs) },
        { "classes", toPicoJson(g_classDefs) },
    };

    picojson::value(output).serialize(std::ostream_iterator<char>(std::cout));

    return 0;
}
