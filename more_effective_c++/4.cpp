#include<iostream>

using namespace std;

class Rational
{
public:
  Rational(int num = 0,int denom = 1):numerator(num),denominator(denom){};
  operator double()
  {
      return numerator * denominator;
  }
  double asDouble()
  {
      return numerator * denominator;
  }
private:
  int numerator;
  int denominator;
};

template <typename T>
class Array
{
    public:
     explicit Array(int si):size(si)//加上explicit无法通过单参函数进行隐式转换
      {
          data = new T(size);
      };

     T& operator[](int i)const
      {
          return data[i];
      }

      ~Array()
      {
          delete [] data;
      } 
    private:
      int size;
      T *data;
};

bool operator==(const Array<int> &a,const Array<int> &b)
{
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == b[i]);
        else
          return false;
    }
    return true;
}

template<typename T>
class vector
{
    public:
      class vectorsize
      {
          public:
            vectorsize(int si):size(si){}
          private:
            int size;
      };
      vector(vectorsize v):vs(v)
      {
          data = new T(vs);
      }
      T& operator[](int i)const
      {
          return data[i];
      }

      ~vector()
      {
          delete [] data;
      } 
    private:
      T* data;
      vectorsize vs;

};

bool operator==(const vector<int> &a,const vector<int> &b)
{
    for (int i = 0; i < 10; i++)
    {
        if (a[i] == b[i]);       //int可以隐式转换成vectorsize，但vectorsize不会再隐式转换成vector
        else
          return false;
    }
    return true;
}

class UPint
{
    private:
      int i;
    public:
      UPint& operator++()
      {
         i += 1;
         return (*this);
      };
      const UPint operator++(int)     //返回的是对象，效率低。返回const对象，防止出现i++++，没有加两次，而且与int的行为不符
      {
          UPint oldvalue(*this);
          this->i += 1;
          return oldvalue;
      }
};

struct A {} ;
struct E {} ;

class alloc
{
   private:
     int data;
   public:
     alloc(int i = 0):data(i){throw E();};
     //重载operator new
     void* operator new(size_t size)
     {
        cout<<"called operator new\n";
        return ::operator new(size);      //调用全局operator new
     }

     //重载operator delete
     void operator delete(void * pointer)
     {
        cout<<"called operator delete\n";
        ::operator delete(pointer);
     }

     //重载operator new[]
     void* operator new[](size_t size)
     {
         cout<<"called operator new[]\n";
         ::operator new[](size);
     }

    //重载operator delete[]
     void operator delete[](void * pointer)
     {
        cout<<"called operator delete[]\n";
        ::operator delete[](pointer);
     }

     //placement new
     void* operator new(size_t size,void* pointer)
     {
         cout<<"called placement new\n";
         return ::operator new(size,pointer);
     }

     //placement delete
    void operator delete(void*memory,void* pointer)
     {
         cout<<"called placement delete\n";
         return ::operator delete(memory,pointer);
     } 

};



class T 
{
public:
    T() 
    { 
        //throw E() ; 
    }
} ;

void * operator new ( std::size_t, const A & )
    {
        void* nothing = 0;
        std::cout << "Placement new called." << std::endl;
        return nothing;
    }
void operator delete ( void *, const A & )
    {
        std::cout << "Placement delete called." << std::endl;
    }



int main()
{
/*     Rational r1(1,2);
    cout<< r1;            //Rational没有重载<<运算符，但是可以隐式转换成double
    cout<< r1.asDouble();

    Array<int> a(10);  //正确，显示调用
    Array<int> b(10);

   // if(a == b);     //错误，无法将int隐式转换成Array
   UPint ui;
   ui++;
   ++ui;

   A c ;
    //try {
        T * p = new (c) T ;
   // } catch (E exp) {std::cout << "Exception caught." << std::endl;}
    cout<<&(*p)<<" "<<&a<<endl; */

/*     alloc*p = new alloc(1);
    delete p;

    alloc* q = new alloc[5];
    delete []q; */

    char *p = new char[sizeof(alloc)];
    try
    {
    alloc *q = new(p) alloc(1);
    }
    catch(E e)
    {
        cout << "Exception caught." << std::endl;
    }



    return 0 ;
}