/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */


#ifndef LGVS_TYPES_H
#define LGVS_TYPES_H


#include <string>
#include <vector>


namespace lgvs {


typedef std::vector<std::wstring> WStringList;
typedef WStringList::iterator WStringListIt;
typedef WStringList::const_iterator WStringListCIt;


} // namespace lgvs


#endif // LGVS_TYPES_H
