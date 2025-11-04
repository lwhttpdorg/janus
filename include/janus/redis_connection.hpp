#pragma once

#include <hiredis/hiredis.h>
#include <memory>
#include <string>

#include "kv_connection.hpp"

class redis_connection: public kv_connection {
public:
	redis_connection(const std::string &host, const unsigned short port) {
		context = redisConnect(host.c_str(), port);
		if (!context || context->err) {
			throw std::runtime_error("Redis connect failed");
		}
	}

	~redis_connection() override {
		redisFree(context);
	}

	bool exists(const std::string &key) override {
		auto r = exec("EXISTS %s", key.c_str());
		return r->type == REDIS_REPLY_INTEGER && r->integer == 1;
	}

	bool expire(const std::string &key, long long seconds) override {
		auto r = exec("EXPIRE %s %lld", key.c_str(), seconds);
		return r->type == REDIS_REPLY_INTEGER && r->integer == 1;
	}

	long long del(const std::string &key) override {
		auto r = exec("DEL %s", key.c_str());
		if (r->type != REDIS_REPLY_INTEGER) {
			throw std::runtime_error("DEL: unexpected reply type");
		}
		return r->integer;
	}

	long long del(const std::vector<std::string> &keys) override {
		if (keys.empty()) return 0;
		std::vector<const char *> argv;
		std::vector<size_t> argvlen;
		argv.push_back("DEL");
		argvlen.push_back(3);
		for (auto &k: keys) {
			argv.push_back(k.c_str());
			argvlen.push_back(k.size());
		}
		auto r = execv(argv, argvlen);
		return (r->type == REDIS_REPLY_INTEGER) ? r->integer : 0;
	}

	/* For String */
	bool set(const std::string &key, const std::string &value) override {
		auto r = exec("SET %s %s", key.c_str(), value.c_str());

		if (r->type == REDIS_REPLY_STATUS) {
			if (std::string(r->str, r->len) == "OK") {
				return true;
			}
		}

#ifdef DEBUG
		std::cerr << "Warning: Redis SET returned non-'OK' status." << std::endl;
#endif
		return false;
	}

	std::optional<std::string> get(const std::string &key) override {
		auto r = exec("GET %s", key.c_str());
		if (r->type == REDIS_REPLY_NIL) return std::nullopt;
		if (r->type == REDIS_REPLY_STRING) return std::string(r->str, r->len);
		return std::nullopt;
	}

	std::optional<std::string> getset(const std::string &key, const std::string &new_value) override {
		auto r = exec("GETSET %s %s", key.c_str(), new_value.c_str());
		if (r->type == REDIS_REPLY_NIL) return std::nullopt;
		if (r->type == REDIS_REPLY_STRING) return std::string(r->str, r->len);
		return std::nullopt;
	}

	long long incr(const std::string &key, long long delta) override {
		auto r = exec("INCRBY %s %lld", key.c_str(), delta);
		if (r->type != REDIS_REPLY_INTEGER) {
			throw std::runtime_error("INCRBY: unexpected reply type");
		}
		return r->integer;
	}

	long long decr(const std::string &key, long long delta) override {
		auto r = exec("DECRBY %s %lld", key.c_str(), delta);
		if (r->type != REDIS_REPLY_INTEGER) {
			throw std::runtime_error("DECRBY: unexpected reply type");
		}
		return r->integer;
	}

	long long append(const std::string &key, const std::string &value) override {
		auto r = exec("APPEND %s %s", key.c_str(), value.c_str());
		if (r->type != REDIS_REPLY_INTEGER) {
			throw std::runtime_error("APPEND: unexpected reply type");
		}
		return r->integer;
	}

	/* For Hash */
	std::optional<std::string> hget(const std::string &key, const std::string &hash_key) override {
		auto r = exec("HGET %s %s", key.c_str(), hash_key.c_str());
		if (r->type == REDIS_REPLY_NIL) {
			return std::nullopt;
		}
		if (r->type == REDIS_REPLY_STRING) {
			return std::string(r->str, r->len);
		}
		throw std::runtime_error("Unexpected reply type in HGET");
	}

	void hget(const std::string &key, std::unordered_map<std::string, std::optional<std::string>> &hash_map) override {
		if (hash_map.empty()) return;

		std::vector<const char *> argv;
		std::vector<size_t> argvlen;

		argv.push_back("HMGET");
		argvlen.push_back(5);
		argv.push_back(key.c_str());
		argvlen.push_back(key.size());

		std::vector<std::string> fields;
		fields.reserve(hash_map.size());
		for (auto &kv: hash_map) {
			argv.push_back(kv.first.c_str());
			argvlen.push_back(kv.first.size());
			fields.push_back(kv.first);
		}

		auto r = execv(argv, argvlen);
		if (r->type != REDIS_REPLY_ARRAY) return;

		for (size_t i = 0; i < r->elements && i < fields.size(); ++i) {
			redisReply *elem = r->element[i];
			if (elem->type == REDIS_REPLY_STRING) {
				hash_map[fields[i]] = std::string(elem->str, elem->len);
			}
			else if (elem->type == REDIS_REPLY_NIL) {
				hash_map[fields[i]] = std::nullopt;
			}
			else {
				throw std::runtime_error("HMGET: unexpected element type");
			}
		}
	}

	bool hset(const std::string &key, const std::string &field, const std::string &value) override {
		auto r = exec("HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
		return r->type == REDIS_REPLY_INTEGER && r->integer >= 0;
	}

	bool hset(const std::string &key, const std::unordered_map<std::string, std::string> &hash_map) override {
		if (hash_map.empty()) return false;

		std::vector<const char *> argv;
		std::vector<size_t> argvlen;

		argv.push_back("HSET");
		argvlen.push_back(4);
		argv.push_back(key.c_str());
		argvlen.push_back(key.size());

		for (auto &kv: hash_map) {
			argv.push_back(kv.first.c_str());
			argvlen.push_back(kv.first.size());
			argv.push_back(kv.second.c_str());
			argvlen.push_back(kv.second.size());
		}

		redisReply *raw = static_cast<redisReply *>(
			redisCommandArgv(context, static_cast<int>(argv.size()), argv.data(), argvlen.data()));
		if (!raw) throw std::runtime_error("HSET command failed");
		std::unique_ptr<redisReply, reply_deleter> r(raw);

		return r->type == REDIS_REPLY_INTEGER && r->integer >= 0;
	}

	std::unordered_map<std::string, std::string> hgetall(const std::string &key) override {
		std::unordered_map<std::string, std::string> result;
		auto r = exec("HGETALL %s", key.c_str());
		if (r->type == REDIS_REPLY_ARRAY) {
			for (size_t i = 0; i + 1 < r->elements; i += 2) {
				result.emplace(r->element[i]->str, r->element[i + 1]->str);
			}
		}
		return result;
	}

	std::vector<std::string> hkeys(const std::string &key) override {
		std::vector<std::string> result;
		auto r = exec("HKEYS %s", key.c_str());
		if (r->type == REDIS_REPLY_ARRAY) {
			for (size_t i = 0; i < r->elements; ++i) {
				result.emplace_back(r->element[i]->str, r->element[i]->len);
			}
		}
		return result;
	}

	std::vector<std::string> hvals(const std::string &key) override {
		std::vector<std::string> result;
		auto r = exec("HVALS %s", key.c_str());
		if (r->type == REDIS_REPLY_ARRAY) {
			for (size_t i = 0; i < r->elements; ++i) {
				result.emplace_back(r->element[i]->str, r->element[i]->len);
			}
		}
		return result;
	}

	long long hdel(const std::string &key, const std::string &hash_key) override {
		auto r = exec("HDEL %s %s", key.c_str(), hash_key.c_str());
		if (r->type != REDIS_REPLY_INTEGER) {
			throw std::runtime_error("HDEL: unexpected reply type");
		}
		return r->integer;
	}

	long long hdel(const std::string &key, const std::vector<std::string> &hash_keys) override {
		if (hash_keys.empty()) return 0;

		std::vector<const char *> argv;
		std::vector<size_t> argvlen;

		argv.push_back("HDEL");
		argvlen.push_back(4);
		argv.push_back(key.c_str());
		argvlen.push_back(key.size());

		for (auto &f: hash_keys) {
			argv.push_back(f.c_str());
			argvlen.push_back(f.size());
		}

		auto r = execv(argv, argvlen);
		if (r->type != REDIS_REPLY_INTEGER) {
			throw std::runtime_error("HDEL: unexpected reply type");
		}
		return r->integer;
	}

protected:
	struct reply_deleter {
		void operator()(redisReply *r) const noexcept {
			if (r) freeReplyObject(r);
		}
	};
	using reply_ptr = std::unique_ptr<redisReply, reply_deleter>;

	reply_ptr exec(const char *fmt, ...) const {
		va_list ap;
		va_start(ap, fmt);
		redisReply *r = static_cast<redisReply *>(redisvCommand(context, fmt, ap));
		va_end(ap);
		if (!r) throw std::runtime_error("Command failed");
		if (r->type == REDIS_REPLY_ERROR) {
			std::string err(r->str, r->len);
			freeReplyObject(r);
			throw std::runtime_error("Redis error: " + err);
		}
		return reply_ptr(r);
	}

	[[nodiscard]] reply_ptr execv(const std::vector<const char *> &argv, const std::vector<size_t> &argvlen) const {
		redisReply *r = static_cast<redisReply *>(redisCommandArgv(
			context, static_cast<int>(argv.size()), const_cast<const char **>(argv.data()), argvlen.data()));
		if (!r) throw std::runtime_error("CommandArgv failed");
		if (r->type == REDIS_REPLY_ERROR) {
			std::string err(r->str, r->len);
			freeReplyObject(r);
			throw std::runtime_error("Redis error: " + err);
		}
		return reply_ptr(r);
	}

private:
	redisContext *context;
};
