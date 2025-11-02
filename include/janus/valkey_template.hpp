#pragma once

#include <memory>
#include <string>

#include "kv_template.hpp"

class kv_connection;

template<typename T>
class serializer;

template<typename K>
class valkey_key_operations;
template<typename K, typename V>
class valkey_value_operations;
template<typename K, typename HK, typename HV>
class valkey_hash_operations;
template<typename K, typename V>
class valkey_list_operations;
template<typename K, typename V>
class valkey_set_operations;
template<typename K, typename V>
class valkey_zset_operations;

/**
 * @brief Concrete implementation of kv_template using the underlying Valkey/Redis client.
 * * This class is the central configuration and dependency injection point. It instantiates
 * and manages the lifecycle of all specific *Operations views.
 * * @tparam K The type of the key.
 * @tparam V The default type of the value.
 * @tparam HK The type of the Hash Key, defaults to K.
 * @tparam HV The type of the Hash Value, defaults to V.
 */
template<typename K, typename V, typename HK = K, typename HV = V>
class valkey_template: public kv_template<K, V, HK, HV> {
private:
	// --- Core Dependencies ---
	std::shared_ptr<kv_connection> connection_;

	// --- Serializer Dependencies ---
	std::shared_ptr<serializer<K>> key_serializer;
	std::shared_ptr<serializer<V>> value_serializer;
	std::shared_ptr<serializer<HK>> hash_key_serializer;
	std::shared_ptr<serializer<HV>> hash_value_serializer;

	// --- Operation Views Instances (Unique ownership) ---
	std::unique_ptr<key_operations<K>> key_ops;
	std::unique_ptr<value_operations<K, V>> value_ops;
	std::unique_ptr<hash_operations<K, HK, HV>> hash_ops;
	std::unique_ptr<list_operations<K, V>> list_ops;
	std::unique_ptr<set_operations<K, V>> set_ops;
	std::unique_ptr<zset_operations<K, V>> zset_ops;

public:
	/**
	 * @brief Constructor: Injects all necessary dependencies.
	 * * @param connection Shared pointer to the low-level connection handler.
	 * @param key_serializer Serializer for the primary key type K.
	 * @param value_serializer Serializer for the primary value type V.
	 * @param hash_key_serializer Serializer for the hash key type HK.
	 * @param hash_value_serializer Serializer for the hash value type HV.
	 */
	valkey_template(std::shared_ptr<kv_connection> connection, std::shared_ptr<serializer<K>> key_serializer,
					std::shared_ptr<serializer<V>> value_serializer,
					std::shared_ptr<serializer<HK>> hash_key_serializer,
					std::shared_ptr<serializer<HV>> hash_value_serializer) :
		connection_(std::move(connection)), key_serializer(std::move(key_serializer)),
		value_serializer(std::move(value_serializer)), hash_key_serializer(std::move(hash_key_serializer)),
		hash_value_serializer(std::move(hash_value_serializer)) {
		// Instantiation of all concrete *Operations views, injecting shared dependencies
		key_ops = std::make_unique<valkey_key_operations<K>>(connection_, key_serializer);

		value_ops = std::make_unique<valkey_value_operations<K, V>>(connection_, key_serializer, value_serializer);

		hash_ops = std::make_unique<valkey_hash_operations<K, HK, HV>>(connection_, key_serializer, hash_key_serializer,
																	   hash_value_serializer);

		list_ops = std::make_unique<valkey_list_operations<K, V>>(connection_, key_serializer, value_serializer);

		set_ops = std::make_unique<valkey_set_operations<K, V>>(connection_, key_serializer, value_serializer);

		zset_ops = std::make_unique<valkey_zset_operations<K, V>>(connection_, key_serializer, value_serializer);
	}

	// ==========================================================
	// Implementation of Operation Views (Returning References)
	// ==========================================================

	/**
	 * @copydoc kv_template::ops_for_key()
	 */
	key_operations<K> &ops_for_key() override {
		return *key_ops;
	}

	/**
	 * @copydoc kv_template::ops_for_value()
	 */
	value_operations<K, V> &ops_for_value() override {
		return *value_ops;
	}

	/**
	 * @copydoc kv_template::ops_for_hash()
	 */
	hash_operations<K, HK, HV> &ops_for_hash() override {
		return *hash_ops;
	}

	/**
	 * @copydoc kv_template::ops_for_list()
	 */
	list_operations<K, V> &ops_for_list() override {
		return *list_ops;
	}

	/**
	 * @copydoc kv_template::ops_for_set()
	 */
	set_operations<K, V> &ops_for_set() override {
		return *set_ops;
	}

	/**
	 * @copydoc kv_template::ops_for_zset()
	 */
	zset_operations<K, V> &ops_for_zset() override {
		return *zset_ops;
	}
};

/**
 * @brief Concrete implementation for string key/value pairs using the Valkey/Redis client.
 * Inherits from valkey_template<std::string, std::string>.
 * This class injects the necessary string_serializer_adapter instances.
 */
class string_valkey_template: public valkey_template<std::string, std::string> {
public:
	/**
	 * @brief Constructor: Initializes the template with a low-level connection,
	 * using a single shared instance of string_serializer_adapter for all string types.
	 * @param connection Shared pointer to the low-level connection handler.
	 */
	explicit string_valkey_template(std::shared_ptr<kv_connection> connection) :
		string_valkey_template(std::move(connection), std::make_shared<string_serializer_adapter<std::string>>()) {
	}

private:
	string_valkey_template(std::shared_ptr<kv_connection> connection,
						   const std::shared_ptr<string_serializer_adapter<std::string>> &shared_adapter) :
		valkey_template(std::move(connection), shared_adapter, shared_adapter, shared_adapter, shared_adapter) {
	}
};
