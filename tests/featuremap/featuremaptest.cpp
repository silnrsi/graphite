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
*/
#include <cstdlib>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <graphite2/Font.h>
#include "inc/Endian.h"
#include "inc/Face.h"
#include "inc/FeatureMap.h"
#include "inc/TtfUtil.h"

#pragma pack(push, 1)

using namespace graphite2;

template<typename T> class _be
{
	T _v;
public:
	_be(const T & t) throw() 				{_v = be::swap<T>(t);}

	operator T () const throw()				{return be::swap<T>(_v); }
};

struct FeatHeader
{
    _be<gr_uint16> m_major;
    _be<gr_uint16> m_minor;
    _be<gr_uint16> m_numFeat;
    _be<gr_uint16> m_reserved1;
    _be<gr_uint32> m_reserved2;
};

struct FeatDefn
{
    _be<gr_uint32> m_featId;
    _be<gr_uint16> m_numFeatSettings;
    _be<gr_uint16> m_reserved1;
    _be<gr_uint32> m_settingsOffset;
    _be<gr_uint16> m_flags;
    _be<gr_uint16> m_label;
};

struct FeatSetting
{
    _be<gr_int16>	m_value;
    _be<gr_uint16>	m_label;
};

struct FeatTableTestA
{
    FeatHeader m_header;
    FeatDefn m_defs[1];
    FeatSetting m_settings[2];
};

const FeatTableTestA testDataA = {
    { 2, 0, 1, 0, 0},
    {{0x41424344, 2, 0, sizeof(FeatHeader) + sizeof(FeatDefn), 0, 1}},
    {{0,10},{1,11}}
};

struct FeatTableTestB
{
    FeatHeader m_header;
    FeatDefn m_defs[2];
    FeatSetting m_settings[4];
};

const FeatTableTestB testDataB = {
    { 2, 0, 2, 0, 0},
    {{0x41424344, 2, 0, sizeof(FeatHeader) + 2 * sizeof(FeatDefn), 0, 1},
     {0x41424345, 2, 0, sizeof(FeatHeader) + 2 * sizeof(FeatDefn) + 2 * sizeof(FeatSetting), 0, 2}},
    {{0,10},{1,11},{0,12},{1,13}}
};
const FeatTableTestB testDataBunsorted = {
    { 2, 0, 2, 0, 0},
    {{0x41424345, 2, 0, sizeof(FeatHeader) + 2 * sizeof(FeatDefn) + 2 * sizeof(FeatSetting), 0, 2},
     {0x41424344, 2, 0, sizeof(FeatHeader) + 2 * sizeof(FeatDefn), 0, 1}},
    {{0,10},{1,11},{0,12},{1,13}}
};

struct FeatTableTestC
{
    FeatHeader m_header;
    FeatDefn m_defs[3];
    FeatSetting m_settings[7];
};

const FeatTableTestC testDataCunsorted = {
    { 2, 0, 3, 0, 0},
    {{0x41424343, 3, 0, sizeof(FeatHeader) + 3 * sizeof(FeatDefn) + 4 * sizeof(FeatSetting), 0, 1},
     {0x41424345, 2, 0, sizeof(FeatHeader) + 3 * sizeof(FeatDefn) + 2 * sizeof(FeatSetting), 0, 3},
     {0x41424344, 2, 0, sizeof(FeatHeader) + 3 * sizeof(FeatDefn), 0, 2}},
    {{0,10},{1,11},{0,12},{1,13},{0,14},{1,15},{2,16}}
};

struct FeatTableTestD
{
    FeatHeader m_header;
    FeatDefn m_defs[4];
    FeatSetting m_settings[9];
};

const FeatTableTestD testDataDunsorted = {
    { 2, 0, 4, 0, 0},
    {{400, 3, 0, sizeof(FeatHeader) + 4 * sizeof(FeatDefn) + 4 * sizeof(FeatSetting), 0, 1},
     {100, 2, 0, sizeof(FeatHeader) + 4 * sizeof(FeatDefn) + 2 * sizeof(FeatSetting), 0, 3},
     {300, 2, 0, sizeof(FeatHeader) + 4 * sizeof(FeatDefn), 0, 2},
     {200, 2, 0, sizeof(FeatHeader) + 4 * sizeof(FeatDefn) + 7 * sizeof(FeatSetting), 0, 2}
    },
    {{0,10},{1,11},{0,12},{10,13},{0,14},{1,15},{2,16},{2,17},{4,18}}
};

struct FeatTableTestE
{
    FeatHeader m_header;
    FeatDefn m_defs[5];
    FeatSetting m_settings[11];
};
const FeatTableTestE testDataE = {
    { 2, 0, 5, 0, 0},
    {{400, 3, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 4 * sizeof(FeatSetting), 0, 1},
     {100, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 2 * sizeof(FeatSetting), 0, 3},
     {500, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 9 * sizeof(FeatSetting), 0, 3},
     {300, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn), 0, 2},
     {200, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 7 * sizeof(FeatSetting), 0, 2}
    },
    {{0,10},{1,11},{0,12},{10,13},{0,14},{1,15},{2,16},{2,17},{4,18},{1,19},{2,20}}
};

const FeatTableTestE testBadOffset = {
    { 2, 0, 5, 0, 0},
    {{400, 3, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 4 * sizeof(FeatSetting), 0, 1},
     {100, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 2 * sizeof(FeatSetting), 0, 3},
     {500, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 9 * sizeof(FeatSetting), 0, 3},
     {300, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn), 0, 2},
     {200, 2, 0, sizeof(FeatHeader) + 5 * sizeof(FeatDefn) + 10 * sizeof(FeatSetting), 0, 2}
    },
    {{0,10},{1,11},{0,12},{10,13},{0,14},{1,15},{2,16},{2,17},{4,18},{1,19},{2,20}}
};

#pragma pack(pop)

class face_handle
{
public:
	typedef std::pair<const void *, size_t>		table_t;
	static const table_t						no_table;

	face_handle(const char *backing_font_path = 0)
	: _header(0), _dir(0)
	{
		if (!backing_font_path) return;
		std::ifstream 	font_file(backing_font_path, std::ifstream::binary);
		const size_t	font_size = size_t(font_file.seekg(0, std::ios::end).tellg());
		font_file.seekg(0, std::ios::beg);
		_header = new char [font_size];
		font_file.read(const_cast<char *>(_header), font_size);
	    if (!TtfUtil::CheckHeader(_header))
	    	throw std::runtime_error(std::string(backing_font_path) + ": invalid font");
	    size_t dir_off, dir_sz;
	    if (!TtfUtil::GetTableDirInfo(_header, dir_off, dir_sz))
	    	throw std::runtime_error(std::string(backing_font_path) + ": invalid font");
	    _dir = _header + dir_off;
	}

	void replace_table(const TtfUtil::Tag name, const void * const data, size_t len) throw() {
		_tables[name] = std::make_pair(data, len);
	}

	const table_t & operator [] (const TtfUtil::Tag name) const throw() {
		const table_t & table = _tables[name];
		if (table.first)	return table;

		size_t off, len;
		if (!TtfUtil::GetTableInfo(name, _header, _dir, off, len))
			return no_table;
		return _tables[name] = table_t(_header + off, len);
	}

	static const gr_face_ops ops;
private:
	static const void * get_table_fn(const void* afh, unsigned int name, size_t *len) {
		const face_handle & fh = *reinterpret_cast<const face_handle *>(afh);
		const table_t & t = fh[name];
		*len = t.second;
		return t.first;
	}

    const char 						  * _header,
									  * _dir;
	mutable std::map<const TtfUtil::Tag, table_t> _tables;
};

const face_handle::table_t	face_handle::no_table = face_handle::table_t(reinterpret_cast<void *>(0),0);
const gr_face_ops face_handle::ops = { sizeof(gr_face_ops), face_handle::get_table_fn, 0 };


template <typename T> void testAssert(const char * msg, const T b)
{
    if (!b)
    {
        fprintf(stderr, msg, b);
        exit(1);
    }
}

template <typename T, typename R> void testAssertEqual(const char * msg, const T a, const R b)
{
    if (a != T(b))
    {
        fprintf(stderr, msg, a, T(b));
        exit(1);
    }
}

face_handle dummyFace;

template <class T> void testFeatTable(const T & table, const char * testName)
{
    FeatureMap testFeatureMap;
    dummyFace.replace_table(TtfUtil::Tag::Feat, &table, sizeof(T));
    const gr_face * face = gr_make_face_with_ops(&dummyFace, &face_handle::ops, gr_face_dumbRendering);
    if (!face) throw std::runtime_error("failed to load font");
    bool readStatus = testFeatureMap.readFeats(*face);
    testAssert("readFeats", readStatus);
    fprintf(stderr, testName, NULL);
    testAssertEqual("test num features %hu,%hu\n", testFeatureMap.numFeats(), table.m_header.m_numFeat);

    for (size_t i = 0; i < sizeof(table.m_defs) / sizeof(FeatDefn); i++)
    {
        const FeatureRef * ref = testFeatureMap.findFeatureRef(table.m_defs[i].m_featId);
        testAssert("test feat\n", ref);
        testAssertEqual("test feat settings %hu %hu\n", ref->getNumSettings(), table.m_defs[i].m_numFeatSettings);
        testAssertEqual("test feat label %hu %hu\n", ref->getNameId(), table.m_defs[i].m_label);
        size_t settingsIndex = (table.m_defs[i].m_settingsOffset - sizeof(FeatHeader)
            - (sizeof(FeatDefn) * table.m_header.m_numFeat)) / sizeof(FeatSetting);
        for (size_t j = 0; j < table.m_defs[i].m_numFeatSettings; j++)
        {
            testAssertEqual("setting label %hu %hu\n", ref->getSettingName(j),
                       table.m_settings[settingsIndex+j].m_label);
        }
    }
}

int main(int argc, char * argv[])
{
	try
	{
		if (argc != 2)	throw std::length_error("not enough arguments: need a backing font");

		dummyFace = face_handle(argv[1]);
		testFeatTable<FeatTableTestA>(testDataA, "A\n");
		testFeatTable<FeatTableTestB>(testDataB, "B\n");
		testFeatTable<FeatTableTestB>(testDataBunsorted, "Bu\n");
		testFeatTable<FeatTableTestC>(testDataCunsorted, "C\n");
		testFeatTable<FeatTableTestD>(testDataDunsorted, "D\n");
		testFeatTable<FeatTableTestE>(testDataE, "E\n");

		// test a bad settings offset stradling the end of the table
		FeatureMap testFeatureMap;
		dummyFace.replace_table(TtfUtil::Tag::Feat, &testBadOffset, sizeof testBadOffset);
		const gr_face * face = gr_make_face_with_ops(&dummyFace, &face_handle::ops, gr_face_dumbRendering);
		bool readStatus = testFeatureMap.readFeats(*face);
		testAssert("fail gracefully on bad table", !readStatus);
	}
	catch (std::exception & e)
	{
		fprintf(stderr, "%s: %s\n", argv[0], e.what());
		return 1;
	}

    return 0;
}
