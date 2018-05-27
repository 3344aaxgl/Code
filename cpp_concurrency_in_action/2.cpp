#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

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

void fund()
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

void funa(int i,string && s)
{
    cout<<"the value of param is:"<<i<<endl;
}

struct funb
{
    void operator()(int i,string & s)
    {
        cout<<"the value of param is:"<<i<<endl;
    }
};

void some_function()
{

}
    
void some_other_function()
{

}

class scoped_thread
{
  private:
    thread t;
  public:
    explicit scoped_thread(thread t_):t(move(t_))
    {
        if(!t.joinable())
          throw std::logic_error("no thread");
    }

    ~scoped_thread()
    {
        t.join();
    }

    scoped_thread(scoped_thread const& ) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

template<typename Iterator,typename T>
struct accumulate_block
{
    void operator()(Iterator first,Iterator second,T& result)
    {
        result = accumulate(first,second,result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length = std::distance(first,last);
    if(!length)
      return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread -1)/min_per_thread;
    unsigned long const hanrdware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = min(hanrdware_threads != 0 ? hanrdware_threads:2, max_threads);
    unsigned long const blocks_size = length / num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads-1);

    Iterator block_start = first;
    for(int i= 0; i<(num_threads-1); i++)
    {
        Iterator block_end = block_start;
        std::advance(block_end,blocks_size);
        threads[i] = thread(accumulate_block<Iterator, T>(),block_start,block_end,ref(results[i]));
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(block_start, last, results[num_threads-1]);
    for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));
    return accumulate(results.begin(),results.end(),init);
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
 
    thread t6(funa,1,"aaa");  //
    t6.join(); 

    char buf[] = {"hello"};
    thread t7(funa,2,string(buf));//直接传入buf可能在thread调用funa之前，buf指向的空间已经被释放  
    t7.join();  

    string str("world");
    funb b;
    thread t8(&funb::operator(),&b,3,ref(str));//告诉thread传递的是引用，否则线程会先拷贝副本，然后将副本传递给函数
    t8.join(); 

    string str1("my test");
    thread t9(funa,4,std::move(str1)); //移动传值
    cout<<"now the str1 is:"<<str1<<endl;
    t9.join();

    std::thread t10(some_function);            //创建t10
    std::thread t12 = std::move(t10);           //t10将所有权转移给t12
    t10 = std::thread(some_other_function);   //隐式调用移动操作
    std::thread t13;
    t13 = std::move(t12);              //t12所有权转移给t13
    //t10 = std::move(t13);              //错误，t10已经有关联的线程

    t10.join();
    //t12.join();
    t13.join();
    int some_local_state;
    scoped_thread t(std::thread((func(some_local_state))));
    
    vector<int> v(104);
    int i = 0;
    for(vector<int>::iterator iter = v.begin(); iter != v.end(); iter ++)
    {
        *iter = ++i;
    }

    cout<<endl;
    cout<<parallel_accumulate(v.begin(),v.end(),0)<<endl;

    cout<<"ppid:"<<std::this_thread::get_id()<<endl;

    return 0;
}
