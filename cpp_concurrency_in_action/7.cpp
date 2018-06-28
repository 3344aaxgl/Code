#include <iostream>
#include <atomic>
#include <memory>
#include <future>


class spin_lock
{
  private:
    std::atomic_flag flag;
  public:
    void lock()
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
};

template<typename T>
class lock_free_stack
{
  private:
    struct node
    {
        std::shared_ptr<T> data;
        node* next;

        node(T value):data(std::make_shared<T>(value))
        {

        }
    };
    std::atomic<node*> head;
  public:
    void push(T value)
    {
        node* const new_node = new node(value);
        new_node->next = head.load();
        while(!head.compare_exchange_weak(new_node->next,new_node));
    }

    std::shared_ptr<T> pop()
    {
        node* old_head = head.load();
        while(old_head && head.compare_exchange_strong(old_head,old_head->next));
        return old_head?old_head->data:std::shared_ptr<T>();
    }
};

//通过线程计数解决内存泄露的无锁队列
template<typename T>
class lock_free_stack_by_threadcount
{
  private:
    struct node
    {
        std::shared_ptr<T> data;
        node* next;

        node(T value):data(std::make_shared<T>(value))
        {

        }
    };
    static void delete_nodes(node* nodes)
    {
        while(nodes)
        {
            node* next = nodes->next;
            delete nodes;
            nodes = next;
        }
    }

    void chain_pending_nodes(node* first,node* last)
    {
        last->next = to_be_deleted.load();
        while(to_be_deleted.compare_exchange_weak(last->next,first));
    }

    void chain_pending_nodes(node* nodes)
    {
        node* first = nodes;
        node* last = first;
        while(nodes)
        {
            last = nodes;
            nodes = nodes->next;
        }
        chain_pending_nodes(first,last);
    }

    void chain_pending_node(node* nodes)
    {
        chain_pending_nodes(nodes,nodes);
    }

    void try_reclaim(node* old_head)
    {
        if(threads_in_pop == 1)
        {
            node * nodes_to_delete = to_be_deleted.exchange(nullptr);
            //这里需要在校验一次是因为如果在第一次校验和原子操作之间，有其他的节点加入待删除
            //链表中，而这里不再次进行判断的话，那这个新加入的节点可能会出现内存泄露
            //只要有一个瞬间，线程计数为1，那就说明该节点没有其他人引用了
            if(!--threads_in_pop)
            {
                delete_nodes(nodes_to_delete);
            }
            else
            {
                chain_pending_nodes(nodes_to_delete);
            }
            delete old_head;
        }
        else
        {
            chain_pending_node(old_head);
            --threads_in_pop;
        }
    }
    std::atomic<node*> head;
    std::atomic<unsigned> threads_in_pop;
    std::atomic<node*> to_be_deleted;
  public:
    void push(T value)
    {
        node* new_node = new node(value);
        new_node->next = head.load();
        while(!head.compare_exchange_weak(new_node->next, new_node));
    }

    std::shared_ptr<T> pop()
    {
        //操作之前，先将计数增加,使用前置自增，效率更高
        ++threads_in_pop;
        node* old_head = head.load();
        while(old_head && !head.compare_exchange_weak(old_head,old_head->next));
        std::shared_ptr<T> res;
        if(old_head)
        {
            res.swap(old_head->data);
        }
        try_reclaim(old_head);
        return res;      
    }

    lock_free_stack_by_threadcount()
    {
        head.store(nullptr);
    }

    ~lock_free_stack_by_threadcount()
    {
        node* old_head = head.load();
        node* next_node = old_head->next;
        while(old_head)
        {
           delete old_head;
           old_head = next_node;
           next_node = next_node->next;
        }
    } 
};


//使用风险指针解决内存泄露问题

unsigned const max_hazard_pointers = 128;
struct hazard_pointer
{
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};

hazard_pointer hazard_pointers[max_hazard_pointers];

class hp_owner
{
  private:
    hazard_pointer* hp;
  public:
    hp_owner(const hazard_pointer&) = delete;
    hp_owner& operator=(const hazard_pointer&) = delete;
    hp_owner():hp(nullptr)
    {
        for(unsigned i = 0; i < max_hazard_pointers; i++)
        {
            std::thread::id old_id;
            if(hazard_pointers[i].id.compare_exchange_strong(old_id,std::this_thread::get_id()))
            {
                hp = &hazard_pointers[i];
                break;
            }
        }
        if(!hp)
          throw std::runtime_error("no hazard pointer available");
    }

    std::atomic<void*>& get_pointer()
    {
        return hp->pointer;
    }

    ~hp_owner()
    {
        hp->pointer.store(nullptr);
        hp->id.store(std::thread::id());
    }
};

std::atomic<void*>& get_hazard_pointer_for_current_thread()
{
    thread_local static hp_owner hp;
    return hp.get_pointer();
}

bool outstanding_hazard_pointers_for(void* nodes)
{
    for(int i = 0;i < max_hazard_pointers; i++)
    {
        if(hazard_pointers[i].pointer.load() == nodes)
          return true;
    }
    return false;
}

template <typename T>
void to_delete(void*p)
{
    delete static_cast<T*>(p);
}

struct data_to_reclaim
{
    void* data;
    std::function<void(void *)> deleter;
    data_to_reclaim* next;

    template <typename T>
    data_to_reclaim(T* value):data(value),deleter(&to_delete<T>),next(0)
    {

    }

    ~data_to_reclaim()
    {
        deleter(data);
    }
};

std::atomic<data_to_reclaim*> nodes_to_reclaim;

void add_to_reclaim_list(data_to_reclaim* node)
{
    node->next = nodes_to_reclaim.load();
    while(nodes_to_reclaim.compare_exchange_weak(node->next,node));
}

template <typename T>
void reclaim_later(T* data)
{
    add_to_reclaim_list(new data_to_reclaim(data));
}

void delete_node_with_no_hazards()
{
    data_to_reclaim* current = nodes_to_reclaim.exchange(nullptr);
    while(current)
    {
        data_to_reclaim* next = current->next;
        if(outstanding_hazard_pointers_for(current->data))
        {
            delete current;
        }
        else
        {
            add_to_reclaim_list(current);
        }
        current = next;        
    }
}

template <typename T>
class lock_free_stack_by_hazardpointer
{
  private:
    struct node
    {
        std::shared_ptr<T> data;
        node* next;

        node(T value):data(std::make_shared<T>(value))
        {

        }
    };
    std::atomic<node*> head;
  public:
    lock_free_stack_by_hazardpointer()
    {
        head.store(nullptr);
    }

    ~lock_free_stack_by_hazardpointer()
    {
        while(pop());
    }

    void push(T value)
    {
        node* new_node = new node(value);
        new_node->next = head.load();
        while(!head.compare_exchange_weak(new_node->next, new_node));
    }

    std::shared_ptr<T> pop()
    {
        std::atomic<void*>& hp = get_hazard_pointer_for_current_thread();
        node* old_head = head.load();

        do
        {
            node* temp;
            do
            {
                temp = old_head;
                hp.store(old_head);
                old_head = head.load();
            } while (old_head != temp); //保证hp存储原子操作前后head保持一致，这样风险指针指向的才是头结点
        } while (old_head && !head.compare_exchange_strong(old_head, old_head->next));

        hp.store(nullptr);//删除完后清空风险指针
        std::shared_ptr<T> res;
        if(old_head)
        {

            res.swap(old_head->data);
            if(outstanding_hazard_pointers_for(old_head))//风险指针不为空，加入待删除链表
            {
                reclaim_later(old_head);
            }
            else//删除节点
            {
                delete old_head;
            }
            //这里再次删除没有风险指针的节点是因为在删除节点时，
            //可能有风险指针指向节点，但这个风险指针这次没有删除这个节点
            //再次循环时删除了另一个节点，从而导致这个节点没有线程会将其删除
            delete_node_with_no_hazards();
        }
        return res;
    }
};

//使用智能指针的引用计数解决内存泄露问题
template <typename T>
class lock_free_stack_by_smartpointer
{
  private:
    struct node
    {
        std::shared_ptr<T> data;
        std::shared_ptr<node> next;
        node(T const &data_) : data(std::make_shared<T>(data_))
        {
        }
    };
    std::shared_ptr<node> head;

  public:
    void push(T const &data)
    {
        std::shared_ptr<node> const new_node = std::make_shared<node>(data);
        new_node->next = head.load();
        while (!std::atomic_compare_exchange_weak(&head,
                                                  &new_node->next, new_node))
            ;
    }
    std::shared_ptr<T> pop()
    {
        std::shared_ptr<node> old_head = std::atomic_load(&head);
        while (old_head && !std::atomic_compare_exchange_weak(&head,
                                                              &old_head, old_head->next))
            ;
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
};

//使用引用计数解决内存泄露问题
template <typename T>
class lock_free_stack_by_referencecount
{
  private:
    struct node;
    //直接将引用计数存放在节点中
    struct counted_node_ptr
    {
        int external_count;
        node* ptr;
    };

    struct node
    {
        std::shared_ptr<T> data;
        std::atomic<int> internal_count;
        counted_node_ptr next;

        node(T value):data(std::make_shared<T>(value)),internal_count(0)
        {

        }
    };

    std::atomic<counted_node_ptr> head;

    //compare_exchange_strong按位去比较调用对象与期望对象
    //这里保证每个对同一head对象访问都能增加head的外部计数
    void increase_head_count(counted_node_ptr& old_head)
    {
        counted_node_ptr new_head;
        do
        {
            new_head = old_head;
            ++new_head.external_count;
        }while(head.compare_exchange_strong(old_head,new_head));
    }

  public:
    void push(T value)
    {
        node* new_node = new node(value);
        counted_node_ptr p;
        p.external_count = 1;
        p.ptr = new_node;
        p.ptr->next = head.load();
        while(head.compare_exchange_weak(p.ptr->next,p));
    }

    std::shared_ptr<T> pop()
    {
        counted_node_ptr old_head = head.load();
        while(1)
        {
            increase_head_count(old_head);
            node* ptr = old_head.ptr;
            if(!ptr)
            {
                return std::shared_ptr<T>();
            }
            //如果删除节点成功，则将这个线程增加的外部引用计数减去，
            //并减去push时默认的1，剩下的值就是这个节点其他访问这个节点的
            //线程数。将内部计数加上这个数字，如果加完之后内部计数为0
            //则说明没有其他线程访问这个节点，可以删除节点空间
            if(head.compare_exchange_strong(old_head, ptr->next))
            {
                std::shared_ptr<T> res;
                res.swap(ptr->data);
                int increase_count = old_head.external_count - 2;
                if(ptr->internal_count.fetch_add(increase_count) == -increase_count)
                  delete ptr;
                return res;
            }
            //如果失败，说明这个对象在增加引用到现在已经被其他线程pop
            //所以把内部引用计数减一,如果内部计数为1，说明没有其他
            //线程在使用这个节点，则可以删除节点空间
            else if(ptr->internal_count.fetch_sub(1) == 1)
            {
                delete ptr;
            }
        }
    }
};

int main()
{
    lock_free_stack_by_threadcount<int> lfs;
    std::thread t1(lock_free_stack_by_threadcount<int>::push,&lfs,0);
    std::thread t2(lock_free_stack_by_threadcount<int>::push,&lfs,1);

    t1.join();
    t2.join();
    
    auto t3 = std::async(lock_free_stack_by_threadcount<int>::pop, &lfs);
    std::cout<<*(t3.get())<<'\n';

    lock_free_stack_by_hazardpointer <int> lfh;
    std::thread t4(lock_free_stack_by_hazardpointer<int>::push,&lfh,0);
    std::thread t5(lock_free_stack_by_hazardpointer<int>::push,&lfh,1);

    t4.join();
    t5.join();
    
    auto t6 = std::async(lock_free_stack_by_hazardpointer<int>::pop, &lfh);
    std::cout<<*(t6.get())<<'\n';

    return 0;
}
