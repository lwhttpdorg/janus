#pragma once

#include "kv_connection.hpp"
#include "operations.hpp"
#include "redis_template.hpp"

template<typename K, typename V>
class default_value_operations: public value_operations<K, V> {
public:
	explicit default_value_operations(redis_template<K, V> &ops) : tpl(ops) {
	}

	bool set(const K &key, const V &value) override {
		return tpl.get_connection().set(tpl.serialize_key(key), tpl.serialize_value(value));
	}

	std::optional<V> get(const K &key) override {
		auto val = tpl.get_connection().get(tpl.serialize_key(key));
		if (val) return tpl.deserialize_value(*val);
		return std::nullopt;
	}

	long long incr(const K &key, long long delta) override {
		return tpl.get_connection().incr(tpl.serialize_key(key), delta);
	}

	long long decr(const K &key, long long delta) override {
		return tpl.get_connection().decr(tpl.serialize_key(key), delta);
	}

	long long append(const K &key, const V &value) override {
		return tpl.get_connection().append(tpl.serialize_key(key), tpl.serialize_value(value));
	}

	std::optional<V> get_and_set(const K &key, const V &value) override {
		auto val = tpl.get_connection().getset(tpl.serialize_key(key), tpl.serialize_value(value));
		if (val) return tpl.deserialize_value(*val);
		return std::nullopt;
	}

private:
	redis_template<K, V> &tpl;
};

template<typename K, typename V>
class default_hash_operations: public hash_operations<K, V> {
public:
	explicit default_hash_operations(redis_template<K, V> &ops) : tpl(ops) {
	}

private:
	redis_template<K, V> &tpl;
};

template<typename K, typename V>
class default_list_operations: public list_operations<K, V> {
public:
	explicit default_list_operations(redis_template<K, V> &ops) : tpl(ops) {
	}

private:
	redis_template<K, V> &tpl;
};

template<typename K, typename V>
class valkey_set_operations: public set_operations<K, V> {
public:
	explicit valkey_set_operations(redis_template<K, V> &ops) : tpl(ops) {
	}

private:
	redis_template<K, V> &tpl;
};

template<typename K, typename V>
class default_zset_operations: public zset_operations<K, V> {
public:
	explicit default_zset_operations(redis_template<K, V> &ops) : tpl(ops) {
	}

private:
	redis_template<K, V> &tpl;
};
