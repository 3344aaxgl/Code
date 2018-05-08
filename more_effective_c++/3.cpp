#include <iostream>
#include <string>

using namespace std;

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

class BalanceBST : public BST
{
  private:
    string value;

  public:
    BalanceBST() : value("hello"){};
    ostream &operator<<(ostream &os)
    {
        //(dynamic_cast<BST&>(*this) << os) ;
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

int main()
{
    BalanceBST bt[10];
    //printBSTArray(cout,bt,10);
    cout<<sizeof(BST)<<" "<<sizeof(int)<<" "<<sizeof(BalanceBST)<<endl;
    return 0;
}