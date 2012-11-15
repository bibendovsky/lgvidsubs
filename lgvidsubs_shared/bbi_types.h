#ifndef BBI_TYPES_H
#define BBI_TYPES_H


#include <string>
#include <vector>


namespace bbi {


    typedef std::string String;
    typedef std::wstring WString;

#ifdef _UNICODE
    typedef WString TString;
#else
    typedef String TString;
#endif // _UNICODE

    typedef std::vector<WString> WStringList;


} // namespace bbi


#endif // BBI_TYPES_H
