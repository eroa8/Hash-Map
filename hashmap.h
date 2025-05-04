#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;

template <typename KeyT, typename ValT>
class HashMap {
 private:
  struct ChainNode {
    const KeyT key;
    ValT value;
    ChainNode* next;

    ChainNode(KeyT key, ValT value) : key(key), value(value), next(nullptr) {
    }

    ChainNode(KeyT key, ValT value, ChainNode* next)
        : key(key), value(value), next(next) {
    }
  };

  ChainNode** data;
  size_t sz;
  size_t capacity;

  // Utility members for begin/next
  ChainNode* curr;
  size_t curr_idx;

 public:
  /**
   * Creates an empty `HashMap` with 10 buckets.
   */
  HashMap() {
    this->sz = 0;
    this->capacity = 10;

    // Create empty array with 10 buckets
    this->data = new ChainNode*[capacity];

    // Have to set each bucket to nullptr
    for (size_t i = 0; i < capacity; ++i) {
      this->data[i] = nullptr;
    }
  }

  /**
   * Creates an empty `HashMap` with `capacity` buckets.
   */
  HashMap(size_t capacity) {
    this->sz = 0;
    this->capacity = capacity;

    // Create empty array with the amount of buckets given (capacity)
    this->data = new ChainNode*[capacity];

    // have to set each bucket to nullptr
    for (size_t i = 0; i < capacity; ++i) {
      this->data[i] = nullptr;
    }
  }

  /**
   * Checks if the `HashMap` is empty. Runs in O(1).
   */
  bool empty() const {
    return sz == 0;
  }

  /**
   * Returns the number of mappings in the `HashMap`. Runs in O(1).
   */
  size_t size() const {
    return sz;
  }

  /**
   * Adds the mapping `{key -> value}` to the `HashMap`. If the key already
   * exists, does not update the mapping (like the C++ STL map).
   *
   * Uses the hash value of the key to determine the location in the
   * underlying hash table. Creates exactly one new node; resizes by doubling
   * when the load factor exceeds 1.5.
   *
   * On resize, doesn't create new nodes, but rearranges existing ones.
   *
   * Runs in O(L), where L is the length of the longest chain.
   */
  void insert(KeyT key, ValT value) {
    size_t index = std::hash<KeyT>{}(key) % capacity;
    ChainNode* current = data[index];

    // Check if the key already exists, do nothing
    while (current != nullptr) {
      if (current->key == key) {  // Comepares the keys
        return;
      }
      current = current->next;
    }

    // Otherwise insert new node at head of the chain
    ChainNode* new_node = new ChainNode(key, value, data[index]);
    data[index] = new_node;
    sz++;  // Increase size

    // Check load factor and resize if needed
    double load_factor =
        static_cast<double>(sz) / static_cast<double>(capacity);
    if (load_factor > 1.5) {
      rehash(capacity *
             2);  // Resize and rehash existing nodes (calls helper function)
    }
  }

  void rehash(size_t new_capacity) {
    ChainNode** old_data = data;
    size_t old_capacity = capacity;

    data = new ChainNode*[new_capacity];
    capacity = new_capacity;

    // Initialize new data array
    for (size_t i = 0; i < new_capacity; ++i) {
      data[i] = nullptr;
    }

    // Rehash all nodes
    for (size_t i = 0; i < old_capacity; ++i) {
      ChainNode* current = old_data[i];
      while (current != nullptr) {
        ChainNode* next = current->next;

        // Recalculate index for new hash
        size_t new_index = std::hash<KeyT>{}(current->key) % new_capacity;

        // Move node to new hash (insert at head)
        current->next = data[new_index];
        data[new_index] = current;

        current = next;
      }
    }

    delete[] old_data;  // Make no memory leaks
  }

  /**
   * Return a reference to the value stored for `key` in the map.
   *
   * If key is not present in the map, throw `out_of_range` exception.
   *
   * Runs in O(L), where L is the length of the longest chain.
   */
  ValT& at(const KeyT& key) const {
    size_t index = std::hash<KeyT>{}(key) % capacity;
    ChainNode* current = data[index];

    while (current != nullptr) {
      if (current->key == key) {
        return current->value;
      }
      current = current->next;
    }

    throw std::out_of_range("out_of_range");
  }

  /**
   * Returns `true` if the key is present in the map, and false otherwise.
   *
   * Runs in O(L), where L is the length of the longest chain.
   */
  bool contains(const KeyT& key) const {
    size_t index = std::hash<KeyT>{}(key) % capacity;
    ChainNode* current = data[index];

    while (current != nullptr) {
      if (current->key == key) {
        return true;
      }
      current = current->next;
    }

    return false;
  }

  /**
   * Empties the `HashMap`, freeing all nodes. The bucket array may be left
   * alone.
   *
   * Runs in O(N+B), where N is the number of mappings and B is the number of
   * buckets.
   */
  void clear() {
    for (size_t i = 0; i < capacity; ++i) {
      ChainNode* current = data[i];
      while (current != nullptr) {
        ChainNode* to_delete = current;
        current = current->next;
        delete to_delete;
      }
      data[i] = nullptr;  // reset head of chain
    }
    sz = 0;
  }

  /**
   * Destructor, cleans up the `HashMap`.
   *
   * Runs in O(N+B), where N is the number of mappings and B is the number of
   * buckets.
   */
  ~HashMap() {
    clear();        // Frees all nodes in the chains
    delete[] data;  // Frees the array of pointers
  }

  /**
   * Removes the mapping for the given key from the `HashMap`, and returns the
   * value.
   *
   * Throws `out_of_range` if the key is not present in the map. Creates no new
   * nodes, and does not update the key or value of any existing nodes.
   *
   * Runs in O(L), where L is the length of the longest chain.
   */
  ValT erase(const KeyT& key) {
    size_t index = std::hash<KeyT>{}(key) % capacity;
    ChainNode* current = data[index];
    ChainNode* prev = nullptr;

    while (current != nullptr) {
      if (current->key == key) {
        ValT val = current->value;

        if (prev == nullptr) {
          // Deleting head of chain
          data[index] = current->next;
        } else {
          prev->next = current->next;
        }

        delete current;
        sz--;  // subtract from size
        return val;
      }

      prev = current;
      current = current->next;
    }

    // If we got here, key wasn't found
    throw std::out_of_range("Key not found in erase");
  }

  /**
   * Copy constructor.
   *
   * Copies the mappings from the provided `HashMap`.
   *
   * Runs in O(N+B), where N is the number of mappings in `other`, and B is the
   * number of buckets.
   */
  HashMap(const HashMap& other) {
    capacity = other.capacity;
    sz = other.sz;

    // Step 1: allocate new array of same capacity
    data = new ChainNode*[capacity];
    for (size_t i = 0; i < capacity; ++i) {
      data[i] = nullptr;
    }

    // Step 2: deep copy each chain
    for (size_t i = 0; i < capacity; ++i) {
      ChainNode* current = other.data[i];
      ChainNode** ptr = &data[i];

      while (current != nullptr) {
        *ptr = new ChainNode(current->key, current->value);
        ptr = &((*ptr)->next);
        current = current->next;
      }
    }
  }

  /**
   * Assignment operator; `operator=`.
   *
   * Clears this table, and copies the mappings from the provided `HashMap`.
   *
   * Runs in O((N1+B1) + (N2+B2)), where N1 and B1 are the number of mappings
   * and buckets in `this`, and N2 and B2 are the number of mappings and buckets
   * in `other`.
   */
  HashMap& operator=(const HashMap& other) {
    if (this == &other) {
      return *this;  // protect against self-assignment
    }

    this->clear();
    delete[] this->data;

    this->capacity = other.capacity;
    this->sz = other.sz;

    this->data = new ChainNode*[capacity];
    for (size_t i = 0; i < capacity; ++i) {
      data[i] = nullptr;
    }

    for (size_t i = 0; i < capacity; ++i) {
      ChainNode* current = other.data[i];
      ChainNode** ptr = &data[i];

      while (current != nullptr) {
        *ptr = new ChainNode(current->key, current->value);
        ptr = &((*ptr)->next);
        current = current->next;
      }
    }

    return *this;
  }

  // =====================

  /**
   * Resets internal state for an iterative traversal.
   *
   * See `next` for usage details. Modifies nothing except for `curr` and
   * `curr_idx`.
   *
   * Runs in worst-case O(B), where B is the number of buckets.
   */
  void begin() {
    curr_idx = 0;
    curr = nullptr;

    // Loop through the buckets to find the first non-empty one
    while (curr_idx < capacity && data[curr_idx] == nullptr) {
      curr_idx++;
    }

    if (curr_idx < capacity) {
      curr = data[curr_idx];
    }
  }

  /**
   * Uses the internal state to return the "next" key and value
   * by reference, and advances the internal state. Returns `true` if the
   * reference parameters were set, and `false` otherwise.
   *
   * Example usage:
   *
   * ```c++
   * HashMap<string, int> hm;
   * hm.begin();
   * string key;
   * int value;
   * while (hm.next(key, val)) {
   *   cout << key << ": " << val << endl;
   * }
   * ```
   *
   * Does not visit the mappings in any defined order.
   *
   * Modifies nothing except for `curr` and `curr_idx`.
   *
   * Runs in worst-case O(B) where B is the number of buckets.
   */
  bool next(KeyT& key, ValT& value) {
    if (curr == nullptr) {
      return false;
    }

    key = curr->key;
    value = curr->value;

    curr = curr->next;

    while (curr == nullptr && ++curr_idx < capacity) {
      curr = data[curr_idx];
    }

    return true;
  }

  // ===============================================

  /**
   * Returns a pointer to the underlying memory managed by the `HashMap`.
   * For autograder testing purposes only.
   */
  void* get_data() const {
    return this->data;
  }

  /**
   * Returns the capacity of the underlying memory managed by the `HashMap`. For
   * autograder testing purposes only.
   */
  size_t get_capacity() {
    return this->capacity;
  }
};
