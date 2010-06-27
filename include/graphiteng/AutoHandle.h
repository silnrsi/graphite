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
    AutoHandle<OBJCLASS, pDELETEFN>& operator=(const AutoHandle<OBJCLASS, pDELETEFN>& src) { if (&src!=this) { release(); m_p=src.m_p; src.m_p=NULL; } return *this; }
    ~AutoHandle() { release();}

    OBJCLASS* operator->() const { return m_p; }		//cannot be used by client code - only available witin graphite code!
//    OBJCLASS& operator*() const {return *m_p; };

    bool isNull() const { return m_p==NULL; }

protected:
    OBJCLASS* ptr() const { return m_p; }
    void setPtr(OBJCLASS* p/*takes ownership*/=NULL) { release(); m_p=p; }

private:
    void release() { if (m_p) (*pDELETEFN)(m_p); }		//the 'if' is not really necessary but is an optimization which will often help

private:
    mutable OBJCLASS* m_p;
};

#endif // !AUTO_HANDLE_INCLUDE
