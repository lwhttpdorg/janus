#pragma once

#include <memory>
#include <string>

#include "redis_operations.hpp"

class kv_connection;

template<typename T>
class serializer;

template<typename K, typename V>
class redis_operations {
public:
	virtual ~redis_operations() = default;

	virtual bool exists(const K &key) = 0;

	virtual bool expire(const K &key, long long seconds) = 0;

	virtual long long del(const K &key) = 0;

	virtual long long del(const std::vector<K> &keys) = 0;

	virtual value_operations<K, V> &ops_for_value() = 0;

	virtual hash_operations<K, V> &ops_for_hash() = 0;

	virtual list_operations<K, V> &ops_for_list() = 0;

	virtual set_operations<K, V> &ops_for_set() = 0;

	virtual zset_operations<K, V> &ops_for_zset() = 0;
};

template<typename K, typename V>
class redis_template: public redis_operations<K, V> {
public:
	/**
	 * @brief Constructor: Injects all necessary dependencies.
	 * * @param conn Shared pointer to the low-level connection handler.
	 * @param k_serializer Serializer for the key type K.
	 * @param v_serializer Serializer for the value type V.
	 */
	redis_template(const std::shared_ptr<kv_connection>& conn, const std::shared_ptr<serializer<K>> k_serializer,
				   const std::shared_ptr<serializer<V>> v_serializer) :
		connection(conn), key_serializer(k_serializer), value_serializer(v_serializer) {
		if (conn == nullptr || k_serializer == nullptr || v_serializer == nullptr) {
			throw std::invalid_argument("redis_template: connection or serializer is null");
		}

		value_ops = std::make_unique<default_value_operations<K, V>>(*this);
		//hash_ops = std::make_unique<default_hash_operations<K, V>>(*this);
		//list_ops = std::make_unique<default_list_operations<K, V>>(*this);
		//set_ops = std::make_unique<default_set_operations<K, V>>(*this);
		//zset_ops = std::make_unique<default_zset_operations<K, V>>(*this);
	}

	bool exists(const K &key) override {
		auto serialized_key = this->serialize_key(key);
		return connection->exists(serialized_key);
	}

	bool expire(const K &key, long long seconds) override {
		auto serialized_key = this->serialize_key(key);
		return connection->expire(serialized_key, seconds);
	}

	long long del(const K &key) override {
		auto serialized_key = this->serialize_key(key);
		return connection->del(serialized_key);
	}

	long long del(const std::vector<K> &keys) override {
		std::vector<K> s_keys;
		for (const auto &key: keys) {
			s_keys.push_back(this->serialize_key(key));
		}
		return connection->del(s_keys);
	}

	// ==========================================================
	// Implementation of Operation Views (Returning References)
	// ==========================================================

	value_operations<K, V> &ops_for_value() override {
		return *value_ops;
	}

	hash_operations<K, V> &ops_for_hash() override {
		return *hash_ops;
	}

	list_operations<K, V> &ops_for_list() override {
		return *list_ops;
	}

	set_operations<K, V> &ops_for_set() override {
		return *set_ops;
	}

	zset_operations<K, V> &ops_for_zset() override {
		return *zset_ops;
	}

	[[nodiscard]] std::string serialize_key(const K &key) const {
		return key_serializer->serialize(key);
	}

	[[nodiscard]] K deserialize_key(const std::string &data) const {
		return key_serializer->deserialize(data);
	}

	[[nodiscard]] std::string serialize_value(const V &value) const {
		return value_serializer->serialize(value);
	}

	[[nodiscard]] V deserialize_value(const std::string &data) const {
		return value_serializer->deserialize(data);
	}

	kv_connection& get_connection() { return *connection; }

private:
	std::shared_ptr<kv_connection> connection;
	std::shared_ptr<serializer<K>> key_serializer;
	std::shared_ptr<serializer<V>> value_serializer;

	// --- Operation Views Instances (Unique ownership) ---
	std::unique_ptr<value_operations<K, V>> value_ops;
	std::unique_ptr<hash_operations<K, V>> hash_ops;
	std::unique_ptr<list_operations<K, V>> list_ops;
	std::unique_ptr<set_operations<K, V>> set_ops;
	std::unique_ptr<zset_operations<K, V>> zset_ops;
};
