#include <iostream>
#include <mutex>
#include <atomic>

class Singleton
{

};


//static关键字实现单例模式
Singleton& GetInstance_static()
{
    static Singleton signal;
    return signal;
}

//call_once调用实现单例模式
std::once_flag flag;
Singleton* single;

Singleton* GetInstance_call_once()
{
    std::call_once(flag,[&](){single = new Singleton();});
    return single;
}

//内存序列 + 双重检查锁机制实现单例模式

std::atomic<Singleton*> m_instance;
std::mutex m;

Singleton* GetInstance_memory_order()
{
    Singleton* tmp = m_instance.load(std::memory_order_acquire);
    if(tmp == nullptr)
    {
        std::lock_guard<std::mutex> lk(m);
        tmp = m_instance.load(std::memory_order_relaxed);
        if(tmp == nullptr)
        {
            tmp = new Singleton();
            m_instance.store(tmp,std::memory_order_release);
        }
    }
    return tmp;
}

int main()
{
    return 0;
}