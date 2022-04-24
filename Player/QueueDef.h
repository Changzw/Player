//
//  QueueDef.h
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#ifndef QueueDef_h
#define QueueDef_h

#include <list>
#include <mutex>

using namespace std;
template <class T>
class MediaQueue: public list<T> {
public:
  MediaQueue() {
  }
  
  ~MediaQueue() {
  }
  
  void enqueue(const T &t) {
    m_mutex.lock();
    list<T>::push_back(t);
    m_mutex.unlock();
  }
  
  T dequeue() {
    m_mutex.lock();
    T t = NULL;
    if (!list<T>::empty()) {
      t = list<T>::front();
      list<T>::pop_front();
    }
    m_mutex.unlock();
    
    return t;
  }
  
  bool isEmpty() {
    m_mutex.lock();
    bool b = list<T>::empty();
    m_mutex.unlock();
    return b;
  }
  
private:
  mutex m_mutex;
};






#endif /* QueueDef_h */
