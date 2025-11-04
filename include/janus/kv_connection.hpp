#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

class kv_connection {
public:
	virtual ~kv_connection() = default;

	virtual bool exists(const std::string &key) = 0;

	virtual bool expire(const std::string &key, long long seconds) = 0;

	virtual long long del(const std::string &keys) = 0;

	virtual long long del(const std::vector<std::string> &keys) = 0;

	/* For String */
	virtual bool set(const std::string &key, const std::string &value) = 0;

	virtual std::optional<std::string> get(const std::string &key) = 0;

	virtual std::optional<std::string> getset(const std::string &key, const std::string &new_value) = 0;

	virtual long long incr(const std::string &key, long long delta) = 0;

	virtual long long decr(const std::string &key, long long delta) = 0;

	virtual long long append(const std::string &key, const std::string &value) = 0;

	/* For Hash */
	virtual std::optional<std::string> hget(const std::string &key, const std::string &hash_key) = 0;

	virtual void hget(const std::string &key,
					  std::unordered_map<std::string, std::optional<std::string>> &hash_map) = 0;

	virtual bool hset(const std::string &key, const std::string &field, const std::string &value) = 0;

	virtual bool hset(const std::string &key, const std::unordered_map<std::string, std::string> &hash_map) = 0;

	virtual std::unordered_map<std::string, std::string> hgetall(const std::string &key) = 0;

	virtual std::vector<std::string> hkeys(const std::string &key) = 0;

	virtual std::vector<std::string> hvals(const std::string &key) = 0;

	virtual long long hdel(const std::string &key, const std::string &hash_key) = 0;

	virtual long long hdel(const std::string &key, const std::vector<std::string> &hash_keys) = 0;

	/* For list */
	/* For Set */
	/* For ZSet */
};
