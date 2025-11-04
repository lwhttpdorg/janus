#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

template<typename K, typename V>
class value_operations {
public:
	virtual ~value_operations() = default;

	virtual bool set(const K &key, const V &value) = 0;

	virtual std::optional<V> get(const K &key) = 0;

	virtual long long incr(const K &key, long long delta) = 0;

	virtual long long decr(const K &key, long long delta) = 0;

	virtual long long append(const K &key, const V &value) = 0;

	virtual std::optional<V> get_and_set(const K &key, const V &value) = 0;
};

template<typename K, typename V>
class hash_operations {
public:
	virtual ~hash_operations() = default;

	virtual void put(const K &key, const K &hash_key, const V &value) = 0;

	virtual void put_all(const K &key, const std::unordered_map<K, V> &entries) = 0;

	virtual std::optional<V> get(const K &key, const K &hash_key) = 0;

	virtual std::unordered_map<K, V> entries(const K &key) = 0;

	virtual std::vector<K> keys(const K &key) = 0;

	virtual std::vector<V> values(const K &key) = 0;

	virtual long long delete_field(const K &key, const std::vector<K> &hash_keys) = 0;
};

template<typename K, typename V>
class list_operations {
public:
	virtual ~list_operations() = default;
};

template<typename K, typename V>
class set_operations {
public:
	virtual ~set_operations() = default;
};

template<typename K, typename V>
class zset_operations {
public:
	virtual ~zset_operations() = default;
};
