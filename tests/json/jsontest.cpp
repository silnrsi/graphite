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
// JSON debug logging very basic test harness
// Author: Tim Eves
#include "json.h"

using namespace graphite2;

int main(int argc, char * argv[])
{
	json	jo(argc == 1 ? stdout : fopen(argv[1], "w"));

	jo << json::object
			<< json::key("empty object") << json::object << json::close;

	jo << json::key("primitive types") << json::object
			<< json::key("string")	<< "a string"
			<< json::key("number") 	<< 0.54
			<< json::key("integer") << 123
			<< json::key("boolean") << false
			<< json::key("null") 	<< json::null
			<< json::close;

	jo << json::key("complex object") << json::object
			<< json::key("firstName") 	<< "John"
			<< json::key("lastName") 	<< "Smith"
			<< json::key("age")			<< 25
			<< json::key("address")		<< json::object
				<< json::key("streetAddress") << "21 2nd Street"
				<< json::key("city")		<< "New York"
				<< json::key("state")		<< "NY"
				<< json::key("postalCode") 	<< "10021"
				<< json::close
			<< json::key("phoneNmuber") << json::array
				<< json::object
					<< json::key("type") 	<< "home"
					<< json::key("number") 	<< "212 555-1234"
					<< json::close
				<< json::object
					<< json::key("type") 	<< "fax"
					<< json::key("number")	<< "646 555-4567";

	return 0;
}

