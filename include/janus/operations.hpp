#pragma once

#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

template<typename K, typename V>
class base_operations {
public:
	base_operations(std::shared_ptr<kv_connection> conn, std::shared_ptr<serializer<K>> key_s,
					std::shared_ptr<serializer<V>> value_s): connection(std::move(conn)),
															key_serializer(std::move(key_s)),
															value_serializer(std::move(value_s)) {
		if (connection == nullptr || key_serializer == nullptr || value_serializer == nullptr) {
			throw std::invalid_argument("connection or key_serializer or value_serializer is null");
		}
	}

	/**
	 * @brief Checks if a key exists (EXISTS key).
	 */
	bool exists(const K &key) {
		auto &serialized_key = this->key_serialize(key);
		return connection->exists(serialized_key);
	}

	/**
	 * @brief Deletes the specified key (DEL key).
	 */
	long long del(const K &key) {
		auto &serialized_key = this->key_serialize(key);
		return connection->del(serialized_key);
	}

	/**
	 * @brief Sets an expiration time on a key (EXPIRE key seconds).
	 */
	bool expire(const K &key, long long seconds) {
		auto &serialized_key = this->key_serialize(key);
		return connection->expire(serialized_key, seconds);
	}

protected:
	std::shared_ptr<kv_connection> connection;
	std::shared_ptr<serializer<K>> key_serializer;
	std::shared_ptr<serializer<V>> value_serializer;

	byte_array key_serialize(const K &key) const {
		return key_serializer->serialize(key);
	}

	K key_deserialize(const byte_array &data) const {
		return key_serializer->deserialize(data);
	}

	byte_array value_serialize(const V &value) const {
		return value_serializer->serialize(value);
	}

	V value_deserialize(const byte_array &data) const {
		return value_serializer->deserialize(data);
	}
};

template<typename K, typename V>
class value_operations {
public:
	virtual ~value_operations() = default;

	virtual void set(const K &key, const V &value) = 0;

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

	virtual long long lpush(const K &key, const std::vector<V> &values) = 0;

	virtual long long rpush(const K &key, const std::vector<V> &values) = 0;

	virtual std::optional<V> lpop(const K &key) = 0;

	virtual std::optional<V> rpop(const K &key) = 0;

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
