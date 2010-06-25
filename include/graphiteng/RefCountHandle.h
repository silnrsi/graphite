#ifndef REF_COUNT_HANDLE_INCLUDE
#define REF_COUNT_HANDLE_INCLUDE

class RefCountHandle
{
public:
    RefCountHandle() : m_pCount(new unsigned int(1)) {}
    RefCountHandle(const RefCountHandle&src) : m_pCount(src.m_pCount) { AddRef(); }
    bool operator=(const RefCountHandle& src) { src.AddRef(); bool res=AboutToFinish(); Release(); m_pCount=src.m_pCount; return res; }  //return value indicates if this's object should be deleted
    ~RefCountHandle() { Release();}
    
    bool AboutToFinish() const { return *m_pCount==1 ; }
    
private:
    void AddRef() const { ++*m_pCount; }
    void Release() const { if (--*m_pCount==0) delete m_pCount; }

private:
    mutable unsigned int* m_pCount;	//counts how many times shared. Never 0
};

#endif // !REF_COUNT_HANDLE_INCLUDE
