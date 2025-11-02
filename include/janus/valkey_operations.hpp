#pragma once

#include <memory>
#include <utility>

#include "kv_connection.hpp"
#include "operations.hpp"

template<typename K, typename V>
class valkey_value_operations: public value_operations<K, V> {
public:
	valkey_value_operations(std::shared_ptr<kv_connection> connection, std::shared_ptr<serializer<K>> key_serializer,
							std::shared_ptr<serializer<V>> value_serializer) :
		connection_(std::move(connection)), key_serializer_(std::move(key_serializer)),
		value_serializer_(std::move(value_serializer)) {
	}

	void set(const K &key, const V &value) override {
		auto &serialized_key = key_serializer_->serialize(key);
		auto &serialized_value = value_serializer_->serialize(value);
		connection_->set(serialized_key, serialized_value);
	}

	std::optional<V> get(const K &key) override {
		auto &serialized_key = key_serializer_->serialize(key);
		return connection_->get(serialized_key);
	}

	void set_expire(const K &key, const V &value, long long seconds) override {
	}

	long long increment(const K &key, long long delta) override {
	}

	std::unordered_map<K, V> multi_get(const std::vector<K> &keys) override {
	}

	std::optional<V> get_and_set(const K &key, const V &value) override {
	}

protected:
	std::shared_ptr<kv_connection> connection_;
	std::shared_ptr<serializer<K>> key_serializer_;
	std::shared_ptr<serializer<V>> value_serializer_;
};

template<typename K, typename HK, typename HV>
class valkey_hash_operations: public hash_operations<K, HK, HV> {
public:
};

template<typename K, typename V>
class valkey_list_operations: public list_operations<K, V> {
public:
};

template<typename K, typename V>
class valkey_set_operations: public set_operations<K, V> {
public:
};

template<typename K, typename V>
class valkey_zset_operations: public zset_operations<K, V> {
public:
};
