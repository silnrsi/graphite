/*  GRAPHITENG LICENSING

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
    If not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Main.h"
#include "FeatureMap.h"
#include "TtfTypes.h"
#include "TtfUtil.h"


namespace gr2 = org::sil::graphite::v2;

#pragma pack(push, 1)

// TODO fix this for other platforms

//#define PACKED __attribute__ ((packed))

struct FeatHeader
{
    gr2::uint16 m_major;
    gr2::uint16 m_minor;
    gr2::uint16 m_numFeat;
    gr2::uint16 m_reserved1;
    gr2::uint32 m_reserved2;
};

struct FeatDefn
{
    gr2::uint32 m_featId;
    gr2::uint16 m_numFeatSettings;
    gr2::uint16 m_reserved1;
    gr2::uint32 m_settingsOffset;
    gr2::uint16 m_flags;
    gr2::uint16 m_label;
};

struct FeatSetting
{
    gr2::int16 m_value;
    gr2::uint16 m_label;
};

struct FeatTable
{
    FeatHeader m_header;
    FeatDefn m_defs[0];
    FeatSetting m_settings[0];
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

class DummyFaceHandle
{
public:
    DummyFaceHandle() : m_table(NULL), m_tableLen(0) {}
    ~DummyFaceHandle() { if (m_table) { free(m_table); m_table = NULL; m_tableLen = 0; }}
    template <class T> void init(const T & data)
    {
        if (m_table) free(m_table);
        m_table = malloc(sizeof(T));
        memcpy(m_table, &data, sizeof(T));
        // convert to big endian if needed
        T * bigEndian = reinterpret_cast<T*>(m_table);
        bigEndian->m_header.m_major = gr2::swap16(data.m_header.m_major);
        bigEndian->m_header.m_minor = gr2::swap16(data.m_header.m_minor);
        bigEndian->m_header.m_numFeat = gr2::swap16(data.m_header.m_numFeat);
        m_tableLen = sizeof(T);
        for (size_t i = 0; i < sizeof(data.m_defs)/sizeof(FeatDefn); i++)
        {
            bigEndian->m_defs[i].m_featId = gr2::swap32(data.m_defs[i].m_featId);
            bigEndian->m_defs[i].m_numFeatSettings = gr2::swap16(data.m_defs[i].m_numFeatSettings);
            bigEndian->m_defs[i].m_settingsOffset = gr2::swap32(data.m_defs[i].m_settingsOffset);
            bigEndian->m_defs[i].m_flags = gr2::swap16(data.m_defs[i].m_flags);
            bigEndian->m_defs[i].m_label = gr2::swap16(data.m_defs[i].m_label);
        }
        for (size_t i = 0; i < sizeof(data.m_settings)/sizeof(FeatSetting); i++)
        {
            bigEndian->m_settings[i].m_value = gr2::swap16(data.m_settings[i].m_value);
            bigEndian->m_settings[i].m_label = gr2::swap16(data.m_settings[i].m_label);
        }
    }
    void * m_table;
    size_t m_tableLen;
};

const void * getTestFeat(const void* appFaceHandle, unsigned int /*name*/, size_t *len)
{
    const DummyFaceHandle * dummyFace = reinterpret_cast<const DummyFaceHandle*>(appFaceHandle);
    if (len)
        *len = dummyFace->m_tableLen;
    return dummyFace->m_table;
}

template <class T> void testAssert(const char * msg, T b)
{
    if (!b)
    {
        fprintf(stderr, msg, b);
        exit(1);
    }
}

template <class T> void testAssertEqual(const char * msg, T a, T b)
{
    if (a != b)
    {
        fprintf(stderr, msg, a, b);
        exit(1);
    }
}

template <class T> void testFeatTable(const T & table, const char * testName)
{
    gr2::FeatureMap testFeatureMap;
    DummyFaceHandle dummyFace;
    dummyFace.init<T>(table);
    const gr2::GrFace* npFace=NULL;
    bool readStatus = testFeatureMap.readFeats(&dummyFace, getTestFeat, npFace);
    testAssert("readFeats", readStatus);
    fprintf(stderr, testName, NULL);
    testAssertEqual("test num features %hu,%hu\n", testFeatureMap.numFeats(), table.m_header.m_numFeat);

    for (size_t i = 0; i < sizeof(table.m_defs) / sizeof(FeatDefn); i++)
    {
        const gr2::FeatureRef * ref = testFeatureMap.findFeatureRef(table.m_defs[i].m_featId);
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

int main(int argc, char ** argv)
{
    assert(sizeof(struct FeatTable) == sizeof(struct FeatHeader));
    testFeatTable<FeatTableTestA>(testDataA, "A\n");
    testFeatTable<FeatTableTestB>(testDataB, "B\n");
    testFeatTable<FeatTableTestB>(testDataBunsorted, "Bu\n");
    testFeatTable<FeatTableTestC>(testDataCunsorted, "C\n");
    testFeatTable<FeatTableTestD>(testDataDunsorted, "D\n");
    testFeatTable<FeatTableTestE>(testDataE, "E\n");

    // test a bad settings offset stradling the end of the table
    gr2::FeatureMap testFeatureMap;
    DummyFaceHandle dummyFace;
    dummyFace.init<FeatTableTestE>(testBadOffset);
    const gr2::GrFace* npFace=NULL;
    bool readStatus = testFeatureMap.readFeats(&dummyFace, getTestFeat, npFace);
    testAssert("fail gracefully on bad table", !readStatus);

    return 0;
}
