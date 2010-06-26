#ifndef REF_COUNT_HANDLE_INCLUDE
#define REF_COUNT_HANDLE_INCLUDE

template <class OBJCLASS, void (*pDELETEFN)(OBJCLASS *p)>
class RefCountHandle
{
public:
    RefCountHandle(OBJCLASS* p/*takes ownership*/=NULL) : m_pCount(new unsigned int(1)), m_p(p) {}
    RefCountHandle(const RefCountHandle<OBJCLASS, pDELETEFN>& src) : m_pCount(src.m_pCount), m_p(src.m_p) { AddRef(); }
    RefCountHandle<OBJCLASS, pDELETEFN>& operator=(const RefCountHandle<OBJCLASS, pDELETEFN>& src) { src.AddRef(); Release(); m_pCount=src.m_pCount; m_p=src.m_p; return *this; }
    ~RefCountHandle() { Release();}

    OBJCLASS* operator->() const { return m_p; }		//cannot be used by client code - only available witin graphite code!
    OBJCLASS& operator*() const {return *m_p; };

protected:
    OBJCLASS* Ptr() const { return m_p; }
    void SetPtr(OBJCLASS* p/*takes ownership*/=NULL) { Release(); m_pCount=new unsigned int(1); m_p=p; }

private:
    void AddRef() const { ++*m_pCount; }
    void Release() const { if (--*m_pCount==0) { delete m_pCount; (*pDELETEFN)(m_p); } }

private:
    mutable unsigned int* m_pCount;	//counts how many times shared. Never 0
    OBJCLASS* m_p;
};

#endif // !REF_COUNT_HANDLE_INCLUDE
