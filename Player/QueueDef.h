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
  
  void enqueu(const T &t) {
    
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
