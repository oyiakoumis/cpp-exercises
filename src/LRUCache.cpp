#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

template <typename Key, typename Value> class LRUCache {
  private:
    struct Node {
        Key key;
        Value value;
        Node *prev;
        Node *next;
        Node(const Key &k, const Value &v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };

    Node *head_;
    Node *tail_;
    std::unordered_map<Key, Node *> map_;
    size_t capacity_;

    void detach(Node *n) {
        if (!n) return;
        if (n->prev)
            n->prev->next = n->next;
        else
            head_ = n->next;
        if (n->next)
            n->next->prev = n->prev;
        else
            tail_ = n->prev;
        n->prev = nullptr;
        n->next = nullptr;
    }

    void push_front(Node *n) {
        if (!n) return;
        n->prev = nullptr;
        n->next = head_;
        if (head_) head_->prev = n;
        head_ = n;
        if (!tail_) tail_ = n;
    }

  public:
    explicit LRUCache(size_t capacity) : head_(nullptr), tail_(nullptr), capacity_(capacity) {}

    std::optional<Value> get(const Key &key) {
        auto it = map_.find(key);
        if (it == map_.end()) return std::nullopt;
        Node *n = it->second;
        detach(n);
        push_front(n);
        return n->value;
    }

    void put(const Key &key, const Value &value) {
        if (capacity_ == 0) return;
        auto it = map_.find(key);
        if (it == map_.end()) {
            Node *n = new Node(key, value);
            push_front(n);
            map_[key] = n;
            if (size() > capacity_) {
                Node *victim = tail_;
                map_.erase(victim->key);
                detach(victim);
                delete victim;
            }
        } else {
            Node *n = it->second;
            detach(n);
            push_front(n);
            n->value = value;
        }
    }

    bool contains(const Key &key) const {
        return map_.find(key) != map_.end();
    }

    size_t size() const {
        return map_.size();
    }

    void clear() {
        Node *cur = head_;
        while (cur) {
            Node *nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        map_.clear();
        head_ = nullptr;
        tail_ = nullptr;
    }

    ~LRUCache() {
        clear();
    }

    LRUCache(const LRUCache &) = delete;
    LRUCache &operator=(const LRUCache &) = delete;
};

int main() {
    std::cout << "=== LRUCache Tests ===\n";

    // Test 1: Basic operations
    LRUCache<int, std::string> cache(3);
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    auto val = cache.get(1);
    std::cout << "Get key 1: " << (val ? *val : "not found") << std::endl;
    std::cout << "Contains key 2: " << cache.contains(2) << std::endl;
    std::cout << "Size: " << cache.size() << std::endl;

    // Test 2: LRU eviction
    cache.get(1);         // Make key 1 recent
    cache.put(4, "four"); // Should evict key 2

    std::cout << "After adding key 4:\n";
    std::cout << "  Key 1 exists: " << cache.contains(1) << std::endl;
    std::cout << "  Key 2 exists: " << cache.contains(2) << std::endl;
    std::cout << "  Key 4 exists: " << cache.contains(4) << std::endl;

    // Test 3: Update existing key
    cache.put(1, "ONE");
    val = cache.get(1);
    std::cout << "Updated key 1: " << (val ? *val : "not found") << std::endl;

    // Test 4: Edge case - capacity 1
    LRUCache<int, int> small_cache(1);
    small_cache.put(10, 100);
    small_cache.put(20, 200); // Should evict 10
    std::cout << "Capacity-1 cache contains 10: " << small_cache.contains(10) << std::endl;
    std::cout << "Capacity-1 cache contains 20: " << small_cache.contains(20) << std::endl;

    std::cout << "Tests completed!\n";
    return 0;
}