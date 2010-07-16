#include "GlyphFaceCache.h"
#include "graphiteng/IFace.h"
#include "GrFace.h"     //for the tags

using namespace org::sil::graphite::v2;



/*virtual*/ bool GlyphFaceCacheHeader::initialize(const IFace* iFace/*not NULL*/)    //return result indicates success. Do not use if failed.
{
    if ((m_pLoca = iFace->getTable(tagLoca, &m_lLoca)) == NULL) return false;
    size_t lHead;
    if ((m_pHead = iFace->getTable(tagHead, &lHead)) == NULL) return false;
    size_t lGlyf;
    if ((m_pGlyf = iFace->getTable(tagGlyf, &lGlyf)) == NULL) return false;
    if ((m_pHmtx = iFace->getTable(tagHmtx, &m_lHmtx)) == NULL) return false;
    size_t lHHea;
    if ((m_pHHea = iFace->getTable(tagHhea, &lHHea)) == NULL) return false;
    size_t lGlat;
    if ((m_pGlat = iFace->getTable(tagGlat, &lGlat)) == NULL) return false;

    size_t lMaxp;
    const void* pMaxp = iFace->getTable(tagMaxp, &lMaxp);
    if (pMaxp==NULL) return false;
    m_nGlyphsWithGraphics = (unsigned short)TtfUtil::GlyphCount(pMaxp);
    
    size_t lGloc;
    if ((m_pGloc = iFace->getTable(tagGloc, &lGloc)) == NULL) return false;
    if (lGloc < 6) return false;
    int version = swap32(*((uint32 *)m_pGloc));
    if (version != 0x00010000) return false;

    m_numAttrs = swap16(((uint16 *)m_pGloc)[3]);

    unsigned short locFlags = swap16(((uint16 *)m_pGloc)[2]);
    if (locFlags&1)
    {
        m_locFlagsUse32Bit = true;
        m_nGlyphsWithAttributes = (unsigned short)((lGloc - 12) / 4);
    }
    else
    {
        m_locFlagsUse32Bit = false;
        m_nGlyphsWithAttributes = (unsigned short)((lGloc - 10) / 2);
    }
    
    if (m_nGlyphsWithAttributes>m_nGlyphsWithGraphics) 
        m_nGlyphs = m_nGlyphsWithAttributes;
    else
        m_nGlyphs = m_nGlyphsWithGraphics;

    return true;
}


/*static*/ EGlyphCacheStrategy GlyphFaceCache::nearestSupportedStrategy(EGlyphCacheStrategy requested)
{
    if (requested>=ePreload) return ePreload;
    if (requested>=eLoadOnDemand) return eLoadOnDemand;
    
    return eOneCache;
}



/*static*/ GlyphFaceCache* GlyphFaceCache::makeCache(const GlyphFaceCacheHeader& hdr, EGlyphCacheStrategy requested)
{
    switch (nearestSupportedStrategy(requested))
    {
      case ePreload:
            return new(hdr) GlyphFaceCachePreloaded(hdr);
      case eLoadOnDemand:
            return new(hdr) GlyphFaceCacheLoadedOnDemand(hdr);           
      default:      //eOneCache
            return new(hdr) GlyphFaceCacheOneItem(hdr);
    }
}



GlyphFaceCacheOneItem::GlyphFaceCacheOneItem(const GlyphFaceCacheHeader& hdr)   //always use with the above new, passing in the same GlyphFaceCacheHeader
:   GlyphFaceCache(hdr),
    m_LoadedGlyphNo(-1)   //-1 means none loaded
{
}


/*virtual*/ GlyphFaceCacheOneItem::~GlyphFaceCacheOneItem()
{
    if (m_LoadedGlyphNo==-1)   //-1 means none loaded
        return;
    
    delete glyphDirect();       //invoke destructor
}


/*virtual*/ EGlyphCacheStrategy GlyphFaceCacheOneItem::getEnum() const
{
    return eOneCache;
}


/*virtual*/ const GlyphFace *GlyphFaceCacheOneItem::glyph(unsigned short glyphid) const      //result may be changed by subsequent call with a different glyphid
{
    incAccesses();
    if (m_LoadedGlyphNo==glyphid)
        return glyphDirect();
    
    incLoads();
    
    if (m_LoadedGlyphNo!=-1)   //-1 means none loaded
        delete glyphDirect();       //invoke destructor
        
    m_LoadedGlyphNo = glyphid;
    new(glyphDirect()) GlyphFace(*this, glyphid);
    
    return glyphDirect();
}



GlyphFaceCacheLoadedOnDemand::GlyphFaceCacheLoadedOnDemand(const GlyphFaceCacheHeader& hdr)
:   GlyphFaceCache(hdr)
{
    unsigned int nGlyphs = numGlyphs();
    
    for (unsigned int i = 0; i < nGlyphs; i++)
    {
         *glyphPtrDirect(i) = NULL;
    }
}

/*virtual*/ GlyphFaceCacheLoadedOnDemand::~GlyphFaceCacheLoadedOnDemand()
{
//    delete[] m_glyphs;        //can't do this since not allocated by new[] and so does not know array size.
    unsigned int nGlyphs = numGlyphs();
    for (unsigned int i=0 ; i<nGlyphs; ++i)
    {
        GlyphFace *p = *glyphPtrDirect(i);
        if (p)
        {
            delete p;      //invokes d'tor. Does not release the memory.
            free(p);
        }
    }
}


/*virtual*/ EGlyphCacheStrategy GlyphFaceCacheLoadedOnDemand::getEnum() const
{
    return eLoadOnDemand;
}


/*virtual*/ const GlyphFace *GlyphFaceCacheLoadedOnDemand::glyph(unsigned short glyphid) const      //result may be changed by subsequent call with a different glyphid
{ 
    incAccesses();
    GlyphFace **p = glyphPtrDirect(glyphid);
    if (*p)
        return *p;

    incLoads();
    *p = (GlyphFace*)malloc(sizeof(GlyphFace));
    new(*p) GlyphFace(*this, glyphid);
    return *p;
}



GlyphFaceCachePreloaded::GlyphFaceCachePreloaded(const GlyphFaceCacheHeader& hdr)
:   GlyphFaceCache(hdr)
{
    unsigned int nGlyphs = numGlyphs();
    
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementGlyphs);
        XmlTraceLog::get().addAttribute(AttrNum, nGlyphs);
    }
#endif
    for (unsigned int i = 0; i < nGlyphs; i++)
    {
        incLoads();
        new(glyphDirect(i)) GlyphFace(*this, i);
    }
#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementGlyphs);
#endif
}

/*virtual*/ GlyphFaceCachePreloaded::~GlyphFaceCachePreloaded()
{
//    delete[] m_glyphs;        //can't do this since not allocated by new[] and so does not know array size.
    unsigned int nGlyphs = numGlyphs();
    for (unsigned int i=0 ; i<nGlyphs; ++i)
        delete glyphDirect(i);      //invokes d'tor. Does not release the memory.
}


/*virtual*/ EGlyphCacheStrategy GlyphFaceCachePreloaded::getEnum() const
{
    return ePreload;
}


/*virtual*/ const GlyphFace *GlyphFaceCachePreloaded::glyph(unsigned short glyphid) const      //result may be changed by subsequent call with a different glyphid
{ 
    incAccesses();
    return glyphDirect(glyphid); 
}

