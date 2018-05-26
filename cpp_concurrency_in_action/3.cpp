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

int main()
{
    std::vector<std::thread> threads(3);

    for(int i = 0; i < 3; i++)
    {
        threads[i] = std::thread(add_to_list, i+1);
    }

    for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));

    std::cout<<list_contains(4)<<" "<<list_contains(3);

    return 0;
}

