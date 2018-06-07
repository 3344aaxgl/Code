#include <iostream>
#include <list>
#include <memory>

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
    static std::shared_ptr<ThePrinters> makePrinters();
    static std::shared_ptr<ThePrinters> makePrinters(const ThePrinters &rhs);
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
int Counted<ThePrinters>::numObjects = 10;

std::shared_ptr<ThePrinters> ThePrinters::makePrinters()
{
    return std::shared_ptr<ThePrinters>(new ThePrinters());
}

std::shared_ptr<ThePrinters> ThePrinters::makePrinters(const ThePrinters& p)
{
    return std::shared_ptr<ThePrinters>(new ThePrinters(p));
}

int main()
{
    NewsLetter nl(cin);

    return 0;
}