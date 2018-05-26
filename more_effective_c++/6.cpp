#include <iostream>
using namespace std;

int Countchar(const string& str)
{
    cout<<"const string&"<<endl;
    return str.length();
}

int Count(string str)     //重载冲突
{
    cout<<" string"<<endl;
    return str.length();
}

int Countsize(string& str)     //无法进行隐式转换
{
    cout<<" string"<<endl;
    return str.length();
}

class A
{
public:
  A(int i):data(i)
  {
      cout<<"called constructor"<<endl;
  }

  A(const A& a)
  {
      data = a.data;
      cout<<"called copy constructor"<<endl;
  }

  ~A()
  {
      cout<<"called deconstructor"<<endl;
  }
public:
  int data;
};

A RVOTest(int i)
{
    return A(i);
}

A NRVOTest(int i)
{
    A a(i);
    return a;
}

class UPInt
{
public:
  UPInt(int value):data(value)
  {

  }
  friend const UPInt operator+(const UPInt& lhs, const UPInt& rhs);
  friend const UPInt operator+(const UPInt& lhs, int rhs);
  friend const UPInt operator+(int lhs, const UPInt& rhs);
  friend ostream& operator<<(ostream&, const UPInt& rhs);
private:
  int data;
};

const UPInt operator+(const UPInt& lhs, const UPInt& rhs)
{
   return UPInt(lhs.data + rhs.data);
}

const UPInt operator+(const UPInt& lhs, int rhs)
{
   return UPInt(lhs.data + rhs);
}

const UPInt operator+(int lhs, const UPInt& rhs)
{
   return UPInt(lhs + rhs.data);
}

ostream& operator<<(ostream& os, const UPInt& rhs)
{
    os << rhs.data;
    return os;
}

int main()
{
    char arr[] = {"hello world"};
    cout<<Countchar(arr)<<endl;
    cout<<Count(arr)<<endl;
    cout<<Count(arr)<<endl;
    //cout<<Countsize(arr)<<endl;

    A a = RVOTest(1);
    A b = NRVOTest(2);

    UPInt u1(1);
    UPInt u2(2);

    cout<<u1 + u2<<" "<<u1+1<<" "<<2+u2<<endl;
    return 0;
}