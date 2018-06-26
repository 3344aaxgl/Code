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

//通过引用计数解决内存泄露的无锁队列
template<typename T>
class lock_free_stack_by_referencecount
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
            //只要有一个瞬间，引用计数为1，那就说明该节点没有其他人引用了
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

    ~lock_free_stack_by_referencecount()
    {
        node* old_head = head.load();
        node* next_node = p->next

        while(old_head);
        {
           delete old_head;
           old_head = next_node;
           next_node = next_node->next;
        }
    }
};


int main()
{
    lock_free_stack_by_referencecount<int> lfs;
    std::thread t1(lock_free_stack_by_referencecount<int>::push,&lfs,0);
    std::thread t2(lock_free_stack_by_referencecount<int>::push,&lfs,1);

    t1.join();
    t2.join();
    
    auto t3 = std::async(lock_free_stack_by_referencecount<int>::pop, &lfs);
    std::cout<<*(t3.get())<<'\n';

    return 0;
}
