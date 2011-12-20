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


void json::context(const char current) throw()
{
	assert(_context - _contexts < std::ptrdiff_t(sizeof _contexts));
	fprintf(_stream, "%.1s ", _context);
	indent();
	*_context = current;
}


inline
void json::indent(const int d) throw()
{
	if (_flatten)
		return;
	else
		switch(*_context)
		{
		case ',':
		case '{':
		case '[':
		case ']':
		case '}':
			fprintf(_stream, "\n%*s",  4*int(_context - _contexts + d), "");
			break;
		}
}

inline
void json::push_context(const char prefix, const char suffix) throw()
{
	context(suffix);
	*++_context = prefix;
}

void json::pop_context() throw()
{
	assert(_context > _contexts);

	if (*_context != ',')
		fputc(*_context, _stream);
	else
		indent(-1);
	fputc(*--_context, _stream);
	*_context = ',';

	if (_flatten >= _context)	_flatten = 0;
}


void json::flat(json & j) throw()
{
	j._flatten = j._context;
}

void json::close(json & j) throw()
{
	j.pop_context();
}


void json::object(json & j) throw()
{
	j.push_context('{', '}');
}


void json::array(json & j) throw()
{
	j.push_context('[', ']');
}


json::~json() throw ()
{
	while (_context > _contexts)	pop_context();
	fputc('\n',_stream);
	fflush(_stream);
}


json & json::operator << (json::string s) throw()
{
	const char ctxt = _context[-1] == '}' && *_context != ':' ? ':' : ',';
	context(ctxt);
	fprintf(_stream, "\"%s\"", s);
	if (ctxt == ':')	fputc(' ', _stream);

	return *this;
}

json & json::operator << (json::number f) throw()	{ context(','); fprintf(_stream, "%f", f); return *this; }
json & json::operator << (json::integer d) throw()	{ context(','); fprintf(_stream, "%d", d); return *this; }
json & json::operator << (json::boolean b) throw()	{ context(','); fputs(b ? "true" : "false", _stream); return *this; }
json & json::operator << (json::_null_t) throw()	{ context(','); fputs("null",_stream); return *this; }
json & json::operator << (_context_t ctxt) throw()	{ ctxt(*this); return *this; }

