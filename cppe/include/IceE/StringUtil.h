// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_STRING_UTIL_H
#define ICEE_STRING_UTIL_H

#include <IceE/Config.h>

namespace IceE
{

//
// Add escape sequences (like "\n", or "\0xxx") to make a string
// readable in ASCII.
//
ICEE_API std::string escapeString(const std::string&, const std::string&);

//
// Remove escape sequences added by escapeString.
//
ICEE_API bool unescapeString(const std::string&, std::string::size_type, std::string::size_type, std::string&);

//
// If a single or double quotation mark is found at the start
// position, then the position of the matching closing quote is
// returned. If no quotation mark is found at the start position, then
// 0 is returned. If no matching closing quote is found, then
// std::string::npos is returned.
//
ICEE_API std::string::size_type checkQuote(const std::string&, std::string::size_type = 0);

//
// Match `s' against the pattern `pat'. A * in the pattern acts
// as a wildcard: it matches any non-empty sequence of characters
// other than a period (`.'). We match by hand here because
// it's portable across platforms (whereas regex() isn't).
//
ICEE_API bool match(const std::string&, const std::string&, bool = false);

}

#endif
