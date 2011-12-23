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
#include "graphite2/Log.h"
#include "debug.h"
#include "CharInfo.h"
#include "Slot.h"

using namespace graphite2;

extern "C" {


bool graphite_start_logging(FILE * logFile, GrLogMask mask)
{
	if (!logFile || !mask)	return false;

	dbgout = new json(logFile);
	return dbgout;
}

void graphite_stop_logging()
{
	delete dbgout;
}


} // extern "C"


namespace
{
	inline
	uint32 slotid(const Slot * const p) throw()
	{
		size_t s = size_t(p);
		s ^= s >> (s & 7) & ~size_t(0xffff);
		s ^= s >> (s & 3) & ~size_t(0xffffff);
		return uint32(s);
	}
}

json *graphite2::dbgout = 0;


json & graphite2::operator << (json & j, const CharInfo & ci) throw()
{
	return j << json::object
				<< "base"			<< ci.base()
				<< "unicode"		<< ci.unicodeChar()
				<< "breakWeight"	<< ci.breakWeight()
				<< "slot" << json::object << json::flat
					<< "before"	<< ci.before()
					<< "after"	<< ci.after()
					<< json::close
				<< json::close;
}


json & graphite2::operator << (json & j, const Slot & s) throw()
{
	j << json::object
		<< "id"				<< slotid(&s)
		<< "gid"			<< s.gid()
		<< "charinfo" << json::object << json::flat
			<< "original"		<< s.original()
			<< "before"			<< s.before()
			<< "after" 			<< s.after()
			<< json::close
		<< "origin"			<< s.origin()
		<< "advance"		<< s.advancePos()
		<< "insertBefore"	<< s.isInsertBefore();
	if (s.getBidiLevel() > 0)
		j 	<< "bidi"		<< s.getBidiLevel();
	if (s.attachedTo())
		j << "parent" << json::object << json::flat
			<< "id"				<< s.attachedTo()->index()
			<< "offset"			<< s.attachOffset()
			<< json::close;
	if (s.firstChild())
	{
		j	<< "children" << json::array << json::flat;
		for (const Slot *c = s.firstChild(); c; c = c->nextSibling())  j << c->index();
		j		<< json::close;
	}
	return j << json::close;
}


json & graphite2::operator << (json & j, const slots & r) throw()
{
	const Slot * s = r.first,
			   * const end = r.second ? r.second->next() : 0;
	j << json::array;
	for (; s && s != end; s = s->next()) j << *s;
	j << json::close;
	return j;
}
