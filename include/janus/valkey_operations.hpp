#pragma once

#include <memory>

#include "kv_connection.hpp"
#include "operations.hpp"

template<typename K, typename V>
class valkey_value_operations : public base_operations<K, V>, public value_operations<K, V> {
public:
	valkey_value_operations(std::shared_ptr<kv_connection> conn, std::shared_ptr<serializer<K>> key_s,
							std::shared_ptr<serializer<V>> value_s) : base_operations<K, V>(conn, key_s, value_s) {
	}

	void set(const K &key, const V &value) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_value = this->value_serialize(value);
		this->connection->set(serialized_key, serialized_value);
	}

	std::optional<V> get(const K &key) override {
		auto &serialized_key = this->key_serialize(key);
		if (auto val = this->connection->get(serialized_key); val.has_value()) {
			auto &serialized_value = val.value();
			return this->value_deserialize(serialized_value);
		}
		return std::nullopt;
	}

	long long incr(const K &key, long long delta) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_value = this->value_serialize(delta);
		return this->connection->incr(serialized_key, serialized_value);
	}

	long long decr(const K &key, long long delta) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_value = this->value_serialize(delta);
		return this->connection->decr(serialized_key, serialized_value);
	}

	long long append(const K &key, const V &value) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_value = this->value_serialize(value);
		return this->connection->append(serialized_key, serialized_value);
	}

	std::optional<V> get_and_set(const K &key, const V &value) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_value = this->value_serialize(value);
		return this->connection->get_and_set(serialized_key, serialized_value);
	}
};

template<typename K, typename V>
class valkey_hash_operations : public base_operations<K, V>, public hash_operations<K, V> {
public:
	valkey_hash_operations(std::shared_ptr<kv_connection> conn, std::shared_ptr<serializer<K>> key_s,
							std::shared_ptr<serializer<V>> value_s) : base_operations<K, V>(conn, key_s, value_s) {
	}

	void put(const K &key, const K &hash_key, const V &value) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_hkey = this->key_serialize(hash_key);
		auto &serialized_value = this->value_serialize(value);
		this->connection->put(serialized_key, serialized_hkey, serialized_value);
	}

	void put_all(const K &key, const std::unordered_map<K, V> &entries) override {
		auto &serialized_key = this->key_serialize(key);
		// 序列化hash_key和hash_value并put_all
	}

	std::optional<V> get(const K &key, const K &hash_key) override {
		auto &serialized_key = this->key_serialize(key);
		auto &serialized_hkey = this->key_serialize(hash_key);
		auto val = this->connection->get(serialized_key, serialized_hkey);
		if (val.has_value()) {
			auto &serialized_value = val.value();
			return this->value_deserialize(serialized_value);
		}
		return std::nullopt;
	}

	std::unordered_map<K, V> entries(const K &key) override {
		auto &serialized_key = this->key_serialize(key);
		auto val = this->connection->hgetall(serialized_key);
		//TODO
	}

	std::vector<K> keys(const K &key) override {
	}

	std::vector<V> values(const K &key) override {
	}

	long long delete_field(const K &key, const std::vector<K> &hash_keys) override {
	}
};

template<typename K, typename V>
class valkey_list_operations : public base_operations<K, V>, public list_operations<K, V> {
public:
	valkey_list_operations(std::shared_ptr<kv_connection> conn, std::shared_ptr<serializer<K>> key_s,
							std::shared_ptr<serializer<V>> value_s) : base_operations<K, V>(conn, key_s, value_s) {
	}

	long long lpush(const K &key, const std::vector<V> &values) override {
	}

	long long rpush(const K &key, const std::vector<V> &values) override {
	}

	std::optional<V> lpop(const K &key) override {
	}

	std::optional<V> rpop(const K &key) override {
	}

	std::vector<V> range(const K &key, long long start, long long end) override {
	}

	long long size(const K &key) override {
	}

	std::optional<V> index(const K &key, long long index) override {
	}

	long long remove(const K &key, long long count, const V &value) override {
	}
};

template<typename K, typename V>
class valkey_set_operations : public base_operations<K, V>, public set_operations<K, V> {
public:
	valkey_set_operations(std::shared_ptr<kv_connection> conn, std::shared_ptr<serializer<K>> key_s,
						std::shared_ptr<serializer<V>> value_s) : base_operations<K, V>(conn, key_s, value_s) {
	}

	long long add(const K &key, const std::vector<V> &values) override {
	}

	bool is_member(const K &key, const V &member) override {
	}

	std::unordered_set<V> members(const K &key) override {
	}

	long long remove(const K &key, const std::vector<V> &members) override {
	}

	// 求交集
	std::unordered_set<V> intersect(const K &key, const K &other_key) override {
	}

	// 求并集
	std::unordered_set<V> set_union(const K &key, const K &other_key) override {
	}
};

template<typename K, typename V>
class valkey_zset_operations : public base_operations<K, V>, public zset_operations<K, V> {
public:
	valkey_zset_operations(std::shared_ptr<kv_connection> conn, std::shared_ptr<serializer<K>> key_s,
							std::shared_ptr<serializer<V>> value_s) : base_operations<K, V>(conn, key_s, value_s) {
	}

	bool add(const K &key, const V &member, double score) override {
	}

	double increment_score(const K &key, const V &member, double delta) override {
	}

	std::set<V> range(const K &key, long long start, long long end) override {
	}

	std::set<V> range_by_score(const K &key, double min, double max) override {
	}

	long long remove(const K &key, const std::vector<V> &members) override {
	}

	std::optional<long long> rank(const K &key, const V &member) override {
	}

	std::optional<long long> reverse_rank(const K &key, const V &member) override {
	}
};
