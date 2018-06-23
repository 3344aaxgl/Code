#include <iostream>
#include <stack>
#include <queue>
#include <list>
#include <thread>
#include <exception>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <shared_mutex>
#include <algorithm>
#include <string>

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

//单线程版队列
template<typename T>
class queue_singnal
{
  private:
    struct node
    {
        T data;
        std::unique_ptr<node> next;
        node(T value):data(std::move(value))
        {

        }
    };
    
    //自动释放
    std::unique_ptr<node> head;
    node* tail;
  public:
    queue_singnal()
    {

    }

    queue_singnal(const queue_singnal& ) = delete;
    queue_singnal& operator=(const queue_singnal&) = delete;

    std::shared_ptr<T> try_pop()
    {
        if(!head)
        {
            return std::shared_ptr<T>();
        }

        std::shared_ptr<T> res(std::make_shared<T>(std::move(head->data)));
        std::unique_ptr<node> old_head = std::move(head);                  //自动删除
        head = std::move(old_head->next);
        return res;
    }

    void push(T value)
    {
        std::unique_ptr<node> new_node(new node(std::move(value)));
        if(tail)
          tail->next = std::move(new_node);
        else
          head = std::move(new_node);
        tail = new_node;
    }
};


//带虚拟节点可等待线程安全队列
template <typename T>
class threadsafe_queue_virtual
{
  private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::unique_ptr<node> head;
    node* tail;
    std::mutex head_mutex;
    std::mutex tail_mutex;
    std::condition_variable data_cond;

    node* get_tail()
    {
        std::lock_guard<std::mutex> lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head()
    {
        std::unique_ptr<node> old_head = std::move(head);//head不指向链表
        head = std::move(old_head->next);//old_head->next不指向链表，head指向新的头结点
        return old_head;        
    }

    std::unique_ptr<node> try_pop_head()
    {
        std::lock_guard<std::mutex> lock(head_mutex);
        //这里get_tail执行完后释放tail_mutex，tail可能因插入操作而发生改变
        //即此时head和tail已不相等，但此等式为真。可以在try_pop_head外层增加while循环
        //这样最大化并发
        if(head.get() == get_tail())
        {
            return std::unique_ptr<node>();
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T& value)
    {
        std::lock_guard<std::mutex> lock(head_mutex);
        if(head.get() == get_tail())
        {
            return std::unique_ptr<node>();            
        }
        value = std::move(*head->data);
        return pop_head();
    }

    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> lock(head_mutex);
        data_cond.wait(lock,[&](){return head.get() != get_tail();});
        //mutex不可移动和赋值，但unique_lock可以
        return std::move(lock);
    }

    std::unique_ptr<node> wait_pop_head()
    {
        std::unique_lock<std::mutex> lock = wait_for_data();
        return pop_head();
    }

    std::unique_ptr<node> wait_pop_head(T& value)
    {
        std::unique_lock<std::mutex> lock = wait_for_data();
        value = std::move(*head->data);
        return pop_head();
    }

  public:
    threadsafe_queue_virtual():head(new node),tail(head.get())
    {
    }

    threadsafe_queue_virtual(const threadsafe_queue_virtual &) = delete;
    threadsafe_queue_virtual &operator=(const threadsafe_queue_virtual &) = delete;

    std::shared_ptr<T> try_pop();
    bool try_pop(T &value);
    std::shared_ptr<T> wait_and_pop();
    void wait_and_pop(T &value);
    void push(T new_value);
    void empty();
};

template<typename T>
void threadsafe_queue_virtual<T>::push(T new_value)
{
    std::unique_ptr<node> p(new node());
    std::shared_ptr<T> new_node(std::make_shared<T>(std::move(new_value))); 
    std::lock_guard<std::mutex> lock(tail_mutex);
    tail->data = new_node;
    node* new_tail = p.get();
    tail->next = std::move(p);//使用右值，否则空间将被p释放
    tail = new_tail;
}

template<typename T>
std::shared_ptr<T> threadsafe_queue_virtual<T>::try_pop()
{
    std::unique_ptr<node> old_head = try_pop_head();
    return old_head?*old_head->data:std::shared_ptr<T>();
}

template<typename T>
bool threadsafe_queue_virtual<T>::try_pop(T& value)
{
    std::unique_ptr<node>const old_head = try_pop_head(value);
    return old_head != nullptr;
}

template<typename T>
void threadsafe_queue_virtual<T>::empty()
{
    std::lock_guard<std::mutex> lock(head_mutex);
    return head.get() == get_tail();
}

template<typename T>
std::shared_ptr<T> threadsafe_queue_virtual<T>::wait_and_pop()
{
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
}

template<typename T>
void threadsafe_queue_virtual<T>::wait_and_pop(T& value)
{
    std::unique_ptr<node> const old_head = wait_pop_head(value);
}

//线程安全查询表
template<typename Key,typename Value,typename Hash = std::hash<Key>>
class threadsafe_lookup_table
{
  private:
    class bucket_type
    {
      private:
        typedef std::pair<Key,Value> bucket_value;
        typedef std::list<bucket_value> bucket_data;
        typedef typename bucket_data::iterator bucket_iterator;

        bucket_data data;
        mutable std::shared_mutex mutex;

        bucket_iterator find_entry_for(Key const& key)
        {
           return std::find_if(data.begin(),data.end(),[&](bucket_value const & item){return item.first == key;});
        }
      public:
        Value value_for(Key const& key,Value const& default_value)
        {
            std::shared_lock<std::shared_mutex> lock(mutex);
            bucket_iterator found_entry = find_entry_for(key);
            return found_entry == data.end()?default_value:found_entry->second;
        }

        void add_or_update_mapping(Key const& key,Value const&value)
        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            bucket_iterator found_entry = find_entry_for(key);
            if(found_entry == data.end())
            {
                data.push_back(bucket_value(key,value));
            }
            else
            found_entry->second = value;
        }

        void remove_mapping(Key const& key)
        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            bucket_iterator found_entry = find_entry_for(key);
            if(found_entry != data.end())
              data.erase(found_entry);
        }
    };
    std::vector<std::unique_ptr<bucket_type>> buckets;
    Hash hasher;

    bucket_type& get_bucket(Key const & key)
    {
        std::size_t const bucket_index = hasher(key)%buckets.size();
        return *buckets[bucket_index]; 
    }
  public:
    typedef Key   key_type;
    typedef Value mapped_type;
    typedef Hash  hash_type;

    threadsafe_lookup_table(unsigned num_buckets = 19,Hash const& hasher_ = Hash())
    :buckets(num_buckets),hasher(hasher_)
    {
      for(unsigned i = 0;i < num_buckets; i++)
      {
          buckets[i].reset(new bucket_type);
      }
    }

    threadsafe_lookup_table(const threadsafe_lookup_table&) = delete;
    threadsafe_lookup_table& operator=(const threadsafe_lookup_table&) = delete;

    Value value_for(Key const& key,Value const value = Value())
    {
        return get_bucket(key).value_for(key,value);
    }

    void add_or_update_mapping(Key const& key, Value const& value)
    {
        get_bucket(key).add_or_update_mapping(key,value);
    }

    void remove_mapping(Key const& key)
    {
        get_bucket(key).remove_mapping(key);
    }

};

//线程安全队列
template<typename T>
class threadsafe_list
{
  private:
    struct node
    {
      std::mutex m;
      std::shared_ptr<T> data;
      std::unique_ptr<node> next;
      node():next()
      {

      }
      node(const T& value):data(std::make_shared<T>(value))
      {

      }
    };
    node head;
  public:
    threadsafe_list()
    {

    }

    ~threadsafe_list()
    {
        remove_if([](const node&){return true;});
    }

    threadsafe_list(const threadsafe_list&) = delete;
    threadsafe_list& operator=(const threadsafe_list&) = delete;

    void push_front(T const& value)
    {
        std::unique_ptr<node> new_node(new node(value));
        std::unique_ptr<std::mutex> lock(head.m);

        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template<typename Function>
    void for_each(Function f)
    {
        //head不存储值
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node * const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            f(*next->data);
            current = next;
            lk = std::move(next_lk);
        }
    }

    template <typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p)
    {
        node *current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while (node *const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            if (p(*next->data))
            {
                return next->data;
            }
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }

    template <typename Predicate>
    void remove_if(Predicate p)
    {
        node *current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while (node *const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            if (p(*next->data))
            {
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lk.unlock();
            }
            else
            {
                lk.unlock();
                current = next;
                lk = std::move(next_lk);
            }
        }
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

    threadsafe_queue_virtual<int> tv;
    tv.push(1);
    tv.push(2);
    tv.try_pop(temp);
    std::cout<<temp<<'\n';
    tv.wait_and_pop(temp);
    std::cout<<temp<<'\n';


    threadsafe_lookup_table<int,std::string> tlt;
    tlt.add_or_update_mapping(1,"hello");
    tlt.add_or_update_mapping(2,"world");
    std::cout<<tlt.value_for(1)<<" "<<tlt.value_for(2);



    return 0;
}

