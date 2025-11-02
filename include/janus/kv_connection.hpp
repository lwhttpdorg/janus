#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "serialization.hpp"

/**
 * @brief Abstract base class for the low-level key-value store connection.
 * * This interface defines the essential set of binary-level commands
 * that the higher-level type-safe operations classes depend on. It enforces
 * the Dependency Inversion Principle (DIP) by abstracting away the specific
 * client implementation (e.g., redis-plus-plus).
 * * Note: Connection lifecycle, pooling, and explicit disconnect/state checks
 * are delegated to the underlying concrete implementation (e.g., smart pointer
 * management or connection pool).
 */
class kv_connection {
public:
	virtual ~kv_connection() = default;

	virtual bool exists(const byte_array &key) = 0;

	virtual long del(const std::vector<byte_array> &keys) = 0;

	/**
	 * @brief Executes the EXPIRE command.
	 * @param key: The binary key.
	 * @param seconds: The expiration time in seconds.
	 * @return bool: 1 if the timeout was set, 0 otherwise.
	 */
	virtual bool expire(const byte_array &key, long long seconds) = 0;

	/**
	 * @brief Executes the GET key command.
	 * @param key The binary key.
	 * @return std::optional<byte_array> The binary value, or empty optional if the key does not exist (nil).
	 */
	virtual std::optional<byte_array> get(const byte_array &key) = 0;

	/**
	 * @brief Executes the GETSET key value command.
	 * @param key The binary key.
	 * @param new_value The new binary value.
	 * @return std::optional<byte_array> The old binary value, or empty optional if the key did not exist (nil).
	 */
	virtual std::optional<byte_array> getset(const byte_array &key, const byte_array &new_value) = 0;

	virtual byte_array incrby(const byte_array &key, long long delta) = 0;

	virtual std::vector<byte_array> mget(const std::vector<byte_array> &keys) = 0;

	virtual bool hset(const byte_array &key, const std::vector<std::pair<byte_array, byte_array>> &field_values) = 0;

	virtual std::optional<byte_array> hget(const byte_array &key, const byte_array &hash_key) = 0;

	virtual std::unordered_map<byte_array, byte_array> hgetall(const byte_array &key) = 0;

	virtual byte_array hdel(const byte_array &key, const std::vector<byte_array> &hash_keys) = 0;

	virtual byte_array lpush(const byte_array &key, const std::vector<byte_array> &values) = 0;

	virtual byte_array rpop(const byte_array &key) = 0;

	virtual std::vector<byte_array> lrange(const byte_array &key, long long start, long long end) = 0;

	virtual long llen(const byte_array &key) = 0;

	virtual byte_array sadd(const byte_array &key, const std::vector<byte_array> &members) = 0;

	virtual byte_array srem(const byte_array &key, const std::vector<byte_array> &members) = 0;

	virtual byte_array sismember(const byte_array &key, const byte_array &member) = 0;

	virtual std::vector<byte_array> smembers(const byte_array &key) = 0;

	virtual byte_array zadd(const byte_array &key, const std::vector<std::pair<double, byte_array>> &score_members) = 0;

	virtual byte_array zincrby(const byte_array &key, double increment, const byte_array &member) = 0;

	virtual std::vector<std::pair<byte_array, double>> zrange_withscores(const byte_array &key, long long start,
																		 long long end) = 0;

	virtual byte_array zrem(const byte_array &key, const std::vector<byte_array> &members) = 0;

	virtual byte_array command(const std::string &cmd, const std::vector<byte_array> &args) = 0;
};

class valkey_connection: public kv_connection {};

class redis_pp_valkey_connection: public valkey_connection {
	// 使用redis++
};
