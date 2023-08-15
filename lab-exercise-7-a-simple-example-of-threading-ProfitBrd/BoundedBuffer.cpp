#include "BoundedBuffer.h"

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (char* msg, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    // 3. Then push the vector at the end of the queue
    // 4. Wake up threads that were waiting for push
    
        
    //0. Convert the incoming byte sequence given by data and len into a vector<char>
    // char* data = (char*) msg;
    vector<char> d (msg, msg + size);

    //1. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    unique_lock<mutex> l(m);
    slot_available.wait (l, [this]{return (int)q.size() < cap;});
    
    
    //2. Then push the vector at the end of the queue, watch out for racecondition
    q.push (d);
    l.unlock ();
    
    //3. Wake up pop() threads 
    data_available.notify_one ();
}

int BoundedBuffer::pop (char* msg, int size) {
    // 1. Wait until the queue has at least 1 item
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    // 4. Wake up threads that were waiting for pop
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    //1. Wait until the queue has at least 1 item
    unique_lock<mutex> l (m);
    data_available.wait (l, [this]{return q.size() > 0;});
    
    //2. pop the front item of the queue. The popped item is a vector<char>, watch out for race condition
    vector<char> d = q.front ();
    q.pop ();
    l.unlock ();

    //3. Convert the popped vector<char> into a char*, copy that into buf, make sure that vector<char>'s length is <= bufcap
    assert ((int)d.size() <= size);
    memcpy (msg, d.data(), d.size());
        
    //4. wake up any potentially sleeping push() function
    slot_available.notify_one ();
    
    //5. Return the vector's length to the caller so that he knows many bytes were popped
    return d.size ();
}

size_t BoundedBuffer::size () {
    return q.size();
}
