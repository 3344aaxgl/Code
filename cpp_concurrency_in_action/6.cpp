#include <iostream>
#include <stack>
#include <thread>
#include <exception>

struct empty_stack : public std::exception
{
  const char* what() const throw();
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
        data = other.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value);
    std::shared_ptr<T> pop();
    void pop();
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
void threadsafe_stack<T>::pop()
{
    std::lock_guard<std::mutex> lock(m);
    if(data.empty())
      throw empty_stack();
    data.pop();
}

template<typename T>
bool threadsafe_stack<T>::empty() const
{
    std::lock_guard<T> lock(m);
    return data.empty();
}

int main()
{
    return 0;
}

