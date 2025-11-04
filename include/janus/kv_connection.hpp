#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

template<typename K, typename V>
class kv_connection {
public:
	virtual ~kv_connection() = default;

	virtual bool exists(const K &key) = 0;

	virtual bool expire(const K &key, long long seconds) = 0;

	virtual long long del(const std::vector<K> &keys) = 0;

	/* For String */
	virtual bool set(const std::string &key, const std::string &value) = 0;

	virtual std::optional<V> get(const K &key) = 0;

	virtual std::optional<V> getset(const K &key, const V &new_value) = 0;

	virtual V incr(const K &key, long long delta) = 0;

	virtual V decr(const K &key, long long delta) = 0;

	virtual long long append(const K &key, const V &value) = 0;

	/* For Hash */
	virtual std::optional<V> hget(const K &key, const K &hash_key) = 0;

	virtual void hget(const std::string &key,
					  std::unordered_map<std::string, std::optional<std::string>> &hash_map) = 0;

	virtual bool hset(const K &key, const K &field, const V &value) = 0;

	virtual bool hset(const K &key, const std::unordered_map<K, V> &hash_map) = 0;

	virtual std::unordered_map<K, V> hgetall(const K &key) = 0;

	virtual std::vector<K> hkeys(const K &key) = 0;

	virtual std::vector<V> hvals(const K &key) = 0;

	virtual long long hdel(const K &key, const K &hash_key) = 0;

	virtual long long hdel(const K &key, const std::vector<K> &hash_keys) = 0;

	/* For list */
	/* For Set */
	/* For ZSet */
};
