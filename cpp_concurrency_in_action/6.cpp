#include <iostream>
#include <stack>
#include <queue>
#include <thread>
#include <exception>
#include <mutex>
#include <condition_variable>

struct empty_stack : public std::exception
{
  const char* what() const throw(){};
}; 

template <typename T>
class threadsafe_stack
{
  private:
    std::stack<T> data;
    mutable std::mutex m;  //mutable标志说明在const成员函数中仍可以改变此变量 
  public:
    threadsafe_stack()
    {

    }

    threadsafe_stack(const threadsafe_stack& otherstack)
    {
        std::lock_guard<std::mutex> lock(otherstack.m);
        data = otherstack.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value);
    std::shared_ptr<T> pop();
    void pop(T& value);
    bool empty() const;
};

template<typename T>
void threadsafe_stack<T>::push(T new_value)
{
    std::lock_guard<std::mutex> lock(m);
    data.push(std::move(new_value));
}

template<typename T>
std::shared_ptr<T> threadsafe_stack<T>::pop()
{
    std::lock_guard<std::mutex> lock(m);
    if(data.empty())
      throw empty_stack();
    std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
    data.pop();
    return res;
}

template<typename T>
void threadsafe_stack<T>::pop(T& value)
{
    std::lock_guard<std::mutex> lock(m);
    if(data.empty())
      throw empty_stack();
    value = std::move(data.top());
    data.pop();
}

template<typename T>
bool threadsafe_stack<T>::empty() const
{
    std::lock_guard<T> lock(m);
    return data.empty();
}

void push(threadsafe_stack<int>& ts, int n)
{
    for(int i = 0; i < n + 50; i++)
      ts.push(i);
}

//有锁队列

template <typename T>
class threadsafe_queue
{
  private:
    std::queue<T> data;
    std::mutex m;
    std::condition_variable data_cond;
  public:
    threadsafe_queue()
    {

    }

    threadsafe_queue(const threadsafe_queue<T>& sq)
    {
        std::lock_guard<std::mutex> lock(sq.m);
        data = sq.data;
    }

    threadsafe_queue& operator= (const threadsafe_queue<T>& sq) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
        data_cond.notify_one();
    }

    bool wait_and_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        //谓词为假，则挂起线程，并解锁互斥量，被唤醒后，获取互斥量并再次判断谓词
        data_cond.wait(m,[this](){return !data.empty();});
        value = std::move(data.front());
        data.pop();
        return true;
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::lock_guard<std::mutex> lock(m);
        data_cond.wait(m,[this](){return !data.empty();});

        std::shared_ptr<T> res(std::make_shared<T>(std::move(data.front())));
        data.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty())
          return false;
        value = std::move(data.front());
        data.pop();
        return false;        
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty())
          return std::shared_ptr<T>();

        std::shared_ptr<T> res(std::make_shared<T>(std::move(data.front())));
        data.pop();
        return res;        
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }

};

template <typename T>
class threadsafe_queue_shared_ptr
{
  private:
    std::queue<std::shared_ptr<T>> data;
    std::mutex m;
    std::condition_variable data_cond;
  public:
    threadsafe_queue_shared_ptr()
    {

    }

    threadsafe_queue_shared_ptr& operator= (const threadsafe_queue_shared_ptr& sq) = delete;

    void push(T new_value)
    {
        std::shared_ptr<T> temp(std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lock(m);
        data.push(temp);
        data_cond.notify_one();
    }

    bool wait_and_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        //谓词为假，则挂起线程，并解锁互斥量，被唤醒后，获取互斥量并再次判断谓词
        data_cond.wait(m,[this](){return !data.empty();});
        value = std::move(*data.front());
        data.pop();
        return true;
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::lock_guard<std::mutex> lock(m);
        data_cond.wait(m,[this](){return !data.empty();});

        std::shared_ptr<T> res = data.front();
        data.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty())
          return false;
        value = std::move(*data.front());
        data.pop();
        return false;        
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty())
          return std::shared_ptr<T>();

        std::shared_ptr<T> res = data.front();
        data.pop();
        return res;        
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

int main()
{
    threadsafe_stack<int> ts;
    int temp;
    std::thread t1(push, std::ref(ts), 0);
    std::thread t2(push, std::ref(ts), 50);

    t1.join();
    t2.join();

    for(int i = 0; i < 50; i++)
    {
        ts.pop(temp);
        std::cout<<temp<<' ';
    }
    std::cout<<'\n';

    return 0;
}

