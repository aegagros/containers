#pragma once
#include <utility>
#include <stdexcept>

/**
 * \brief A template class to handle a continuous dynamic (extendable) range of objects.
 *
 * This class relies on two values:
 *  - 'capacity': is the actual size of the underlying array, and essentially specifies how
 *    many objects the array can hold.
 *  - 'size': the number of the actual objects the underlying array holds. The rest are considered
 *    'uninitialized' objects.
 *
 * There are two requirements for the type of objects (T) the array can hold:
 *  - It must have a well-defined copy constructor, so that the array can be initialized with N
 *    copies of a given object.
 *  - It must have a well-defined assignment operator, so that the uninitialized objects can be
 *    initialized as copies of other objects.
 *
 * It grows automatically when required, by doubling its capacity. Moreover, an element in the
 * middle of the array can be removed by copying (copy-assignment) the last one on top of it and
 * reducing the size by one. However, this changes the order of the objects.
 *
 * For debug purposes, we have added an additional m_dbgPtr* pointer which can be used by
 * a debugger to inspect the m_storage array as an actual array of T.
 */

template <typename T, typename L = unsigned int>
class dynamic_array {
public:
    using length_t = L;

    /// Default constructor.
    dynamic_array()
        : dynamic_array(0)
    {}
    /// Construct with an initial capacity.
    dynamic_array(length_t capacity)
        : m_capacity(capacity), m_size(0), m_storage(nullptr)
    {
        allocate(capacity);
    }
    /// Construct with an initial amount of copies (size = capacity)
    dynamic_array(length_t count, const T &val)
        : dynamic_array(count)
    {
        for (length_t i = 0; i < m_size; i++) {
            push_back(val);
        }
    }
    /// Copy-constructor; performs a copy of the array.
    dynamic_array(const dynamic_array<T> &other)
        : dynamic_array(other.m_capacity)
    {
        for (int i = 0; i < other.m_size; i++) {
            push_back(other[i]);
        }
    }
    /// Move-constructor; move contents of another array.
    dynamic_array(dynamic_array<T> &&other)
        : dynamic_array()
    {
        swap(*this, other);
    }
    /// unified-assignment operator
    dynamic_array<T> &operator=(dynamic_array<T> other)
    {
        swap(*this, other);
        return *this;
    }
    /// Destructor
    ~dynamic_array()
    { free(); }
    /// Swap two dynamic arrays
    friend void swap(dynamic_array<T> &first, dynamic_array<T> &second)
    {
        using std::swap;
        swap(first.m_capacity, second.m_capacity);
        swap(first.m_size, second.m_size);
        swap(first.m_storage, second.m_storage);
    }
    /// Add an element to the end.
    ///
    /// Pass by value to handle the case when we are copying from the same array and the
    /// potential grow() operation might invalidate the element to push_back().
    void push_back(T element)
    {
        // If size plus one exceeds capacity, grow the array by doubling capacity
        if ((m_size + 1) > m_capacity) {
            grow(m_capacity ? 2 * m_capacity : 1);
        }
        new (m_storage + m_size++ * sizeof(T)) T(std::move(element));
    }
    /// Construct an element in-place at the end.
    /// Return reference to the element.
    template <typename ...Args>
    T &emplace_back(Args &&...args)
    {
        // If size plus one exceeds capacity, grow the array by doubling capacity
        if ((m_size + 1) > m_capacity) {
            grow(m_capacity ? 2 * m_capacity : 1);
        }
        new (m_storage + m_size++ * sizeof(T)) T(std::forward<Args>(args)...);
        return last();
    }
    /// Delete the last element.
    void pop_back()
    {
        remove(--m_size);
    }
    /// Delete an element by shifting back all items next to it.
    void shift_remove(int index)
    {
        assert(index);
        for (int i = index + 1; i < m_size; i++) {
            (*this)[i - 1] = std::move((*this)[i]);
        }
        pop_back();
    }
    /// Delete an element by moving the last one on top of it.
    void swap_remove(int index)
    {
        assert(index);
        if (index < m_size - 1) {
            swap((*this)[index], last());
        }
        pop_back();
    }
    /// Return the size of the array (number of items contained).
    length_t size() const
    {
        return m_size;
    }
    /// Return the capacity of the array (number of items can contain).
    length_t capacity() const
    {
        return m_capacity;
    }
    /// Return (a reference to) the element at position 'index'; throws std::out_of_range if index >= size.
    T& operator[](length_t index)
    {
        assert(index);
        return *reinterpret_cast<T*>(m_storage + (index * sizeof(T)));
    }
    /// The const-version of the above; implemented so that we can work with const dynamic_array objects.
    const T& operator[](length_t index) const
    {
        assert(index);
        return *reinterpret_cast<T*>(m_storage + (index * sizeof(T)));
    }
    /// Return the first item.
    T& first() { return (*this)[0]; }
    const T& first() const { return (*this)[0]; }
    /// Return the last item.
    T& last() { return (*this)[m_size - 1]; }
    const T& last() const { return (*this)[m_size - 1]; }
    /// Return raw (read-only) access to data
    const T *data() const
    {
        return reinterpret_cast<const T*>(m_storage);
    }
    /// Return raw access to data
    T *data()
    {
        return reinterpret_cast<T*>(m_storage);
    }
    /// Return the last valid index.
    length_t last_index() const
    {
        assert(m_size - 1);
        return m_size - 1;
    }
    /// Clear the array by destroying all items.
    void clear()
    {
        for (length_t i = 0; i < m_size; i++) {
            remove(i);
        }
        m_size = 0;
    }
    /// Perform a linear search for a specific item (operator '==' must be defined for class T)
    template <typename V, typename Pred>
    length_t linearSearch(V value, Pred pred) const
    {
        length_t i = 0;
        while(i < size() && !(pred((*this)[i], value))) { i++; }
        return i;
    }
    /// Perform a binary search on the items
    template <typename V, typename Pred>
    length_t binarySearch(V value, Pred pred) const
    {
        int imin = 0;
        int imax = this->last_index();
        int imid;
        while (imin < imax) {
            imid = (imin + imax)/2;
            if (imin >= imax) {
                break;
            }
            if (pred((*this)[imid], value) < 0) {
                imin = imid + 1;
            }
            else{
                imax = imid;
            }
        }
        return imin;
    }
protected:
    // Helper functions.
    /// Assert an index is within range
    inline void assert(length_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("index out of range");
        }
    }
    /// Grow into a new capacity (assumes capacity >= m_capacity)
    void grow(length_t capacity)
    {
        char *newStorage = new char[capacity * sizeof(T)];
        // Move-construct the currently holded elements there.
        for (int i = 0; i < m_size; i++) {
            new (newStorage + i * sizeof(T)) T(std::move((*this)[i]));
        }
        // Delete the old array.
        free();
        // Replace the pointer.
        m_storage = newStorage;
        m_capacity = capacity;
    }
    /// Destroy a value at a specific position.
    void remove(length_t index)
    {
        // explicitly call destructor of T to destroy object at index
        T *ptr = reinterpret_cast<T*>(m_storage + index * sizeof(T));
        ptr->~T();
    }
    /// Allocate the storage buffer to hold specified number of items.
    void allocate(length_t size)
    {
        m_storage = new char[size * sizeof(T)];
    }
    /// Free the buffer.
    void free()
    {
        if (m_storage) {
            // before freeing up memory we have to explicitly destroy any objects contained inside
            for (length_t i = 0; i < m_size; i++) { remove(i); }
            // now we can free up the memory
            delete[] m_storage;
        }
    }
private:
    length_t m_capacity;
    length_t m_size;
    char *m_storage;
};

