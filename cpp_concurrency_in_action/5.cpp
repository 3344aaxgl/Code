#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

class spinclock_mutex
{
  private:
    std::atomic_flag flag;
  public:
    spinclock_mutex():flag(ATOMIC_FLAG_INIT)
    {

    }

    void lock()
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
};

void fun(int n, spinclock_mutex &sp)
{
    for (int i = 0; i < 100; i++)
    {
        sp.lock();
        std::cout << "Now it's thread " << n << '\n';
        sp.unlock();
    }
}

void test_spinlock()
{
    spinclock_mutex sp;
    std::vector<std::thread> v;
    for(int i = 0; i < 10; i++)
      //传入入参，调用构造函数
      v.emplace_back(fun,i,std::ref(sp));
    
    for(auto& t : v)
      t.join();
}

int main()
{
    test_spinlock();
    bool expect = true;
    std::atomic_bool b;
    while(!b.compare_exchange_weak(expect,false))
      std::cout<<"do compare_exchange_weak";
    return 0;
}