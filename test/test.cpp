#include <iostream>

#include "janus/janus.hpp"

int main(int argc, const char **argv) {
	// 1. 创建底层连接
	std::shared_ptr<kv_connection<std::string, std::string>> conn =
		std::make_shared<redis_connection<>>("172.16.0.2", 6379);

	// 2. 创建序列化器
	std::shared_ptr<serializer<std::string>> k_serializer = std::make_shared<string_serializer<std::string>>();
	std::shared_ptr<serializer<unsigned int>> v_serializer = std::make_shared<string_serializer<unsigned int>>();

	// 3. 构造 redis_template
	redis_template<std::string, unsigned int> tpl(conn, k_serializer, v_serializer);

	// 4. 使用 ops_for_value() 操作字符串类型
	auto &value_ops = tpl.ops_for_value();

	value_ops.set("counter", 42);

	auto val = value_ops.get("counter");
	if (val) {
		std::cout << "counter = " << *val << "\n";
	}

	long long new_val = value_ops.incr("counter", 5);
	std::cout << "counter after incr = " << new_val << "\n";

	// 5. 使用通用 key 操作
	if (tpl.exists("counter")) {
		std::cout << "counter exists\n";
	}

	tpl.expire("counter", 60); // 设置过期时间 60 秒

	return 0;
}
