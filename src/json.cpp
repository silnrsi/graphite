/*  GRAPHITE2 LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, 51 Franklin Street,
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
// JSON debug logging
// Author: Tim Eves

#include <cstdio>
#include "json.h"

using namespace graphite2;

void json::push_context(const char *opener, const char closer) throw()
{
	assert(_context - _contexts < std::ptrdiff_t(sizeof _contexts));
	fprintf(_stream, "%s\n%*s%s", _sep,  4*int(_context - _contexts+1), "", opener);
	_sep = "";
	*++_context = closer;
}

void json::pop_context()
{
	assert(_context >= _contexts);
	fprintf(_stream, "\n%*c",  4*int(_context - _contexts)+1, *_context);
	--_context;
	_sep = ", ";
}


void json::property::operator()(json & j) throw()
{
	assert(*j._context == '}');
	fprintf(j._stream, "%s\n%*s\"%s\" : ", j._sep,  4*int(j._context - j._contexts+1), " ", _name);
	j._sep = "";
}


json::~json() throw ()
{
	while (_context >= _contexts)
	{
		fprintf(_stream, "\n%*c", 4*int(_context - _contexts)+1, *_context);
		--_context;
}
	fputc('\n',_stream);
	fflush(_stream);
}


json & json::operator << (json::string s) throw()	{ fprintf(_stream, "%s\"%s\"", _sep, s); _sep = ", "; return *this; }
json & json::operator << (json::number f) throw()	{ fprintf(_stream, "%s%.f", _sep, f); _sep = ", "; return *this; }
json & json::operator << (json::integer d) throw()	{ fprintf(_stream, "%s%d", _sep, d); _sep = ", "; return *this; }
json & json::operator << (json::boolean b) throw()	{ fputs(_sep, _stream); fputs(b ? "true" : "false", _stream); _sep = ", "; return *this; }
json & json::operator << (json::_null_t) throw()	{ fputs(_sep, _stream); fputs("null",_stream); _sep = ", "; return *this; }
