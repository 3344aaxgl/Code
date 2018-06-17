#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <cassert>
#include <string>
#include <queue>

class spinclock_mutex
{
  private:
    std::atomic_flag flag;

  public:
    spinclock_mutex() : flag(ATOMIC_FLAG_INIT)
    {
    }

    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
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
    for (int i = 0; i < 10; i++)
        //传入入参，调用构造函数
        v.emplace_back(fun, i, std::ref(sp));

    for (auto &t : v)
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
    while (!data_ready.load())
        ;
    std::cout << "The answer=" << data[0] << '\n';
}

void writer_thread()
{
    data.push_back(42);
    data_ready.store(true);
}

void fun(int a, int b)
{
    std::cout << a << " " << b << '\n';
}

int getnum()
{
    static int num = 0;
    return ++num;
}

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x()
{
    x.store(true, std::memory_order_seq_cst);
}

void write_y()
{
    y.store(true, std::memory_order_seq_cst);
}

void read_x_then_y()
{
    //如果X为true但z没有增加，那么说明write_x操作先于write_y操作
    while (!x.load(std::memory_order_seq_cst))
        ;
    if (y.load(std::memory_order_seq_cst))
        ++z;
}

void read_y_then_x()
{
    while (!y.load(std::memory_order_seq_cst))
        ;
    if (x.load(std::memory_order_seq_cst))
        ++z;
}

void write_x_then_y()
{
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x_relaxed()
{
    while (!y.load(std::memory_order_relaxed))
        ;
    if (x.load(std::memory_order_relaxed))
        z++;
}

std::atomic<int> x_relaxed(0),y_relaxed(0),z_relaxed(0);
std::atomic<bool> go(false);

const int loop_count = 10;

struct read_values
{
    int x;
    int y;
    int z;
};

read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];

void incresment(std::atomic<int>* var_to_inc,read_values* values)
{
    while(!go.load())
        std::this_thread::yield();
    for(int i = 0;i < loop_count; i++)
    {
        values[i].x = x_relaxed.load(std::memory_order_relaxed);
        values[i].y = y_relaxed.load(std::memory_order_relaxed);
        values[i].z = z_relaxed.load(std::memory_order_relaxed);
        var_to_inc->store(i+1,std::memory_order_relaxed);
        std::this_thread::yield();
    }
}

void read_value(read_values* values)
{
    while(!go.load())
        std::this_thread::yield();
    for(int i = 0;i < loop_count; i++)
    {
        values[i].x = x_relaxed.load(std::memory_order_relaxed);
        values[i].y = y_relaxed.load(std::memory_order_relaxed);
        values[i].z = z_relaxed.load(std::memory_order_relaxed);
        std::this_thread::yield();
    }    
}

void print(read_values* values)
{
    for(int i = 0; i < loop_count; i++)
    {
        if(i)
          std::cout<<',';
        std::cout<<"("<<values[i].x<<","<<values[i].y<<","<<values[i].z<<")";
    }
    std::cout<<'\n';
}

void write_x_release()
{
    x.store(true,std::memory_order_release);
}

void write_y_release()
{
    y.store(true,std::memory_order_release);
}

void read_x_then_y_acquire()
{
    while(!x.load(std::memory_order_acquire))
      ;
    if(y.load(std::memory_order_acquire))
      ++z;
}

void read_y_then_x_acquire()
{
    while(!y.load(std::memory_order_acquire))
      ;
    if(x.load(std::memory_order_acquire));
      ++z;
}

void write_x_then_y_release()
{
    x.store(true,std::memory_order_relaxed);
    y.store(true,std::memory_order_release);
}

void read_y_then_x_acquire1()
{
    while(!y.load(std::memory_order_acquire))
      ;
    if(x.load(std::memory_order_relaxed))
      ++z;
}

std::atomic<int> source[5];
std::atomic<bool> sync1(false),sync2(false);

void thread_1()
{
    source[0].store(0,std::memory_order_relaxed);
    source[1].store(1,std::memory_order_relaxed);
    source[2].store(2,std::memory_order_relaxed);
    source[3].store(3,std::memory_order_relaxed);
    source[4].store(4,std::memory_order_relaxed);
    sync1.store(true,std::memory_order_release);    
}

void thread_2()
{
    while(!sync1.load(std::memory_order_acquire))
      ;
    sync2.store(true,std::memory_order_release);
}

void thread_3()
{
    while(!sync2.load(std::memory_order_acquire))
      ;
    assert(source[0].load(std::memory_order_relaxed) == 0);
    assert(source[1].load(std::memory_order_relaxed) == 1);
    assert(source[2].load(std::memory_order_relaxed) == 2);
    assert(source[3].load(std::memory_order_relaxed) == 3);
    assert(source[4].load(std::memory_order_relaxed) == 4);
}

struct X
{
    int i;
    std::string s;
};

std::atomic<X*> pp;
std::atomic<int> a;

void thread_x()
{
    X* x = new X();
    x->i = 1;
    x->s = "hello";
    a.store(99,std::memory_order_relaxed);
    pp.store(x,std::memory_order_release);
}

void use_x()
{
    X* x;
    while(!(x = pp.load(std::memory_order_consume)))
      ;
    assert(x->i == 1);
    assert(x->s == "hello");
    assert(a.load(std::memory_order_relaxed) == 99);    
}

std::vector<int> queue_data;
std::atomic<int> count;

void populate_queue()
{
    unsigned const num_of_items = 20;
    queue_data.clear();
    for(int i = 0;i < num_of_items; i++)
      queue_data.push_back(i);
    
    count.store(num_of_items, std::memory_order_release);
}

void consume_queue_items(int n)
{
    while(true)
    {
        int item_index;
        if((item_index = count.fetch_sub(1,std::memory_order_acquire)) <= 0)
          break;
        std::cout<<"thread "<<n<<":"<<queue_data[item_index-1]<<'\n';
    }
}

void write_x_then_y_fence()
{
    x.store(true,std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x_fence()
{
    while(!(y.load(std::memory_order_relaxed)))
      ;
    std::atomic_thread_fence(std::memory_order_acquire);
    if(x.load(std::memory_order_relaxed))
      ++z;
}

bool x_fence = false; // x现在是一个非原子变量
void write_x_then_y_notatomic()
{
    x_fence = true; // 1 在栅栏前存储x
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed); // 2 在栅栏后存储y
}

void read_y_then_x_notatomic()
{
    while (!y.load(std::memory_order_relaxed))
        ; // 3 在#2写入前，持续等待
    std::atomic_thread_fence(std::memory_order_acquire);
    if (x) // 4 这里读取到的值，是#1中写入
        ++z;
}


int main()
{
    test_spinlock();

    bool expect = true;
    std::atomic_bool b;
    while (!b.compare_exchange_weak(expect, false))
        std::cout << "do compare_exchange_weak";
    {
        Foo f[5];
        std::atomic<Foo *> p(f);
        Foo *x = p.fetch_add(2);
        assert(x == f);
        assert(p.load() == &f[2]);
        x = (p -= 1);
        assert(x == &f[1]);
        assert(p.load() == &f[1]);
    }
    std::thread t1(reader_thread);
    std::thread t2(writer_thread);

    t1.join();
    t2.join();

    fun(getnum(), getnum());

    std::thread t3(write_x);
    std::thread t4(write_y);
    std::thread t5(read_x_then_y);
    std::thread t6(read_y_then_x);

    t3.join();
    t4.join();
    t5.join();
    t6.join();

    assert(z.load() != 0);

    x = false;
    y = false;
    z = 0;

    std::thread t7(write_x_then_y);
    std::thread t8(read_y_then_x_relaxed);

    t7.join();
    t8.join();
    assert(z.load() != 0);

    std::thread t9(incresment,&x_relaxed,values1);
    std::thread t10(incresment,&y_relaxed,values2);
    std::thread t11(incresment,&z_relaxed,values3);
    std::thread t12(read_value,values4);
    std::thread t13(read_value,values5);

    go = true;

    t13.join();
    t12.join();
    t11.join();
    t10.join();
    t9.join();

    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values5);

    x = false;
    y = false;
    z = 0;

    std::thread t14(write_x_release);
    std::thread t15(write_y_release);
    std::thread t16(read_x_then_y_acquire);
    std::thread t17(read_y_then_x_acquire);
    
    t14.join();
    t15.join();
    t16.join();
    t17.join();

    assert(z.load() != 0);

    x = false;
    y = false;
    z = 0;

    std::thread t18(write_x_then_y_release);
    std::thread t19(read_y_then_x_acquire1);

    t18.join();
    t19.join();

    assert(z.load() != 0);

    std::thread t20(thread_1);
    std::thread t21(thread_2);
    std::thread t22(thread_3);

    t20.join();
    t21.join();
    t22.join();

    std::thread t23(populate_queue);
    std::thread t24(consume_queue_items,1);
    std::thread t25(consume_queue_items,2);

    t23.join();
    t24.join();
    t25.join();

    x = false;
    y = false;
    z = 0;

    std::thread t26(write_x_then_y_fence);
    std::thread t27(read_y_then_x_fence);

    t26.join();
    t27.join();

    assert(z != 0);

    x_fence = false;
    y = false;
    z = 0;
    std::thread t28(write_x_then_y_notatomic);
    std::thread t29(read_y_then_x_notatomic);
    t28.join();
    t29.join();
    assert(z.load() != 0); // 5 断言将不会触发

    return 0;
}