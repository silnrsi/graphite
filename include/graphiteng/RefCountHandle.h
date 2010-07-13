#pragma once

#include <cstdlib>

namespace org { namespace sil { namespace graphite { namespace v2 {

template <class OBJCLASS, void (*pDELETEFN)(OBJCLASS *p)>
class GRNG_EXPORT RefCountHandle
{
public:
    RefCountHandle(OBJCLASS* p/*takes ownership*/=NULL) //: m_pCount(new unsigned int(1)), m_p(p) {}
        : m_pCount(reinterpret_cast<unsigned int*>(malloc(sizeof(unsigned int)))), m_p(p) {}
    RefCountHandle(const RefCountHandle<OBJCLASS, pDELETEFN>& src) : m_pCount(src.m_pCount), m_p(src.m_p) { addRef(); }
    RefCountHandle<OBJCLASS, pDELETEFN>& operator=(const RefCountHandle<OBJCLASS, pDELETEFN>& src) { src.addRef(); release(); m_pCount=src.m_pCount; m_p=src.m_p; return *this; }
    ~RefCountHandle() { release();}

    OBJCLASS* operator->() const { return m_p; }		//cannot be used by client code - only available witin graphite code!
//    OBJCLASS& operator*() const {return *m_p; };

protected:
    OBJCLASS* ptr() const { return m_p; }
    void setPtr(OBJCLASS* p/*takes ownership*/=NULL)// { release(); m_pCount=new unsigned int(1); m_p=p; }
    { release(); m_pCount = reinterpret_cast<unsigned int*>(malloc(sizeof(unsigned int))); m_p=p; }

private:
    void addRef() const { ++*m_pCount; }
    void release() //{ if (--*m_pCount==0) { delete m_pCount; (*pDELETEFN)(m_p); } }
    { if (--*m_pCount==0) { free(m_pCount); (*pDELETEFN)(m_p); } }

private:
    mutable unsigned int* m_pCount;	//counts how many times shared. Never 0
    OBJCLASS* m_p;
};

}}}}
