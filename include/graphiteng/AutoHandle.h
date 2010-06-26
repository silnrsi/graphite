#ifndef AUTO_HANDLE_INCLUDE
#define AUTO_HANDLE_INCLUDE

/* In contrast to AutoHandle which has multiple owners of the same object via a count. AutoHandle has a single owner for each object.
   When one AutoHandle is copied from another, the original AutoHandle loses ownership.
*/

template <class OBJCLASS, void (*pDELETEFN)(OBJCLASS *p)>
class GRNG_EXPORT AutoHandle
{
public:
    AutoHandle() : m_p(NULL) {}
    AutoHandle(OBJCLASS* p/*takes ownership*/) : m_p(p) {}
    AutoHandle(const AutoHandle<OBJCLASS, pDELETEFN>& src) : m_p(src.m_p) { src.m_p=NULL; }
    AutoHandle<OBJCLASS, pDELETEFN>& operator=(const AutoHandle<OBJCLASS, pDELETEFN>& src) { if (&src!=this) { Release(); m_p=src.m_p; src.m_p=NULL; } return *this; }
    ~AutoHandle() { Release();}

    OBJCLASS* operator->() const { return m_p; }		//cannot be used by client code - only available witin graphite code!
//    OBJCLASS& operator*() const {return *m_p; };

    bool IsNull() const { return m_p==NULL; }

protected:
    OBJCLASS* Ptr() const { return m_p; }
    void SetPtr(OBJCLASS* p/*takes ownership*/=NULL) { Release(); m_p=p; }

private:
    void Release() { if (m_p) (*pDELETEFN)(m_p); }		//the 'if' is not really necessary but is an optimization which will often help

private:
    mutable OBJCLASS* m_p;
};

#endif // !AUTO_HANDLE_INCLUDE
