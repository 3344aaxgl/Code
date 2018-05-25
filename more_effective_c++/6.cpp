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

int main()
{
    char arr[] = {"hello world"};
    cout<<Countchar(arr)<<endl;
    cout<<Count(arr)<<endl;
    cout<<Count(arr)<<endl;
    //cout<<Countsize(arr)<<endl;

    A a = RVOTest(1);
    A b = NRVOTest(2);
    return 0;
}