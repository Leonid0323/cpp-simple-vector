// вставьте сюда ваш код для класса SimpleVector
// внесите необходимые изменения для поддержки move-семантики

#pragma once

#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"


class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) {
        capacity_ = capacity;
    }
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
    : items_(size), size_(size), capacity_(size) {
        for (auto it = begin(); it != end(); ++it) {
            *it = Type{};
        };
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
    : items_(size), size_(size), capacity_(size){
        //std::fill(begin(), end(), value);
        for (auto it = begin(); it != end(); ++it) {
            *it = value;
        };
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
    : items_(init.size()), size_(init.size()), capacity_(init.size()){
        std::copy(init.begin(), init.end(), begin());
    }
    
    explicit SimpleVector(ReserveProxyObj capacity)
        : items_(capacity.capacity_),
        size_(0),
        capacity_(capacity.capacity_) {
    }
    
    SimpleVector(const SimpleVector& other) {
        SimpleVector tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        this->swap(tmp);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }
    
    SimpleVector(SimpleVector&& other) noexcept
    : items_(std::move(other.items_)), size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)) {
    }
    
    
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_){
            size_ = new_size;
        } else if (new_size <= capacity_){
            for (auto it = begin() + size_; it != begin() + new_size; ++it) {
                *it = Type{};
            }
            size_ = new_size;
        } else{
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            SimpleVector new_vec(new_capacity);
            std::move(begin(), end(), new_vec.begin());
            items_.swap(new_vec.items_);
            size_ = new_size;
            capacity_ = new_capacity;
            
        }
    }
    
    void PushBack(Type&& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = std::move(item);
    }

    void PushBack(const Type& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = item;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_index = pos - cbegin();
        Resize(size_ + 1);
        std::copy_backward(begin() + pos_index, end(), end() + 1);
        items_[pos_index] = value;
        return begin() + pos_index;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_index = pos - cbegin();
        Resize(size_ + 1);
        std::move_backward(begin() + pos_index, end(), end() + 1);
        items_[pos_index] = std::move(value);
        return begin() + pos_index;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_!=0){
            size_--;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t pos_index = pos - cbegin();
        std::move(begin()+pos_index+1, end(), begin()+pos_index);
        size_--;
        return begin()+pos_index;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >=size_){
            throw std::out_of_range("index is out of range SimpleVector");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >=size_){
            throw std::out_of_range("index is out of range SimpleVector");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }
    
    void Reserve(size_t new_capacity){
        if(new_capacity > capacity_){
            SimpleVector new_vec(new_capacity);
            std::move(begin(), end(), new_vec.begin());
            items_.swap(new_vec.items_);
            capacity_ = new_capacity;
        }
    }
    
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + GetSize();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + GetSize();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + GetSize();
    }
private:
    ArrayPtr<Type> items_{};

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()){
        return false;
    }
    if (lhs.begin() == rhs.begin()){
        return true;
    }
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
