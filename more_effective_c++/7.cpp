#include <iostream>
#include <list>
#include <memory>
#include <algorithm>
#include <string.h>

using namespace std;

class NLcomponent
{
public:
  virtual ~NLcomponent(){};
  virtual NLcomponent* clone(){} ;
  virtual ostream& print(ostream& s)const { return s;};
};

class TextBlock:public NLcomponent
{

public:
  virtual ~TextBlock(){};
  virtual TextBlock* clone()
  {
      return new TextBlock(*this);
  };
  virtual ostream& print(ostream& s) const{ return s;};
};

class Graphic:public NLcomponent
{
public:
  virtual ~Graphic(){};
  virtual Graphic* clone()
  {
      return new Graphic(*this);
  }
  virtual ostream& print(ostream& s) const{ return s;};
};

class NewsLetter
{
private:
    list<NLcomponent *> component;
    NLcomponent * readComponent(istream& str)
    {
        char a;
        while (str >> a)
        {
            if (a >= '0' && a <= '9')//依据不同输入构造不同的对象，此处无法使用智能指针
                return (new TextBlock());
            else
                return (new Graphic());
        }
        return new NLcomponent();
    }
public:
    NewsLetter(istream& str);
    ~NewsLetter()
    {
        for(list<NLcomponent*>::iterator it = component.begin();it != component.end(); it++)
        {
            delete (*it);//析构函数为虚拟函数，所以会调用对应的析构函数
        }
    }
    NewsLetter(NewsLetter& rhs)
    {
        for(list<NLcomponent*>::iterator it = rhs.component.begin();it != rhs.component.end(); it++)
        {
            component.push_back((*it)->clone());
        }
    }
    
};

NewsLetter::NewsLetter(istream& str)
{
    while(str)
    {
        component.push_back(readComponent(str));//将指针存入list
    }
}

inline ostream& operator<<(ostream& s, const NLcomponent& c) 
{
  return c.print(s);
}

class Printer
{
private:
    Printer(){};
    Printer(const Printer& rhs);
public:
    friend Printer& thePrinters();
};

Printer& thePrinters()//友元函数，可以调用私有成员
{
    static Printer p;//静态成员，只会初始化一次
    return p;
}

class ThePrinter
{
private:
    ThePrinter();
    ThePrinter(const ThePrinter&);
    class TooManyObjects{};
    static size_t numObjects;
    const static size_t maxObjects = 10;
public:
    static std::shared_ptr<ThePrinter> makeThePrinter();
    static std::shared_ptr<ThePrinter> makeThePrinter(const ThePrinter& );
};

size_t ThePrinter::numObjects = 0;
const size_t ThePrinter::maxObjects;

ThePrinter::ThePrinter()
{
    if (numObjects >= maxObjects)
    {
        throw TooManyObjects();
    }
}

ThePrinter::ThePrinter(const ThePrinter&)
{
    if (numObjects >= maxObjects)
    {
        throw TooManyObjects();
    }
}

std::shared_ptr<ThePrinter> ThePrinter::makeThePrinter()
{
    return std::shared_ptr<ThePrinter>(new ThePrinter());
}

std::shared_ptr<ThePrinter> ThePrinter::makeThePrinter(const ThePrinter& p)
{
    return std::shared_ptr<ThePrinter>(new ThePrinter(p));
}

//具有计数功能的基类
template<typename T>
class Counted
{
public:
    class TooManyObjects{};
    static int objectCount(){return numObjects;};
protected:
    Counted();
    Counted(const Counted&);
    ~Counted(){--numObjects;};
private:
    void init();
    static int numObjects;
    const static size_t maxObjects ;
};

template<typename T>
Counted<T>::Counted()
{
    init();
}

template<typename T>
Counted<T>::Counted(const Counted<T>&)
{
    init();
}

template<typename T>
void Counted<T>::init()
{
    if(numObjects > maxObjects)
      throw TooManyObjects();
    ++ numObjects;
}

class ThePrinters : private Counted<ThePrinters>
{
public:
    // 伪构造函数
    static std::shared_ptr<ThePrinters> makeThePrinters();
    static std::shared_ptr<ThePrinters> makeThePrinters(const ThePrinters &rhs);
    ~ThePrinters(){};
    void submitJob(const ThePrinters &job);
    void reset();
    void performSelfTest();
    using Counted<ThePrinters>::objectCount; // 参见下面解释
    using Counted<ThePrinters>::TooManyObjects;  // 参见下面解释
  private:
    ThePrinters(){};
    ThePrinters(const ThePrinters &rhs){};
};

template<>
const size_t Counted<ThePrinters>::maxObjects = 10;

template<>
int Counted<ThePrinters>::numObjects = 0;

std::shared_ptr<ThePrinters> ThePrinters::makeThePrinters()
{
    return std::shared_ptr<ThePrinters>(new ThePrinters());
}

std::shared_ptr<ThePrinters> ThePrinters::makeThePrinters(const ThePrinters& p)
{
    return std::shared_ptr<ThePrinters>(new ThePrinters(p));
}

class UPNumber
{
public:
    UPNumber(){};
    UPNumber(const UPNumber&){};
    void destroy()const{delete this;};
private:
    ~UPNumber(){}          //析构函数私有，保证无法将对象分配在栈上
};

class UPNumber1
{
public:
    UPNumber1(){};
    UPNumber1(const UPNumber1&){};
    void destroy()const{delete this;};
protected:
    ~UPNumber1(){}          //析构函数保护，保证无法将对象分配在栈上，同时继承类也可以实现xiantogn
};

class NonNegativeUPNumber:public UPNumber1
{

};

class Asset
{
public:
    Asset();
    ~Asset();
private:
    UPNumber *value;
};
Asset::Asset():value(new UPNumber()) // 正确
{

}
Asset::~Asset()
{
    value->destroy();
}

class UPNumber2
{
public:
    class HeapConstraintViolation {};
    static void * operator new(size_t size);
    UPNumber2();
private:
    static bool onTheHeap;
};

bool UPNumber2::onTheHeap = false;

void* UPNumber2::operator new(size_t size)
{
    onTheHeap = true;

    return ::operator new(size);
}

UPNumber2::UPNumber2()
{
    if(!onTheHeap)
      throw HeapConstraintViolation();
    else
      onTheHeap = false;
}

class HeapTracked
{
public:
    class MissingAddress{};
    virtual ~HeapTracked() = 0;
    static void* operator new(size_t size);
    static void operator delete(void* ptr);
    bool IsOnHeap() const;
private:
    typedef const void* RawAddress;
    static std::list<RawAddress> address;  
};

HeapTracked::~HeapTracked(){}

void* HeapTracked::operator new(size_t size)
{
    void *ptr = ::operator new(size);
    address.push_back(ptr);
    return ptr;
}

void HeapTracked::operator delete(void* ptr)
{
    list<RawAddress>::iterator it = find(address.begin(),address.end(),ptr);
    if(it != address.end())
    {
        address.erase(it);
        ::operator delete(ptr);
    }
    else
    {
        throw MissingAddress();
    }
}

bool HeapTracked::IsOnHeap() const
{
    //指向或引用对象的最终导出类的指针
    const void *rawAddress = dynamic_cast<const void *>(this);
    // 在operator new返回的地址list中查到指针
    list<RawAddress>::iterator it =
        find(address.begin(), address.end(), rawAddress);
    return it != address.end(); // 返回it是否被找到
}

list<HeapTracked::RawAddress> HeapTracked::address;

class Assert:public HeapTracked
{

    
};


template<typename T>
class my_auto_ptr
{
private:
    T* pointee;
public:
    my_auto_ptr(T* p = 0):pointee(p){}

    my_auto_ptr(my_auto_ptr&& mp):pointee(mp.pointee)//不能传入const，会对原有的对象进行更改,这里使用折叠引用
    {
        mp.pointee = 0;
    }

    my_auto_ptr& operator=(my_auto_ptr& mp)//不能传const，理由同上
    {
        if(this == &mp)
           return *this;
        delete pointee;
        pointee = mp.pointee;
        mp.pointee = 0;
        return *this;
    }

    T& operator*() const//这里返回引用是因为pointee可能是指向继承类的指针，如果返回对象，则可能导致问题
    {
        return *pointee;
    }

    T* operator->() const
    {
        return pointee;
    }

    operator void*()
    {
        return static_cast<void*>(pointee);
    }

    operator !()
    {
        return (pointee == nullptr);
    }

/*     operator T *()
    {
        return pointee;
    } */

    template<typename Other>
    operator my_auto_ptr<Other>()
    {
        //通过调用另一个模板类的构造函数来实现隐式转换
        return my_auto_ptr<Other>(pointee);
    }

    ~my_auto_ptr()
    {
        delete pointee;
    }
};

template<typename T>
void printTreeNode(ostream &s, my_auto_ptr<T>& p)//只能传引用，因为拷贝构造函数会对对象进行改动
{
    s << *p;
}

/* class TupleAccessors {
public:
TupleAccessors(const int *pt);

};

TupleAccessors merged(const TupleAccessors& ta1,const TupleAccessors& ta2)
{

} */

class Base
{
public:
    Base()
    {
        cout<<"Base constructor"<<'\n';
    }
};

class Derived:public Base
{
public:
    Derived()
    {

    }
    Derived(const Derived& d)
    {

    }
};

class DerivedDerived:public Derived
{

};

void displayAndPlay(const Base* b)
{
    cout<<"Base"<<endl;
}

void displayAndPlay(const Derived*)
{
    cout<<"derived"<<endl;
}

void displayAndPlay(const my_auto_ptr<Base>& mb)
{

}

void displayAndPlay(const my_auto_ptr<Derived>& md)
{

}

template <class T> // 指向const对象的
class SmartPtrToConst
{ // 灵巧指针
    // 成员函数
  protected:
    union 
    {
        const T *constPointee; // 让 SmartPtrToConst 访问
        T *pointee;            // 让 SmartPtr 访问
    };
};
template <class T> // 指向non-const对象
class SmartPtr : public SmartPtrToConst<T>
{
public:
    SmartPtr(T* p){SmartPtrToConst<T>::pointee = p;}
};

class my_string
{
private:
    struct stringvalue
    {
        char* data;
        int refCount;
        stringvalue(const char* initvalue = "");
        bool shareable;
        ~stringvalue();
    };
    stringvalue* value;
public:
    my_string(const char* initvalue = ""):value(new stringvalue(initvalue))
    {
        ++value->refCount;
        value->shareable = true;
    }
    
    ~my_string()
    {
        if (--value->refCount == 0)
            delete value;
    }

    my_string(const my_string &ms)
    {
        if (!ms.value->shareable)
            value = ms.value;
        else
            value = new stringvalue(ms.value->data);
        ++value->refCount;
    }

    my_string& operator=(const my_string& ms)
    {
        if(this == &ms)
            return *this;
        if(--value->refCount == 0)
            delete value;
        if(ms.value->shareable)
           value = ms.value;
        else
           value = new stringvalue(ms.value->data);
        ++value->refCount;
        return *this;
    }

    const char& operator[](int index) const
    {
        return value->data[index];
    }

    char& operator[](int index)
    {
        if(value->refCount > 1)
        {
            --value->refCount;
            value = new stringvalue(value->data);
        }
        value->shareable = false;
        return value->data[index];
    }
}; 

my_string::stringvalue::stringvalue(const char* initvalue)
{
    data = new char[strlen(initvalue)+1];
    strcpy(data,initvalue);
}

my_string::stringvalue::~stringvalue()
{
    delete [] data;
}

class RCObject
{
  private:
    int refCOunt;
    bool shareable;

  public:
    RCObject() : refCOunt(0), shareable(true) {}

    virtual ~RCObject() = 0;

    void addreference()
    {
        ++refCOunt;
    }

    void removereference()
    {
        if(--refCOunt == 0)
          delete this;
    }

    void markUnshareable()
    {
        shareable = false;
    }

    bool Isshareable()
    {
        return shareable;
    }

    bool Isshared()
    {
        return refCOunt > 1;
    }
};

RCObject::~RCObject()
{

}

template <typename T>
class RCPtr
{
  private:
    T *pointee;
    void init();
  public:
    RCPtr(T *realPtr = 0):pointee(realPtr)
    {
        init();
    }
    RCPtr(const RCPtr &rhs):pointee(rhs.pointee)
    {
        init();
    }
    virtual ~RCPtr()
    {
        pointee->removereference();
    }
    RCPtr &operator=(const RCPtr &rhs)
    {
        if(pointee != rhs.pointee)
        {
            if(pointee)
              pointee->removereference();
            pointee = rhs.pointee;
            init();
        }
        return *this;
    }
    T *operator->() const
    {
        return pointee;
    }
    T &operator*() const
    {
        return *pointee;
    }  
};

template<typename T>
void RCPtr<T>::init()
{
    if(pointee == 0)
      return ;
    if(pointee->Isshareable() == false)
      pointee = new T(*pointee);
    pointee->addreference();
}

class my_RCstring
{
  private:
    struct stringvalue : public RCObject
    {   
        char* data;
        stringvalue(const char *initvalue = "")
        {
            data = new char[strlen(initvalue) +1];
            strcpy(data,initvalue);
        }

        stringvalue(const stringvalue& str)
        {
            data = new char[strlen(str.data) +1];
            strcpy(data,str.data);
        }

        ~stringvalue()
        {
            delete [] data;
        }
    };
    RCPtr<stringvalue> value;
  public:
    my_RCstring(const char* initvalue):value(new stringvalue(initvalue))
    {

    }

    const char & operator[](int index) const
    {
        return value->data[index];
    }
    char & operator[](int index)
    {
        if (value->Isshared())
        {
            value = new stringvalue(value->data);
        }
        value->markUnshareable();
        return value->data[index];
    }
};

template <class T>
class RCIPtr
{
  public:
    RCIPtr(T *realPtr = 0);
    RCIPtr(const RCIPtr &rhs);
    ~RCIPtr();
    RCIPtr &operator=(const RCIPtr &rhs);
    const T *operator->() const; // see below for an
    T *operator->();             // explanation of why
    const T &operator*() const;  // these functions are
    T &operator*();              // declared this way
  private:
    struct CountHolder : public RCObject
    {
        ~CountHolder() { delete pointee; }
        T *pointee;
    };
    CountHolder *counter;
    void init();
    void makeCopy(); // see below
};
template <class T>
void RCIPtr<T>::init()
{
    if (counter->isShareable() == false)
    {
        T *oldValue = counter->pointee;
        counter = new CountHolder;
        counter->pointee = new T(*oldValue);
    }
    counter->addReference();
}
template <class T>
RCIPtr<T>::RCIPtr(T *realPtr)
    : counter(new CountHolder)
{
    counter->pointee = realPtr;
    init();
}
template <class T>
RCIPtr<T>::RCIPtr(const RCIPtr &rhs)
    : counter(rhs.counter)
{
    init();
}
template <class T>
RCIPtr<T>::~RCIPtr()
{
    counter->removeReference();
}
template <class T>
RCIPtr<T> &RCIPtr<T>::operator=(const RCIPtr &rhs)
{
    if (counter != rhs.counter)
    {
        counter->removeReference();
        counter = rhs.counter;
        init();
    }
    return *this;
}
template <class T>         // implement the copy
void RCIPtr<T>::makeCopy() // part of copy-on-
{                          // write (COW)
    if (counter->isShared())
    {
        T *oldValue = counter->pointee;
        counter->removeReference();
        counter = new CountHolder;
        counter->pointee = new T(*oldValue);
        counter->addReference();
    }
}
template <class T>                     // const access;
const T *RCIPtr<T>::operator->() const // no COW needed
{
    return counter->pointee;
}
template <class T>         // non-const
T *RCIPtr<T>::operator->() // access; COW
{
    makeCopy();
    return counter->pointee;
} // needed
template <class T>                    // const access;
const T &RCIPtr<T>::operator*() const // no COW needed
{
    return *(counter->pointee);
}
template <class T>        // non-const
T &RCIPtr<T>::operator*() // access; do the
{
    makeCopy();
    return *(counter->pointee);
} // COW thing

class Widget
{
  public:
    Widget(int size);
    Widget(const Widget &rhs);
    ~Widget();
    Widget &operator=(const Widget &rhs);
    void doThis();
    int showThat() const;
};

class RCWidget
{
  public:
    RCWidget(int size) : value(new Widget(size)) {}
    void doThis() { value->doThis(); }
    int showThat() const { return value->showThat(); }

  private:
    RCIPtr<Widget> value;
};

int main()
{
    //NewsLetter nl(cin);

    std::list<std::shared_ptr<ThePrinters>> l(11);

    for(int i = 0; i< 11; i++)
    {
         l.push_back(ThePrinters::makeThePrinters());
    }
    cout<<ThePrinters::Counted<ThePrinters>::objectCount()<<'\n';

    UPNumber *p = new UPNumber();
    p->destroy();

    NonNegativeUPNumber q;

    Assert* as = new Assert();
    cout<<as->IsOnHeap()<<'\n';
    delete as;

    my_auto_ptr<int> mp ;
    if(!mp)
      cout<<"mp is nullptr"<<endl;

    if(mp);

    if(!mp);

    my_auto_ptr<double> md;
    if(mp == md);

    if(!mp == !md); 

/*     my_auto_ptr<int> mi;

    merged(mp,mi);  */ 

    my_auto_ptr<Derived> mpd;

    //这里经过RVO，只会调用默认构造函数，但在编译的时候会检查是否符合拷贝构造函数，
    //如果拷贝构造函数入参是左引用，但实际传入的是个右值，则不匹配。所以利用引用折叠，将入参改为右值引用
    //这样既可以支持左值，也可以支持右值

    my_auto_ptr<Base> mb = mpd;

    my_auto_ptr<DerivedDerived> mdd;
   // displayAndPlay(mdd);

    DerivedDerived* dd;

    displayAndPlay(dd);

    Base pb;

    my_auto_ptr<Base> mp1 ;                        //non-const对象，non-const指针
    my_auto_ptr<const Base> mp2;                   //const对象，non-const指针
    const my_auto_ptr<Base> mp3 = &pb;             //non-const对象，const指针
    const my_auto_ptr<const Base> mp4 = &pb;       //const对象，const指针

    SmartPtr<int> pCD = new int();
    SmartPtrToConst<int> pConstCD = pCD; // 正确

    my_string ms("hello world"),ms1;
    ms1 = ms;
    

    return 0;
}