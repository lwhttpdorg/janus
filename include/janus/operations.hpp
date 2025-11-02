#pragma once

#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename K>
class key_operations {
public:
	virtual ~key_operations() = default;

	/**
	 * @brief Checks if a key exists (EXISTS key).
	 */
	virtual bool exists(const K &key) = 0;

	/**
	 * @brief Deletes the specified key (DEL key).
	 */
	virtual long long delete_key(const K &key) = 0;

	/**
	 * @brief Sets an expiration time on a key (EXPIRE key seconds).
	 */
	virtual bool expire(const K &key, long long seconds) = 0;
};

template<typename K, typename V>
class value_operations {
public:
	virtual ~value_operations() = default;
	virtual void set(const K &key, const V &value) = 0;
	virtual std::optional<V> get(const K &key) = 0;
	virtual void set_expire(const K &key, const V &value, long long seconds) = 0;
	virtual long long increment(const K &key, long long delta) = 0;
	virtual std::unordered_map<K, V> multi_get(const std::vector<K> &keys) = 0;
	virtual std::optional<V> get_and_set(const K &key, const V &value) = 0;
};

template<typename K, typename HK, typename HV>
class hash_operations {
public:
	virtual ~hash_operations() = default;
	virtual void put(const K &key, const HK &hash_key, const HV &value) = 0;
	virtual std::optional<HV> get(const K &key, const HK &hash_key) = 0;
	virtual void put_all(const K &key, const std::unordered_map<HK, HV> &entries) = 0;
	virtual std::unordered_map<HK, HV> entries(const K &key) = 0;
	virtual std::unordered_set<HK> keys(const K &key) = 0;
	virtual std::vector<HV> values(const K &key) = 0;
	virtual long long delete_field(const K &key, const std::vector<HK> &hash_keys) = 0;
};

template<typename K, typename V>
class list_operations {
public:
	virtual ~list_operations() = default;
	virtual long long left_push(const K &key, const V &value) = 0;
	virtual std::optional<V> right_pop(const K &key) = 0;
	virtual std::vector<V> range(const K &key, long long start, long long end) = 0;
	virtual long long size(const K &key) = 0;
	virtual std::optional<V> index(const K &key, long long index) = 0;
	virtual long long remove(const K &key, long long count, const V &value) = 0;
};

template<typename K, typename V>
class set_operations {
public:
	virtual ~set_operations() = default;
	virtual long long add(const K &key, const std::vector<V> &values) = 0;
	virtual bool is_member(const K &key, const V &member) = 0;
	virtual std::unordered_set<V> members(const K &key) = 0;
	virtual long long remove(const K &key, const std::vector<V> &members) = 0;
	// 求交集
	virtual std::unordered_set<V> intersect(const K &key, const K &other_key) = 0;
	// 求并集
	virtual std::unordered_set<V> set_union(const K &key, const K &other_key) = 0;
};

template<typename K, typename V>
class zset_operations {
public:
	virtual ~zset_operations() = default;
	virtual bool add(const K &key, const V &member, double score) = 0;
	virtual double increment_score(const K &key, const V &member, double delta) = 0;
	virtual std::set<V> range(const K &key, long long start, long long end) = 0;
	virtual std::set<V> range_by_score(const K &key, double min, double max) = 0;
	virtual long long remove(const K &key, const std::vector<V> &members) = 0;
	virtual std::optional<long long> rank(const K &key, const V &member) = 0;
	virtual std::optional<long long> reverse_rank(const K &key, const V &member) = 0;
};
