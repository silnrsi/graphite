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
#include "inc/debug.h"
#include "inc/CharInfo.h"
#include "inc/Slot.h"
#include "inc/Segment.h"

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

void finalise_slot(const Segment & seg, Slot & s) throw();

json *graphite2::dbgout = 0;


json & graphite2::operator << (json & j, const CharInfo & ci) throw()
{
	return j << json::object
				<< "offset"			<< ci.base()
				<< "unicode"		<< ci.unicodeChar()
				<< "break"			<< ci.breakWeight()
				<< "slot" << json::flat << json::object
					<< "before"	<< ci.before()
					<< "after"	<< ci.after()
					<< json::close
				<< json::close;
}


json & graphite2::operator << (json & j, const dslot & ds) throw()
{
	const Segment & seg = ds.first;
	Slot & s = ds.second;

	finalise_slot(seg, s);
	j << json::object
		<< "id"				<< slotid(&s)
		<< "gid"			<< s.gid()
		<< "charinfo" << json::flat << json::object
			<< "original"		<< s.original()
			<< "before"			<< s.before()
			<< "after" 			<< s.after()
			<< json::close
		<< "origin"			<< s.origin()
		<< "advance"		<< s.advancePos()
		<< "insert"			<< s.isInsertBefore()
		<< "break"			<< s.getAttr(&seg, gr_slatBreak, 0);
	if (s.just() > 0)
		j << "justification"	<< s.just();
	if (s.getBidiLevel() > 0)
		j << "bidi"		<< s.getBidiLevel();
	if (s.attachedTo())
		j << "parent" << json::flat << json::object
			<< "id"				<< s.attachedTo()->index()
			<< "level"			<< s.getAttr(0, gr_slatAttLevel, 0)
			<< "offset"			<< s.attachOffset()
			<< json::close;
	j << "user" << json::flat << json::array;
	for (int n = 0; n!= seg.numAttrs(); ++n)
		j	<< s.userAttrs()[n];
		j 	<< json::close;
	if (s.firstChild())
	{
		j	<< "children" << json::flat << json::array;
		for (const Slot *c = s.firstChild(); c; c = c->nextSibling())  j << slotid(c);
		j		<< json::close;
	}
	return j << json::close;
}


inline
void finalise_slot(const Segment & seg, Slot & s) throw()
{
	if (!s.isBase()) return;

	Position 	cp;
	Rect		bb;
	if (s.prev())  cp = s.prev()->origin() + s.prev()->advancePos();
	cp = s.finalise(&seg, 0, cp, bb, bb.tr.y, 0, bb.bl.y = cp.x);
}
