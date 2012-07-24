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

    Alternatively, the contents of this file may be used under the terms
    of the Mozilla Public License (http://mozilla.org/MPL) or the GNU
    General Public License, as published by the Free Software Foundation,
    either version 2 of the License or (at your option) any later version.
*/
#pragma once

#include <graphite2/Types.h>
#include <graphite2/Font.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Start logging all segment creation and updates on the provided face.  This
  * is logged to a JSON file, see "Segment JSON Schema.txt" for a precise
  * definition of the file
  *
  * @return true if the file was successfully created and logging is correctly
  * 			 initialised.
  * @param face the gr_face whose segments you want to log to the given file
  * @param log_path a utf8 encoded file name and path to log to.
  */
GR2_API bool graphite_start_logging(gr_face * face, const char *log_path);


/** Stop logging on the given face.  This will close the log file created by
  * graphite_start_loging.
  *
  * @param face the gr_face whose segments you want to stop logging
  */
GR2_API void graphite_stop_logging(gr_face * face);

#ifdef __cplusplus
}
#endif
