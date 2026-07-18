#include <cstddef>
#include <concepts>
#include <stdexcept>

template <typename T>
concept HashSetKey = 
    std::equality_comparable<T>
    && std::default_initializable<T>
    && requires(T value) { { std::hash<T>{}(value) } -> std::convertible_to<size_t>; };

enum State { Occupied, Empty };

template <HashSetKey T>
struct Entry {
    
    T value;
    State state;
    size_t distance;

    Entry(): state(Empty), distance(0) {
        // default: state is empty and distance is 0
        // distance and value are to be changed by a later insertions
    }
};

template <HashSetKey T>
class MyHashSet {

private:
    
    Entry<T>* data;
    size_t capacity;
    size_t size;

    static constexpr size_t DEFAULT_CAPACITY = 128;

public:

    MyHashSet(): MyHashSet(DEFAULT_CAPACITY) {
        // constructor with default capacity (128 elements)
    }

    MyHashSet(size_t capacity): capacity(capacity), size(0) {
        // constructor with given capacity
        // @throws invalid argument exception if capacity is null
        // @throws invalid argument excption if capacity is not a power of 2
        // @param capacity -> capacity for the HashSet, capacity must be multiple of 2 
        if (capacity == 0)
            throw std::invalid_argument("capacity must be greater than 0");
        if (capacity & (capacity - 1 != 0))
            throw std::invalid_argument("capacity must be multiple of 2");

        data = new Entry<T>[capacity];
    }

    size_t length() {
        return size;
    }

    bool insert(const T& value) {
        
        if (size * 100  >= capacity * 85)
            if (!resize(capacity * 2))
                return false;
        
        T current_value = value;
        size_t hash = std::hash<T>{}(value);
        size_t idx = hash & (capacity - 1);
        size_t distance = 0;

        while (true) {
            if (data[idx].state == Empty) {
                data[idx].value = value;
                data[idx].state = Occupied;
                data[idx].distance = distance;
                size += 1;
                return true;
            } else if (data[idx].state == Occupied && data[idx].value == value) {
                return false;
            } else if (data[idx].state == Occupied && data[idx].distance < distance) {
                std::swap(current_value, data[idx].value);
                std::swap(distance, data[idx].distance);
            } else {
                idx = (idx + 1) & (capacity - 1);
                distance += 1;
            }
        }
    }

    bool contains(const T& value) {

        size_t hash = std::hash<T>{}(value);
        size_t idx = hash & (capacity - 1);
        size_t distance = 0;

        for (size_t i = idx; i < idx + capacity; i++) {

            const Entry<T>& entry = data[i & (capacity - 1)];

            if (entry.state == Empty)
                return false;
            
            if (entry.state == Occupied && entry.value == value)
                return true;
            
            if (entry.state == Occupied && entry.distance < distance)
                return false;
            
            distance += 1;
        }
        
        return false;
    }

    bool remove(const T& value) {
        
        if (size * 100  < capacity * 25)
            if (!resize(capacity / 2))
                return false;

        size_t hash = std::hash<T>{}(value);
        size_t idx = hash & (capacity - 1);
        size_t distance = 0;

        for (size_t i = idx; i < idx + capacity; i++) {

            const Entry<T>& entry = data[i & (capacity - 1)];

            if (entry.state == Empty)
                return false;
            
            if (entry.state == Occupied && entry.value == value) {
                size_t j = (i + 1) & (capacity - 1);
                data[i & (capacity - 1)].state = Empty;
                size -= 1;

                while (data[j].state == Occupied && data[j].distance > 0) {
                    std::swap(data[(j - 1) & (capacity - 1)], data[j]);
                    data[(j - 1) & (capacity - 1)].distance -= 1;
                    j = (j + 1) & (capacity - 1);
                }

                return true;
            }
            
            if (entry.state == Occupied && entry.distance < distance)
                return false;
            
            distance += 1;
        }
        
        return false;
    }

    ~MyHashSet() {
        delete[] data;
    }

private:

    bool resize(size_t new_capacity) {

        Entry<T>* new_data = new Entry<T>[new_capacity];
        if (new_data == nullptr)
            return false;

        for (size_t i = 0; i < capacity; i++) {
            
            if (data[i].state == Empty)
                continue;

            T current_value = data[i].value;
            size_t hash = std::hash<T>{}(current_value);
            size_t idx = hash & (new_capacity - 1);
            size_t distance = 0;

            while (true) {
                if (new_data[idx].state == Empty) {
                    new_data[idx].value = current_value;
                    new_data[idx].state = Occupied;
                    new_data[idx].distance = distance;
                    break;
                } else if (new_data[idx].state == Occupied && new_data[idx].distance < distance) {
                    std::swap(current_value, new_data[idx].value);
                    std::swap(distance, new_data[idx].distance);
                } else {
                    idx = (idx + 1) & (new_capacity - 1);
                    distance += 1;
                }
            }
        }

        Entry<T>* old_data = data;
        data = new_data;
        capacity = new_capacity;
        delete[] old_data;
        return true;
    }
};