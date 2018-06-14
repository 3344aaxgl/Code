#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <cassert>

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
    for (int i = 0; i < 10; i++)
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

class Foo
{

};

std::shared_ptr<Foo> p;
void process_global_data()
{
    std::shared_ptr<Foo> local = std::atomic_load(&p);
}
void update_global_data()
{
    std::shared_ptr<Foo> local(new Foo);
    std::atomic_store(&p, local);
}

std::atomic_bool data_ready(false);
std::vector<int> data;

void reader_thread()
{
    while(!data_ready.load());
    std::cout<<"The answer="<<data[0]<<'\n';
}

void writer_thread()
{
    data.push_back(42);
    data_ready.store(true);
}

void fun(int a,int b)
{
    std::cout<<a<<" "<<b<<'\n';
}

int getnum()
{
    static int num = 0;
    return ++num;
}

int main()
{
    test_spinlock();

    bool expect = true;
    std::atomic_bool b;
    while(!b.compare_exchange_weak(expect,false))
      std::cout<<"do compare_exchange_weak";
    
    Foo f[5];
    std::atomic<Foo*> p(f);
    Foo* x = p.fetch_add(2);
    assert(x == f);
    assert(p.load() == &f[2]);
    x = (p -= 1);
    assert(x == &f[1]);
    assert(p.load() == &f[1]);
    
    std::thread t1(reader_thread);
    std::thread t2(writer_thread);

    t1.join();
    t2.join();

    fun(getnum(),getnum());

    return 0;
}