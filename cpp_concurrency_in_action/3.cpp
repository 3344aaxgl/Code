#include <iostream>
#include <mutex>
#include <algorithm>
#include <list>
#include <thread>
#include <vector>
#include <functional>
#include <stack>
#include <exception>

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    return std::find(some_list.begin(),some_list.end(),value_to_find) != some_list.end();
}

class some_data
{
private:
  int a;
  std::string b;
public:
  void do_something()
  {

  } 
};

class data_wrapper
{
  private:
    some_data data;
    std::mutex m;

  public:
    template <typename Fundction>
    void process_data(Fundction func)
    {
        std::lock_guard<std::mutex> l(m);
        func(data);
    }
};

some_data* unprotected;

void malicious_function(some_data& data)
{
    unprotected = &data;
}

data_wrapper x;

void foo()
{
    x.process_data(malicious_function);
    unprotected->do_something();
}

struct empty_stack : std::exception
{
    const char *what() const throw()
    {
        return "empty stack!";
    };
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack():data(std::stack<T>())
    {

    }
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);  
        if(data.empty()) throw empty_stack();

        std::shared_ptr<T> const res (std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m); 
        if(data.empty()) throw empty_stack();

        value = data.top();
        data.pop();        
    }
    
    bool empty()
    {
        std::lock_guard<std::mutex> lock(m); 
        return data.empty();
    }
};

class X
{
private:
    int data;
    std::mutex m;
public:
    X(int d):data(d)
    {

    }
    friend void swap(X& lhs, X& rhs)
    {
        std::lock(lhs.m, rhs.m);
        std::lock_guard<std::mutex> lock_a(lhs.m,std::adopt_lock);//只支持adopt_lock
        std::lock_guard<std::mutex> lock_b(rhs.m,std::adopt_lock);

        std::swap(lhs.data, rhs.data);
    }
};

//层级互斥量
class hierarchical_mutex
{
private:
    std::mutex interal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    static thread_local unsigned long this_thread_hierarchy_value;
    void check_for_hierarchy_violation()
    {
        if(this_thread_hierarchy_value <= hierarchy_value)//限制锁的层级
        {
            throw std::logic_error("mutex hierarchy violated");
        } 
    }

    void update_hierarchy_value()
    {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }

public:
    explicit hierarchical_mutex(unsigned long value):hierarchy_value(value),previous_hierarchy_value(0)
    {

    }

    void lock()
    {
        check_for_hierarchy_violation();//先检查当前的层次是否大于要锁的层次
        interal_mutex.lock();
        update_hierarchy_value();//更新当前层次为锁住的层次
    }

    void unlock()
    {
        this_thread_hierarchy_value = previous_hierarchy_value;//回退到上一个层次
        interal_mutex.unlock();
    }

    bool try_lock()
    {
        check_for_hierarchy_violation();
        if(!interal_mutex.try_lock())
          return false;
        update_hierarchy_value();
        return true;
    }
};

//每个线程都能独立的拥有这个变量的副本，结合static一起使用，在这个线程中，这个类的所有对象都能按照顺序进行上锁
thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);


void Test_hierarchical_mutex()
{
    hierarchical_mutex  high_level_lock(1000);
    hierarchical_mutex  low_level_lock(500);
    std::lock_guard<hierarchical_mutex> lk1(high_level_lock);//调整顺序报错
    std::lock_guard<hierarchical_mutex> lk2(low_level_lock);
    
    std::cout<<"lock two level lock"<<'\n';
}

class X1
{
private:
    int data;
    std::mutex m;
public:
    X1(int d):data(d)
    {

    }
    friend void swap(X1& lhs, X1& rhs)
    {
        std::unique_lock<std::mutex> lock_a(lhs.m,std::defer_lock);//仍未取得锁，只是方便解锁
        std::unique_lock<std::mutex> lock_b(rhs.m,std::defer_lock);
        std::lock(lock_a, lock_b);//互斥量上锁

        std::swap(lhs.data, rhs.data);
    }
};

std::unique_lock<std::mutex> get_lock()
{
    //引用全局变量
    extern std::mutex some_mutex;
    std::unique_lock<std::mutex> lk(some_mutex);
    return lk;
}

void process_data()
{
    std::unique_lock<std::mutex> lk(get_lock());
}


int main()
{
    std::vector<std::thread> threads(3);

    for(int i = 0; i < 3; i++)
    {
        threads[i] = std::thread(add_to_list, i+1);
    }

    for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));

    std::cout<<list_contains(4)<<" "<<list_contains(3);

    Test_hierarchical_mutex();

    return 0;
}

