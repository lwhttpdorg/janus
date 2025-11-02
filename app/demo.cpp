#include <iostream>

#include <janus/janus.hpp>
#include <utility>

class user {
public:
	user() = default;
	user(unsigned int id, std::string name, std::string email) :
		id_(id), name_(std::move(name)), email_(std::move(email)) {
	}

private:
	unsigned int id_ = 0;
	std::string name_;
	std::string email_;
	friend std::ostream &operator<<(std::ostream &os, const user &u);
	friend std::istream &operator>>(std::istream &is, user &u);
};

std::ostream &operator<<(std::ostream &os, const user &u) {
	// 写入格式：id,name,email
	os << u.id_ << "," << u.name_ << "," << u.email_;
	return os;
}

std::istream &operator>>(std::istream &is, user &u) {
	std::string temp_id;
	std::string temp_name;
	std::string temp_email;

	if (!std::getline(is, temp_id, ',')) {
		is.setstate(std::ios::failbit);
		return is;
	}
	if (!std::getline(is, temp_name, ',')) {
		is.setstate(std::ios::failbit);
		return is;
	}

	if (!std::getline(is, temp_email)) {
		if (is.fail()) return is;
	}

	try {
		u.id_ = std::stoul(temp_id);
		u.name_ = std::move(temp_name);
		u.email_ = std::move(temp_email);
	}
	catch (const std::exception &) {
		is.setstate(std::ios::failbit);
	}

	return is;
}

int main(int argc, const char **argv) {
	std::cout << "Hello World!" << std::endl;
	user someone{1, "Alex", "alex.sandro@gmail.com"};

	std::shared_ptr<kv_connection> connection = std::make_shared<redis_pp_connection>();

	string_valkey_template my_template = string_valkey_template(connection);

	auto &value_ops = my_template.ops_for_value();
	value_ops.set("aaa", std::to_string(1));
	value_ops.set("bbb", std::to_string(2));
	auto v = value_ops.get("aaa");

	auto &hash_ops = my_template.ops_for_hash();

	return 0;
}
