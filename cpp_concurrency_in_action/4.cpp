#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <future>
#include <numeric>
#include <functional>


std::mutex mu;
std::condition_variable data_cond;
std::queue<int> data_queue;
int sum = 0;

auto producer = [](){return (data_queue.size() < 5);};

auto consumer = [](){return (data_queue.size() >= 5);};

void data_producer()
{
    while(1)
    {
        std::unique_lock<std::mutex> lk(mu);
        data_cond.wait(lk,producer); //封装之后加上谓词，不用通过while循环处理假唤醒
        while(data_queue.size() < 5)    //生产者每次生产5个
        {
            std::cout <<"producer:"<<sum <<'\n';
            data_queue.push(sum++);
        }
        data_cond.notify_one();//通知消费者
    }
}

void data_consumer()
{
    while (1)
    {
        std::unique_lock<std::mutex> lk(mu);//需要使用unique_lock，wait会对mutex解锁
        data_cond.wait(lk, consumer);
        while (data_queue.size() > 0)//消费者全部消费
        {
            std::cout << "consumer:" << data_queue.front() << '\n';
            data_queue.pop();
        }
        data_cond.notify_one();//通知生产者
    }
}

template<typename T>
class threadsafe_queue
{
private:
    std::mutex data_mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue(){};

    threadsafe_queue(threadsafe_queue const& other)
    {
        std::lock_guard<std::mutex> lock_guard(data_mut);
        data_queue = other.data_queue;
    }

    threadsafe_queue& operator=(const threadsafe_queue&) = delete;//不允许简单赋值

    void push(T value);

    bool try_pop(T& value);

    std::shared_ptr<T> try_pop();

    void wait_and_pop(T& value);

    std::shared_ptr<T> wait_and_pop();

    bool empty() const;

};

template<typename T>
void threadsafe_queue<T>::push(T value)
{
    std::lock_guard<std::mutex> lk(data_mut);
    data_queue.push(value);
    data_cond.notify_one();//通知等待线程
}

template<typename T>
void threadsafe_queue<T>::wait_and_pop(T& value)
{
    std::lock_guard<std::mutex> lk(data_mut);
    data_cond.wait(lk, [this](){return !data_queue.empty();});//队列不为空唤醒
    value = data_queue.front();
    data_queue.pop();
}

template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    std::lock_guard<std::mutex> lk(data_mut);
    data_cond.wait(lk, [this](){return !data_queue.empty();});
    std::shared_ptr<T> res = std::make_shared<T>(data_queue.front());
    data_queue.pop();
    return res;
}

template<typename T>
bool threadsafe_queue<T>::try_pop(T& value)
{
    std::lock_guard<std::mutex> lk(data_mut);
    if(data_queue.empty())
      return false;
    value = data_queue.front();
    data_queue.pop();
    return true;
}

template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop()
{
    std::lock_guard<std::mutex> lk(data_mut);
    if (data_queue.empty())
        return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
}

template<typename T>
bool threadsafe_queue<T>::empty() const
{
    return data_queue.empty();
}

template<typename T>
int parallel_sum(T beg,T end)
{
    int len = end - beg;
    T mid = beg + len/2;
    if(len < 1000)
      return std::accumulate(beg,end,0);
    auto ft = std::async(std::launch::async, parallel_sum<T>, mid, end);
    int result  = parallel_sum(beg, mid);
    return result + ft.get();
}

int add_func(int a,int b)
{
    return a + b;
}

void task_fun()
{
    //包装lambda表达式
    std::packaged_task<int(int,int)> pa([](int a,int b)-> int {return a + b;});
    std::future<int> ft = pa.get_future();
    pa(1,2);
    std::cout<<"lambda:"<<ft.get()<<'\n';

    std::packaged_task<int()> pb(std::bind(add_func,1,2));
    auto ft1 = pb.get_future();
    pb();
    std::cout<<"bind:"<<ft1.get()<<'\n';

    std::packaged_task<int(int,int)> pc(add_func);
    auto ft2 = pc.get_future();
    std::thread t1(std::move(pc),1,4);
    t1.join();
    std::cout<<"thread:"<<ft2.get()<<'\n';
}

int main()
{

  //  std::thread t1(data_producer);
   // std::thread t2(data_consumer);

  //  t1.join();
   // t2.join();

    std::vector<int> v(1000,1);
    std::cout<<"parallel sum is: "<<parallel_sum(v.begin(),v.end())<<'\n';

    task_fun();

    return 0;
}