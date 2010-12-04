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
#include "graphiteng/Font.h"
#include "XmlTraceLog.h"
#include "GrFaceImp.h"
#include "GrCachedFace.h"

#ifndef DISABLE_FILE_FACE

namespace org { namespace sil { namespace graphite { namespace v2 {



FileFace::FileFace(const char *filename) :
    m_pHeader(NULL),
    m_pTableDir(NULL)
{
    if (!(m_pfile = fopen(filename, "rb"))) return;
    size_t lOffset, lSize;
    if (!TtfUtil::GetHeaderInfo(lOffset, lSize)) return;
    m_pHeader = (TtfUtil::Sfnt::OffsetSubTable*)gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pHeader, 1, lSize, m_pfile) != lSize) return;
    if (!TtfUtil::CheckHeader(m_pHeader)) return;
    if (!TtfUtil::GetTableDirInfo(m_pHeader, lOffset, lSize)) return;
    m_pTableDir = (TtfUtil::Sfnt::OffsetSubTable::Entry*)gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pTableDir, 1, lSize, m_pfile) != lSize) return;
}

FileFace::~FileFace()
{
    if (m_pTableDir)
        free(m_pTableDir);
    if (m_pHeader)
        free(m_pHeader);
    if (m_pfile)
        fclose(m_pfile);
    m_pTableDir = NULL;
    m_pfile = NULL;
    m_pHeader = NULL;
}


const void *FileFace_table_fn(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const FileFace* ttfFaceHandle = (const FileFace*)appFaceHandle;
    TableCacheItem * res;
    switch (name)
    {
        case tagCmap:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiCmap];
            break;
//        case tagCvt:
//            res = &m_tables[TtfUtil::ktiCvt];
//            break;
//        case tagCryp:FileFace
//            res = &m_tables[TtfUtil::ktiCryp];
//            break;
        case tagHead:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHead];
            break;
//        case tagFpgm:
//            res = &m_tables[TtfUtil::ktiFpgm];
//            break;
//        case tagGdir:
//            res = &m_tables[TtfUtil::ktiGdir];
//            break;
        case tagGlyf:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGlyf];
            break;
        case tagHdmx:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHdmx];
            break;
        case tagHhea:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHhea];
            break;
        case tagHmtx:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHmtx];
            break;
        case tagLoca:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiLoca];
            break;
        case tagKern:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiKern];
            break;
//        case tagLtsh:
//            res = &m_tables[TtfUtil::ktiLtsh];
//            break;
        case tagMaxp:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiMaxp];
            break;
        case tagName:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiName];
            break;
        case tagOs2:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiOs2];
            break;
        case tagPost:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiPost];
            break;
//        case tagPrep:
//            res = &m_tables[TtfUtil::ktiPrep];
//            break;
        case tagFeat:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiFeat];
            break;
        case tagGlat:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGlat];
            break;
        case tagGloc:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGloc];
            break;
        case tagSilf:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSilf];
            break;
        case tagSile:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSile];
            break;
        case tagSill:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSill];
            break;
        default:
            res = NULL;
    }
    assert(res); // don't expect any other table types
    if (!res) return NULL;
    if (res->data() == NULL)
    {
        char *tptr;
        size_t tlen, lOffset;
        if (!TtfUtil::GetTableInfo(name, ttfFaceHandle->m_pHeader, ttfFaceHandle->m_pTableDir, lOffset, tlen)) return NULL;
        if (fseek(ttfFaceHandle->m_pfile, lOffset, SEEK_SET)) return NULL;
        tptr = gralloc<char>(tlen);
        if (fread(tptr, 1, tlen, ttfFaceHandle->m_pfile) != tlen) return NULL;
        res->set(tptr, tlen);
    }
    if (len) *len = res->size();
    return res->data();
}
#endif			//!DISABLE_FILE_FACE

extern "C" 
{
    GRNG_EXPORT GrFace* make_face(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, int canDumb)
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_face    
    {
        GrFace *res = new GrFace(appFaceHandle, getTable);

#ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementFace);
#endif
        bool valid = true;
        valid &= res->readGlyphs();
        if (!valid) {
            delete res;
            return 0;
        }
        valid &= res->readGraphite();
        valid &= res->readFeatures();
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementFace);
#endif
        
        if (!canDumb && !valid) {
            delete res;
            return 0;
        }
        return res;
    }

    GRNG_EXPORT GrFeatureVal* face_featureval_for_lang(const GrFace* pFace, uint32 langname/*0 means clone default*/) //clones the features. if none for language, clones the default
    {
        assert(pFace);
        return pFace->theSill().cloneFeatures(langname);
    }


    GRNG_EXPORT const GrFeatureRef* face_find_fref(const GrFace* pFace, uint32 featId)  //When finished with the FeatureRef, call destroy_FeatureRef
    {
        assert(pFace);
        const FeatureRef* pRef = pFace->featureById(featId);
        return pRef;
    }

    GRNG_EXPORT unsigned short face_n_fref(const GrFace* pFace)
    {
        assert(pFace);
        return pFace->numFeatures();
    }

    GRNG_EXPORT const GrFeatureRef* face_fref(const GrFace* pFace, uint16 i) //When finished with the FeatureRef, call destroy_FeatureRef
    {
        assert(pFace);
        const FeatureRef* pRef = pFace->feature(i);
        return pRef;
    }

    GRNG_EXPORT unsigned short face_n_languages(const GrFace* pFace)
    {
        assert(pFace);
        return pFace->theSill().numLanguages();
    }

    GRNG_EXPORT uint32 face_lang_by_index(const GrFace* pFace, uint16 i)
    {
        assert(pFace);
        return pFace->theSill().getLangName(i);
    }


    GRNG_EXPORT void face_destroy(GrFace *face)
    {
        delete face;
    }


    GRNG_EXPORT uint16 face_name_lang_for_locale(GrFace *face, const char * locale)
    {
        if (face)
        {
            return face->languageForLocale(locale);
        }
        return 0;
    }

 #if 0      //hidden since no way to release atm.
 
    GRNG_EXPORT uint16 *face_name(const GrFace * pFace, uint16 nameid, uint16 lid)
    {
        size_t nLen = 0, lOffset = 0, lSize = 0;
        const void *pName = pFace->getTable(tagName, &nLen);
        uint16 *res;
        if (!pName || !TtfUtil::GetNameInfo(pName, 3, 0, lid, nameid, lOffset, lSize))
            return NULL;
        lSize >>= 1;
        res = gralloc<uint16>(lSize + 1);
        for (size_t i = 0; i < lSize; ++i)
            res[i] = swap16(*(uint16 *)((char *)pName + lOffset));
        res[lSize] = 0;
        return res;
    }
#endif

    GRNG_EXPORT unsigned short face_n_glyphs(const GrFace* pFace)
    {
        return pFace->getGlyphFaceCache()->numGlyphs();
    }


#ifndef DISABLE_FILE_FACE
    GRNG_EXPORT GrFace* make_file_face(const char *filename)   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_face
    {
        FileFace* pFileFace = new FileFace(filename);
        if (pFileFace->m_pTableDir)
        {
          GrFace* pRes = make_face(pFileFace, &FileFace_table_fn, 0);
          if (pRes)
          {
            pRes->takeFileFace(pFileFace);        //takes ownership
            return pRes;
          }
        }
        
        //error when loading

        delete pFileFace;
        return NULL;
    }

#endif      //!DISABLE_FILE_FACE
}

}}}}// end namespace

