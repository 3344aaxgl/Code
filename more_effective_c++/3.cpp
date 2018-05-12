#include <iostream>
#include <string>
#include <vector>

using namespace std;

template<typename T,int n = 10>
class refer
{
	private:
	    T arr[n];
    public:
	    refer(const T& t)
		{
			for(int i = 0; i< n; i++)
			    arr[i] = t;
		}

		T& operator[](int i)
		{
			return arr[i];
		}

};

void fun(char *p)
{
    if (p != NULL)
        cout<< *p <<'\n';
}

void fun(char &c)
{
    cout<< c<<'\n';
}

//------------------------------------------------

struct C {
    int m = 0;
    void hello() const {
        std::cout << "Hello world, this is B!\n";
    }
};
struct F : C {
    void hello() const {
        std::cout << "Hello world, this is D!\n";
    }
};
 
enum class E { ONE = 1, TWO, THREE };
enum EU { ONE = 1, TWO, THREE };

//------------------------------------------------

struct V {
    virtual void f() {};  // 必须为多态以使用运行时检查的 dynamic_cast
};
struct A : virtual V {};
struct B : virtual V {
  B(V* v, A* a) {
    // 构造中转型（见后述 D 的构造函数中的调用）
    dynamic_cast<B*>(v); // 良好定义： v 有类型 V* ， B 的 V 基类，产生 B*
    dynamic_cast<B*>(a); // 未定义行为： a 有类型 A* ， A 非 B 的基类
  }
};
struct D : A, B {
    D() : B((A*)this, this) { }
};
 
struct Base {
    virtual ~Base() {}
};
 
struct Derived: Base {
    virtual void name() {}
};

//------------------------------------------------
class BST
{
  private:
    int data;

  public:
    BST() : data(0){};
   virtual ostream &operator<<(ostream &os)
    {
        os << "BST.data:" << data << ' ';
        return os;
    }
};

//------------------------------------------------

class EquipmentPiece {
public:
EquipmentPiece(int IDNumber):data(IDNumber){};
private:
  int data;
};
class BalanceBST : public BST
{
  private:
    string value;

  public:
    BalanceBST() : value("hello"){};
    ostream &operator<<(ostream &os)
    {
        os << " BalanceBST.value: " << value<<'\n';
        return os;
    }
};

void printBSTArray(ostream& os, BST bBSTArray[], int numElements)
{
    for (int i = 0; i < numElements; i++) {
     bBSTArray[i] << os;
    }
}

template <typename T>
class Array
{
    public:
      Array(int s);
      ~Array();
    private:
      int size;
      T* data;
};

template <typename T>
Array<T>::Array(int s):size(s)
{
    data = new T[size];
}

template <typename T>
Array<T>::~Array()
{
    delete [] data;
}

int main()
{

    //item1 指针与引用的区别
    {
        char *p;
        char c = 'A';
        char &d = c;

        p = &c;

        fun(p);
        fun(d);

        refer<int> ref(1);
        cout << ref[0] << '\n';
        ref[0] = 2;
        cout << ref[0] << '\n';
        return 0;
    }
    //item2 尽量使用C++风格的类型转换
    {
        // 1: 初始化转换
        int n = static_cast<int>(3.14);
        std::cout << "n = " << n << '\n';
        std::vector<int> v = static_cast<std::vector<int>>(10);
        std::cout << "v.size() = " << v.size() << '\n';

        // 2: 静态向下转型
        F f;
        C &br = f; // 通过隐式转换向上转型
        br.hello();
        F &another_d = static_cast<F &>(br); // 向下转型
        another_d.hello();
        C cr;
        F another_f = static_cast<F &>(cr);//static_cast可以转换，dynamic_cast不行
        //another_f = dynamic_cast<F &>(cr);

        // 3: 左值到亡值
        std::vector<int> v2 = static_cast<std::vector<int> &&>(v);
        std::cout << "after move, v.size() = " << v.size() << '\n';

        // 4: 弃值表达式
        static_cast<void>(v2.size());

        // 5. 隐式转换的逆
        void *nv = &n;
        int *ni = static_cast<int *>(nv);
        std::cout << "*ni = " << *ni << '\n';

        // 6. 数组到指针后随向上转型
        F fa[10];
        C *dp = static_cast<C *>(fa);

        // 7. 有作用域枚举到 int 或 float
        E e = E::ONE;
        int one = static_cast<int>(e);
        std::cout << one << '\n';

        // 8. int 到枚举，枚举到另一枚举
        E e2 = static_cast<E>(one);
        EU eu = static_cast<EU>(e2);

        // 9. 指向成员指针向上转型
        int F::*pm = &F::m;
        std::cout << br.*static_cast<int C::*>(pm) << '\n';

        // 10. void* 到任何类型
        void *voidp = &e;
        std::vector<int> *p = static_cast<std::vector<int> *>(voidp);

        D d;                             // 最终导出类
        A &a = d;                        // 向上转型，可以用 dynamic_cast ，但不必须
        D &new_d = dynamic_cast<D &>(a); // 向下转型
        B &new_b = dynamic_cast<B &>(a); // 侧向转型

        Base *b1 = new Base;
        if (Derived *d = dynamic_cast<Derived *>(b1)) //转换失败
        {
            std::cout << "downcast from b1 to d successful\n";
            d->name(); // 调用安全
        }

        Base *b2 = new Derived;
        if (Derived *d = dynamic_cast<Derived *>(b2)) //转换成功,必须要有虚函数
        {
            std::cout << "downcast from b2 to d successful\n";
            d->name(); // 调用安全
        }

        delete b1;
        delete b2;
    }

    BalanceBST bt[10];
   // printBSTArray(cout,bt,10);
  
    int ID1, ID2, ID3, ID4,ID5,ID6,ID7,ID8,ID9, ID10; // 存储设备ID号的变量

    EquipmentPiece bestPieces[] = { EquipmentPiece(ID1), 
    EquipmentPiece(ID2),
    EquipmentPiece(ID3),
    EquipmentPiece(ID4),
    EquipmentPiece(ID5),
    EquipmentPiece(ID6),
    EquipmentPiece(ID7),
    EquipmentPiece(ID8),
    EquipmentPiece(ID9),
    EquipmentPiece(ID10)
    };

   EquipmentPiece* worstPrices[10];
   for(int i = 0;i < 10; i++)
       worstPrices[i] = new EquipmentPiece(i);
   for(int i = 0;i < 10; i++)
       delete worstPrices[i];

    void * rawMemory = operator new(sizeof(EquipmentPiece) * 10);
    EquipmentPiece* bestPieces1 = static_cast<EquipmentPiece*>(rawMemory);
    for(int i = 0;i < 10; i++)
       new(&bestPieces1[i]) EquipmentPiece(i);
    for(int i = 0;i < 10; i++)
       bestPieces1[i].~EquipmentPiece();
    operator delete [](rawMemory);
    
    Array<int> arr(10); 

    
    return 0;
}