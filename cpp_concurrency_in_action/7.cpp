#include <iostream>
#include <atomic>


class spinlock_mutex
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
        T data;
        node* next;

        node(T value):data(value)
        {

        }
    };
    std::atomic<node*> head;
  public:
    void push(T value)
    {
        node* new_node = new node(value);
        new_node->next = head.load();
        while(!head.compare_exchange_weak(new_node->next,new_node));
    }

    void pop(T& result)
    {
        node* old_head = head.load();
        while(old_head && head.compare_exchange_strong(old_head,old_head->next));
        result = old_head->data;
    }
};

int main()
{
    
    return 0;
}
