/* 单线程的C++垃圾回收器
 *张远
 *2015-9-27
 */
//A single -threaded garbage collector
#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include <iomanip>

using namespace std;
//To watch the action of garbagecollector,please define DISPLAY
//#define DISPLAY
//Exception thrown when an attempt is made to use an Iter that exceeds the range of the underlying object.

//
class OutOfRangeExc{
	//Add functionality if needed by your application
};

//An iterator-like class for cycling through arrays that are pointed to by GCPtrs.Iter pointers do not 
//participate in or affect garbage collection.Thus,an Iter pointing to some object does not prevent that 
//object from being recycing
/*定义一个类似于迭代器iterator的Iter类，用于由垃圾指针GCptrs指向数组。Iter类指针并不参与或影响垃圾回收。因此，
 *Iter指向的对象不能解决循环引用问题。
*/
template <class T>class Iter{
T* ptr;  //current pointer value,指向当前对象
T* end;   //points to element one past end.指向最后一个元素下一个位置
T* begin;  //points to start of allocated array.指向首元素
unsigned length;  //length of sequence
public:
	Iter(){
	 ptr=end=begin=NULL;
	 length=0;
	}

	Iter(T* p,T* first,T* last){
	 ptr=p;
	 end=last;
	 begin=first;
	 length=last-first;
	}

	//Return length of sequence to which this Iter points.
	unsigned size(){return length;}

	//Return value pointed to by ptr.Do not allow outOfBound access
	T& operator*(){
	 if(ptr>=end||ptr<begin)
		 throw OutOfRangeExc();
	 return *ptr;
	}

	//Return address pointed bo by ptr.Do not allow outOfBound access
	T* operator->(){
	 if(ptr>=end||ptr<begin)
		throw OutOfRangeExc();
  	 return ptr;
	}

	//prefix ++
    Iter operator++(){
	 ptr++;
	 return *this;
	}
	//prefix --
	Iter operator--(){
	 ptr--;
	 return *this;
	}
	//postfix ++
	Iter operator++(int){
	 T* tmp=ptr;
	 ptr++;
	 return Iter<T>(tmp,begin,end);
	}
	//postfix --
	Iter operator--(int){
	 T* tmp=ptr;
	 ptr--;
	 return Iter(tmp,begin,end);
	}

	//Return a reference to the object at the specified index.Do not allow outOfBounds access.
	T& operator[](int i){
	if(i<0||i>=(end-begin))
		throw OutOfRangeExc();
	return ptr[i];
	}

	//Define the relational operators
	bool operator ==(Iter op2){
	 return ptr==op2.ptr;
	}
	bool operator!=(Iter op2){
	 return ptr!=op2.ptr;
	}
	bool operator<(Iter op2){
	 return ptr<op2.ptr;
	}
	bool operator<=(Iter op2){
	 return ptr<=op2.ptr;
	}
	bool operator>(Iter op2){
	 return ptr>op2.ptr;
	}
	bool operator>=(Iter op2){
	 return ptr>=op2.ptr;
	}

	//Subtract an integer from an Iter
	Iter operator-(int n){
	 ptr-=n;
	 return *this;
	}
	//Add an integer from an Iter
	Iter operator+(int n){
	 ptr+=n;
	 return *this;
	}

	//Return number of elements between two Iters
	int operator-(Iter<T>& itr2){
	return ptr-itr2.ptr;
	}
};

//This class defines an element that is stored in the garbage collection information list.
template <class T>class GCInfo{
public:
	unsigned refcount;  //current reference count 引用计数
	T* memPtr;    //pointer to allocated memtory，指向分配的内存
	bool isArray;  //isArray is true if memPtr points to an allocated array.It is false otherwise.
	unsigned arraySize;  //size of array
	//Here,mPtr points to the allocated memory.If this is an array,then size specifices the size of the array
	GCInfo(T* mPtr,unsigned size=0){
	 refcount=1;
	 memPtr=mPtr;
	 if(size!=0)
		 isArray=true;
	 else 
		 isArray=false;
	 arraySize=size;
	}
};

//Overloading operator== allows GCInfos to be compared. This is needed by the STL list class.
template <class T> bool operator==(const GCInfo<T>& ob1,const GCInfo<T>& ob2){
 return (ob1.memPtr==ob2.memPtr); 
}

//GCPtr implements a pointer type that uses garbage collection to release unused memory.
//A GCPtr must only be used to point to memory that was dymically allocated using new.
//When used to refer to an allocated array,specofy the size.
template <class T ,int size=0>class GCPtr{
//gclist maintains the garbage collection list.
static list<GCInfo<T> > gclist;
//addr points to the allocated memory to which this GCPtr pointer currently points.
T* addr;
//isArray is true if this GCPtr points to array .This is false otherwise.
bool isArray;
//If this GCPtr is pointing to an allocated array,then arraySize contains its size
unsigned arraySize;
static bool first;  //true when first GCPtr is created

//Retrun an iterator to pointer info in gclist
typename list<GCInfo<T> >::iterator findPtrInfo(T* ptr);
public:
	//Define an iterator type for GCPtr<T>
	typedef Iter<T> GCiterator;

	//Construct both initialized and uninitialized objects
	GCPtr(T* t=NULL){
	 //Register shutdown() as an exit function
	 if(first) atexit(shutdown);
	 first=false;
	 list<GCInfo<T> >::iterator p;
	 p=findPtrInfo(t);
	 //if t is already in gclist,then increment its reference count.Otherwise,add it to the list.
 	 if(p!=gclist.end())
		p->refcount++;
	 else{
	   GCInfo<T> gcObj(t,size);
	    gclist.push_front(gcObj);
	 }
	  addr=t;
	  arraySize=size;
	  if(size>0) isArray=true;
	  else isArray=false;
     #ifdef DISPLAY
      cout<<"Constructing GCPtr. ";
  	  if(isArray)
		cout<<"Size is "<<arraySize<<endl;
	   else
		cout<<endl;
      #endif
	}

	//Copy Constructor.
	GCPtr(const GCPtr &ob){
		list <GCInfo<T> > ::iterator p;
		p=findPtrInfo(ob.addr);
		p->refcount++: //increment ref_count
		addr=ob.addr;
		arraySie=ob.arraySize;
		if(arraySize>0) isArray=true;
		else isArray=false;
       #ifdef DISPLAY
		cout<<"Constructing Copy.";
		if(isArray)
			cout<<"Size is "<<arraySize<<endl;
		else
			cout<<endl;
       #endif
	}

  //Destructor for GCPtr.
	~GCPtr();

	 //Collect garbage. Return true if at least one object was freed.
	static bool collect();

	//Overload assignment of pointer to GCPtr
	T* operator=(T* t);

	//Overload assignment of GCPtr to GCPtr
	GCPtr& operator=(GCPtr& rv);

	//Return a reference to the object pointed to by this GCPtr
	T& operator*(){
	 return * addr;
	}
	//Return the address being pointed to
	T* operator->(){
	 return addr;
	}
	//Return a reference to the object at the index specified by i
	T& operator[](int i){
	 return addr[i];
	}
	//Conversion function to T*
	operator T*(){return addr;}
	//Return an Iter to the start of the allocated memory
	Iter<T> begin(){
	 int size;
	 if(isArray) size=arraySize;
	 else size=1;
	 return Iter<T>(addr,addr,addr+size);
	}

	//Return an Iter to the one past the end of an allocated memory
	Iter<T> end(){
	 int size;
	 if(isArray) size=arraySize;
	 else size=1;
	 return Iter<T>(addr+size,addr,addr+size);
	}

	//Return the size of gclist for this type of GCPtr 
	static int gclistSize(){return gclist.size();}

	//A utility function that displays gclist
	static void showlist();

	//Clear gclist when program exit
	static void shutdown();	
};

//Creates storage for the static variables 
//静态变量定义，及初始化
template <class T,int size>
 list<GCInfo<T> > GCPtr<T,size>::gclist;   
template <class T,int size>
 bool GCPtr<T,size>::first=true;

//Destructor for GCPtr
template <class T,int size>
 GCPtr<T,size>:: ~GCPtr( ){
  list<GCInfo<T> >::iterator p;
  p=findPtrInfo(addr);
  if(p->refcount) p->refcount--;
  #ifdef DISPLAY
   cout<<"GCPtr going out of scope\n";
  #endif
 //Collect garbage when a pointer goes out of scope
  collect();
}

//Colect garbage.Returns true if at least one object was freed
template <class T,int size>
 bool GCPtr<T,size>::collect(){
  bool memfreed=false;
  #ifdef DISPLAY
   cout<<"---------------------------------------------\n";
   cout<<"* * * * * * * * * * * * * * * * * * * * * * *\n";
   cout<<"Before garbage collecntion for ";
   showlist();
  #endif
  list<GCInfo<T> >::iterator p;
  do{
   //Scan gclist looking for unreference pointers
	  for(p=gclist.begin();p!=gclist.end();p++){
	   //if in-use skip
		if(p->refcount>0) continue;
		memfreed=true;
		//Free memeory unless the GCPtr is null
		if(p->memPtr){
			if(p->isArray){
             #ifdef DISPLAY
		       cout<<"\nDeleting array of size "<<p->arraySize<<endl<<endl;
             #endif
			 delete []p->memPtr;
			}
			else{	
             #ifdef DISPLAY
			  cout<<"\nDeleting:"<<*(T* )p->memPtr<<"\n\n";
             #endif
			 delete p->memPtr;
			}
		}
		//Remove unused entry from gclist
		gclist.remove(*p);
		break;  //Restart the search
	  }
  }while(p!=gclist.end());
  #ifdef DISPLAY
    cout<<"After garbage collection for";
    showlist();
	//cout<<"* * * * * * * * * * * * * * * * * * * * * * *\n";
	cout<<"==============================================\n";
  #endif
  return memfreed;
}

//Overload assignment of pointer to GCPtr
template <class T,int size>
 T* GCPtr<T,size>::operator =(T* t){
	list<GCInfo<T> >::iterator p;
	//First,decrease the reference count.for the memory currently being pointed to
	p=findPtrInfo(addr);
	p->refcount--;
	//Next,if the new address is already existent int the system,increment its count.
	//Otherwise ,create a new entry for gclist
	p=findPtrInfo(t);
	if(p!=gclist.end())
		p->refcount++;
	else{
	 GCInfo<T> gcObj(t,size);
	 gclist.push_front(gcObj);
	}
	addr=t; //Store the address
	return t;
}

//Overload assignment of GCPtr to GCPtr
template <class T,int size>
 GCPtr<T,size>& GCPtr<T,size>::operator =(GCPtr& rv){
	list<GCInfo<T> >::iterator p;
	//First,decrement the reference count for the memory currently being pointed to
	p=findPtrInfo(addr);
	p->refcount--;
	//Next,increment the reference count of the new address
	p=findPtrInfo(rv.addr);
    p->refcount++:
	addr=rv.addr;
	return rv;
}

//A utility function that display gclist
template <class T,int size>
void GCPtr<T,size>::showlist(){
	list<GCInfo<T> >::iterator p;
	cout<<"gclist<"<<typeid(T).name()<<"," <<size<<">:\n";
	cout<<"memPtr       refcount   value\n";
	if(gclist.begin()==gclist.end()){
		cout<<"--Empty--\n";
		return;
	}
	for(p=gclist.begin();p!=gclist.end();p++){
	 cout<<"["<<(void*)p->memPtr<<"]"<<" "<<setw(6)<<p->refcount<<" ";
	 if(p->memPtr) cout<<setw(10)<<*p->memPtr;
	 else cout<<setw(10)<<"---";
	 cout<<endl;
	}
}

//Find a pointer in gclist 
template <class T,int size>
 typename list<GCInfo<T> >::iterator GCPtr<T,size>::findPtrInfo(T* ptr){
	list<GCInfo<T> >::iterator p;
	//Find ptr in the gclist
	for(p=gclist.begin();p!=gclist.end();p++)
		if(p->memPtr==ptr)
			return p;
	return p;
}

//Clear gclist when program exits
template <class T,int size>
 void GCPtr<T,size>::shutdown(){
  if(gclistSize()==0) return;
  list<GCInfo<T> >::iterator p;
  for(p=gclist.begin();p!=gclist.end();p++)
	  p->refcount=0;
  #ifdef DISPLAY
   cout<<"Before collenting for shutdown() for "<<typeid(T).name()<<"\n";
  #endif
  collect();
  #ifdef DISPLAY
   cout<<"After collecting for shutdown() for "<<typeid(T).name()<<"\n";
  #endif
}