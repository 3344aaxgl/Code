#include <iostream>
#include <memory>


class string
{
  private:
    int size;
    std::shared_ptr<char> data;

  public:
    string(int i):size(i),data(std::shared_ptr<char>(new char[i]))
    {
    }

};

struct A
{

};

void funa()
{
    try
    {
        A a;
        std::cout<<"the address of a:"<<&a<<std::endl;
       throw a;
    }
    catch(A& a)
    {
        std::cout<<"the address of catched a:"<<&a<<std::endl;//虽然传递的是引用，但仍然进行了复制
    }
}

void funb()
{
    try
    {
        A a;
        std::cout<<"the address of a:"<<&a<<std::endl;
       throw a;
    }
    catch(A a)
    {
        std::cout<<"the address of catched a:"<<&a<<std::endl;//传值进行了复制
    }
}

void func()
{
    try
    {
        static A a;
        std::cout<<"the address of a:"<<&a<<std::endl;
       throw a;
    }
    catch(A& a)
    {
        std::cout<<"the address of catched a:"<<&a<<std::endl;//即使对象在try之后仍然存在，还是进行了复制
    }
}

class Base
{
  public:
    virtual void print()
    {
        std::cout << "Base" << std::endl;
    }
};

class Derived : public Base
{
  public:
    virtual void print()
    {
        std::cout << "Derived" << std::endl;
    }
};

void funf()
{
    try
    {
        try
        {
            Derived d;
            Base &b = d;
            throw d;
        }
        catch (Base &b)
        {
            b.print();//Derived
            throw;    //将异常原样抛出，仍为Derived
        }
    }
    catch (Base &b)
    {
        b.print();//Derived
    }
}

void fund()
{
    try
    {
        try
        {
            Derived d;
            Base &b = d;
            throw d;
        }
        catch (Base &b)
        {
            b.print();//Derived
            throw b;  //抛出的为Base，进行拷贝之后变为Base类型
        }
    }
    catch (Base &b)
    {
        b.print();//Base
    }
}

void fune()
{
    try
    {
        try
        {
            Derived d;
            Base &b = d;
            throw b;   //抛出的对象为Base，即使b引用的是一个SpecialWidget
        }
        catch (Base &b)
        {
            b.print();//Base
            throw;
        }
    }
    catch (Base &b)
    {
        b.print(); //Base
    }
}

void fung()
{
    try
    {
        Derived d;
        throw d;
    }
    catch (Base b) //接收的的类型为Base
    {
        b.print(); //Base
    }
}

void funh()
{
    int i = 0;
    throw i;
}

void funi() throw(double)
{
    funh();
}

void funj()
{
    try
    {
        funi();
    }
    catch(double& i)
    {
        std::cout<<"catched"<<std::endl;
    }
}

int main()
{
    string str(16);
    funa();
    funb();
    func();
    std::cout<<"----------------------------"<<std::endl;
    fund();
    fune();
    funf();
    fung();
    funj();
}

