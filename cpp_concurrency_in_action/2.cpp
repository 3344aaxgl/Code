#include <iostream>
#include <thread>

using namespace std;

int do_something()
{
    cout<<"called function\n";
    return 0;
}

auto la = [](){cout<<"called lambda\n";};

struct do_otherthig
{
    void operator()()
    {
        cout<<"called functor\n";
    }
};

auto lb = [](){cout<<"called std::function\n";};

struct func 
{
  int& i;
  func(int& i_) : i(i_) {}
  void operator() ()
  {
    for (unsigned j=0 ; j<1000000 ; ++j)
    {
      do_something1(i);// 1. 潜在访问隐患:悬空引用
    }
  }

  void do_something1(int &i)
  {

  }   
};


void oops() 
{
  int some_local_state=0;
  func my_func(some_local_state);
  std::thread my_thread(my_func); 
  my_thread.detach(); // 2. 不等待线程结束
}// 3. 新线程可能还在运行

void func()
{
    thread t6(do_something);
    do_otherthig d1;
    try
    {
        d1();
    }
    catch(...)       //出现异常，也要调用join
    {
        t6.join();
        throw;
    }
    t6.join();
}

class thread_guard
{
    private:
      thread& t;
    public:
      thread_guard(thread& t_):t(t_)
      {

      }
      ~thread_guard()
      {
          if(t.joinable())
          {
              t.join();
          }
      }

      thread_guard(thread_guard const &) = delete;
      thread_guard& operator = (thread const &) = delete;
};

void f()
{
    thread t7(do_something);
    do_otherthig d1;
    thread_guard tg1(t7);
    d1();
}

int main(int argc, char const *argv[])
{
    thread t1(do_something);
    thread t2(la);
    thread t3((do_otherthig()));       //传递临时变量，会被解析为函数声明。可以使用多组括号或大括号进行初始化
    thread t4{function<void(void)>(lb)};
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    thread t5(oops);
    t5.detach();
    return 0;
}
