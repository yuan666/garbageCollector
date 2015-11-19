#include <iostream>
//#include <new>
#include "gc.h"

using namespace std;

class MyClass{
private:
	int a,b;
public:
	double val;
	MyClass(){a=b=0;}
	MyClass(int x,int y):a(x),b(y){val=0.0;}
	~MyClass(){cout<<"Destructing MyClass("<<a<<", "<<b<<")"<<endl;}
	int sum(){return a+b;}
	friend ostream& operator<<(ostream& os,MyClass& obj){
		os<<"("<<obj.a<<" "<<obj.b<<")";
		return os;
	}
};
class LoadTest{
int a,b;
public:
	double n[100000];
	double val;
	LoadTest(){a=b=0;}
	LoadTest(int x,int y):a(x),b(y){val=0.0;}
	friend ostream& operator<<(ostream& os,LoadTest& obj){
		os<<"("<<obj.a<<" "<<obj.b<<")";
		return os;
	}
};

int main(){
 GCPtr<int> p;
 GCPtr<int>q;

 try{
  cout<<"������������:\n";
  p=new int(110);
  q=new int(120);
  cout<<"Value at p is: "<<*p<<endl;
  cout<<"Value at q is: "<<*q<<endl;

  cout<<"�������:\n";
  GCPtr<int,10>ap=new int[10];
  for(int i=0;i<10;i++)
	  ap[i]=i;
  for(i=0;i<10;i++)
	  cout<<ap[i]<<" ";
  cout<<endl;

  cout<<"����������:\n";
  GCPtr<int,10>bp=new int[10];
  for(i=0;i<10;i++)
	  bp[i]=i;
  GCPtr<int>::GCiterator iter;
  for(iter=bp.begin();iter<bp.end()-1;++iter)
	  cout<<*iter<<"-->";
  cout<<*iter<<endl;

  cout<<"�ֶ������������պ�������:\n";
  GCPtr<int> x=new int(1);
  x=new int(2);
  x=new int(3);
  x=new int(4);
  GCPtr<int>::collect();  //�ֶ������������պ���
  cout<<"Value at x is: "<<*x<<endl;

  cout<<"��{}����:\n";
  cout<<"Before entering block.\n";
  {
	GCPtr<int>r=new int(119);
	cout<<"Value at r is: "<<*r<<endl;
  }
  cout<<"After exiting block.\n";

  cout<<"�Զ��������:\n";
  GCPtr<MyClass>obj=new MyClass(10,20);
  cout<<*obj<<endl;
  obj=new MyClass(100,200);
  cout<<*obj<<endl;
  cout<<"Sum is: "<<obj->sum()<<endl;  //obj��ָ��
  obj->val=1000;
  cout<<"obj->val="<<obj->val<<endl;
  cout<<"now obj is: "<<*obj<<endl;

 }catch(bad_alloc exc){
  cout<<"Allocation failure!\n";
  return 1;
 }
 cout<<"Done.\n\n";

////���������¹��̲������ã�����������Windows�Ѿ����ڴ汣�����ƣ���������ǿ���ڴ������
// cout<<"��ʾ�ظ���������ڴ棬�������ڴ�ľ�������ʾ����collect()���ղ���ʹ���ڴ�:\n";
//  GCPtr<LoadTest> tp;
//  for(int i=0;i<20000;i++){
//  try{
//	tp=new LoadTest(i,i);
//  }catch(bad_alloc ba){
//	//�������쳣��ʱ��Ҳ�����ڴ�����ʱ��
//	cout<<"Last object: "<<*tp<<endl;
//	cout<<"Length of gclist before calling collect():"<<tp.gclistSize()<<endl;
//	GCPtr<LoadTest>::collect();
//	cout<<"Length of gclist after calling collect():"<<tp.gclistSize()<<endl;
//  }
//  }
 return 0;
}