#include <cstddef>
#include <concepts>
#include <stdexcept>

template <typename T>
concept HashSetKey = 
    std::equality_comparable<T>
    && std::default_initializable<T>
    && requires(T value) { { std::hash<T>{}(value) } -> std::convertible_to<size_t>; };

enum State { Occupied, Empty };

/**
 * @brief An entry in a set.
 * 
 * @tparam T The element type stored in the hash set.
 *          Must satisfy the HashSetKey concept.
 */
template <HashSetKey T>
struct Entry {
    
    T value;
    State state;
    size_t distance;

    /**
     * @brief Constructs an empty entry.
     */
    Entry(): state(Empty), distance(0) {}
};

/**
 * @brief A mutable robin hood implementation of a hash set.
 * 
 * @tparam T The element type stored in the hash set.
 *         Must satisfy the HashSetKey concept.
 * 
 * @invariant `capacity > 0`
 * @invariant `size <= capacity`
 * @invariant `data != nullptr`
 * @invariant `size` is always less than 85% of capacity.
 * @invariant `size` is always greater than 25% of capacity iff `capacity <= DEFAULT_CAPACITY`.
 * @invariant data points to an allocated array of exactly `capacity` elements `Entry<T>`.
 * @invariant For every occupied entry in data with index `i`,
 *            `(hash(value) & (capacity - 1) + distance) mod capacity == i`
 */
template <HashSetKey T>
class MyHashSet {

private:
    
    Entry<T>* data;
    size_t capacity;
    size_t size;

    static constexpr size_t DEFAULT_CAPACITY = 128;

public:

    /**
     * @brief Construct an empty set with the default capacity.
     * 
     * @throw std::bad_alloc if the allocation fails.
     */
    MyHashSet(): MyHashSet(DEFAULT_CAPACITY) {}

    /**
     * @brief Construct an empty set with the specified capacity.
     * 
     * @param capacity The initial number of elements that can be stored without reallocation.
     * @throw std::invalid_argument if `capacity == 0`.
     * @throw std::invalid_argument if capacity is not a power of 2.
     * @throw std::bad_alloc if the allocation fails.
     */
    MyHashSet(size_t capacity): capacity(capacity), size(0) {
        if (capacity == 0)
            throw std::invalid_argument("capacity must be greater than 0");
        if (capacity & (capacity - 1 != 0))
            throw std::invalid_argument("capacity must be multiple of 2");

        data = new Entry<T>[capacity];
    }

    /**
     * @note Copying is disabled because the container owns dynamically 
     *       allocated storage and does not implement deep-copy semantics.
     */
    MyHashSet(const MyHashSet&) = delete;

    /**
     * @note Copying is disabled because the container owns dynamically 
     *       allocated storage and does not implement deep-copy semantics.
     */
    MyHashSet<T>& operator=(const MyHashSet<T>&) = delete;

    /**
     * @note Move construction is disabled because this container's 
     *       invariants do not permit a moved-from state.
     */
    MyHashSet(const MyHashSet&&) = delete;

    /**
     * @note Move assignment is disabled because this container's 
     *       invariants do not permit a moved-from state.
     */
    MyHashSet<T>& operator=(const MyHashSet<T>&&) = delete;

    /**
     * @return The number of elements currently stored in the set.
     */
    size_t length() const {
        return size;
    }

    /**
     * @par Complexity
     *      Worst case O(n)
     *      Average case O(1)
     * 
     * @note Average-case complexity assumes a well-distributed hash function.
     * 
     * @return true if the value was successfully inserted in the set, otherwise false.
     * @throw std::bad_alloc if the allocation fails.
     * 
     * @post The length of this set is increased by 1 iff the function returns true.
     * @post The value is present in the set.
     * @post All values initially present in the set are still present.
     */
    bool insert(const T& value) {
        
        if (size * 100  >= capacity * 85)
            resize(capacity * 2);
        
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

    /**
     * @par Complexity
     *      Worst case O(n)
     *      Average case O(1)
     * 
     * @note Average-case complexity assumes a well-distributed hash function.
     * 
     * @return true if the value is present in the set, otherwise false.
     */
    bool contains(const T& value) const {

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

    /**
     * @par Complexity
     *      Worst case O(n)
     *      Average case O(1)
     * 
     * @note Average-case complexity assumes a well-distributed hash function.
     * 
     * @throw std::bad_alloc if the allocation fails.
     * @return true if the value was successfully removed, otherwise false.
     * 
     * @post The length of this set is decreased by 1 iff the function returns true.
     * @post `value` is not in the set.
     * @post All values different from `value` initially present in the set are still present.
     */
    bool remove(const T& value) {
        
        if (size * 100  < capacity * 25)
            resize(capacity / 2);

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

    /**
     * @par Complexity
     *      O(n)
     * 
     * @throw std::bad_alloc if the allocation fails.
     * 
     * @pre `new_capacity > 0`
     * @post All values initially present in the set are still present.
     * @post The capacity of this set is equal to new_capacity.
     */
    void resize(size_t new_capacity) {

        Entry<T>* new_data = new Entry<T>[new_capacity];

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
    }

};