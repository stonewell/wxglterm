#pragma once

#include <memory>
#include <stack>
#include <stdexcept>
#include <functional>
#include <mutex>

template <class T, class D = std::default_delete<T>>
class SmartObjectPool
{
private:
    using pointer_type = SmartObjectPool<T, D>*;
    using weak_pool_ptr_type = std::weak_ptr<pointer_type>;

    struct ReturnToPool_Deleter {
    public:
        explicit ReturnToPool_Deleter(weak_pool_ptr_type pool)
            : pool_(pool) {}

        void operator()(T* ptr) {
            if (auto pool_ptr = pool_.lock())
                (*pool_ptr.get())->add(ptr);
            else
                D{}(ptr);
        }
    private:
        weak_pool_ptr_type pool_;
    };

    static T * null_item() {
        return nullptr;
    }
public:
    using item_ptr_type = std::unique_ptr<T, ReturnToPool_Deleter>;
    using item_create_type = std::function<T *()>;

    SmartObjectPool(item_create_type func = null_item)
        : this_ptr_{new pointer_type(this)}
        , pool_ {}
        , item_create_func_ {func} {
        }

    virtual ~SmartObjectPool(){}

    void add(T * ptr) {
        std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
        pool_.push(std::unique_ptr<T, D>{ptr});
    }

    item_ptr_type acquire(bool create_new = true) {
        std::lock_guard<std::recursive_mutex> guard(m_UpdateLock);
        bool empty = pool_.empty();

        if (empty && !create_new)
            throw std::out_of_range("Cannot acquire object from an empty pool.");

        if (empty && create_new) {
            T * t = item_create_func_();

            if (!t) {
                throw std::out_of_range("Cannot create new item.");
            }

            add(t);
        }

        item_ptr_type tmp {pool_.top().release(),
                    ReturnToPool_Deleter{weak_pool_ptr_type{this_ptr_}} };
        pool_.pop();
        return tmp;
    }

    bool empty() const {
        return pool_.empty();
    }

    size_t size() const {
        return pool_.size();
    }

private:
    std::shared_ptr<pointer_type > this_ptr_;
    std::stack<std::unique_ptr<T, D>> pool_;
    item_create_type item_create_func_;
    std::recursive_mutex m_UpdateLock;
};
