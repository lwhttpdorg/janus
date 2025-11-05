#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"
#include "janus/janus.hpp"

#define DEFAULT_REDIS_HOST "127.0.0.1"
#define DEFAULT_REDIS_PORT 6379

class set_operations_test: public ::testing::Test {
protected:
	// Type aliases (K and V are both std::string)
	using key_type = std::string;
	using value_type = std::string;

	const key_type test_key_1 = "test_set_a";
	const key_type test_key_2 = "test_set_b";

	// Connection parameters (No trailing underscores)
	std::string redis_host;
	unsigned short redis_port{DEFAULT_REDIS_PORT};

	// Janus components (No trailing underscores)
	std::shared_ptr<kv_connection> conn;
	std::shared_ptr<serializer<key_type>> k_serializer;
	std::shared_ptr<serializer<value_type>> v_serializer;
	std::unique_ptr<redis_template<key_type, value_type>> tpl;

	void SetUp() override {
		// 1. Retrieve connection parameters from environment variables
		if (const char *env_host = std::getenv("TEST_REDIS_HOST")) {
			redis_host = env_host;
		}
		else {
			redis_host = DEFAULT_REDIS_HOST;
			std::cerr << "Warning: TEST_REDIS_HOST not set. Using default: " << redis_host << std::endl;
		}

		if (const char *env_port = std::getenv("TEST_REDIS_PORT")) {
			try {
				int port_int = std::stoi(env_port);
				if (port_int > 0 && port_int < 65536) {
					redis_port = static_cast<unsigned short>(port_int);
				}
				else {
					throw std::runtime_error("Port out of range.");
				}
			}
			catch ([[maybe_unused]] const std::exception &e) {
				redis_port = DEFAULT_REDIS_PORT;
				std::cerr << "Warning: Invalid TEST_REDIS_PORT value. Using default: " << redis_port << std::endl;
			}
		}
		else {
			redis_port = DEFAULT_REDIS_PORT;
			std::cerr << "Warning: TEST_REDIS_PORT not set. Using default: " << redis_port << std::endl;
		}

		// 2. Create underlying connection
		try {
			conn = std::make_shared<redis_connection>(redis_host, redis_port);
		}
		catch (const std::runtime_error &e) {
			// If connection fails, skip all tests in this fixture
			GTEST_SKIP() << "Skipping test: Could not connect to Redis at " << redis_host << ":" << redis_port
						 << ". Error: " << e.what();
		}

		// 3. Create Serializers
		k_serializer = std::make_shared<string_serializer<key_type>>();
		v_serializer = std::make_shared<string_serializer<value_type>>();

		// 4. Construct redis_template
		tpl = std::make_unique<redis_template<key_type, value_type>>(conn, k_serializer, v_serializer);

		// 5. Clean up test key
		clear_test_keys();
	}

	void TearDown() override {
		// 6. Clean up test key
		if (tpl) {
			clear_test_keys();
		}
	}

	// Helper to clean keys
	void clear_test_keys() const {
		tpl->del(test_key_1);
		tpl->del(test_key_2);
	}

	// Helper function to get Set operations interface
	[[nodiscard]] auto &set_ops() const {
		return tpl->ops_for_set();
	}
};

// --- Test Cases ---

TEST_F(set_operations_test, sadd_sismember_scard) {
	std::vector<value_type> members = {"a", "b", "c", "b"};

	// 1. Test SADD (a, b, c)
	long long added_count = set_ops().sadd(test_key_1, members);
	EXPECT_EQ(added_count, 3) << "SADD should only count unique new members.";

	// 2. Test SCARD
	EXPECT_EQ(set_ops().scard(test_key_1), 3);

	// 3. Test SISMEMBER
	EXPECT_TRUE(set_ops().sismember(test_key_1, "a"));
	EXPECT_FALSE(set_ops().sismember(test_key_1, "d"));
}

TEST_F(set_operations_test, srem_and_smembers) {
	// Setup initial data: {1, 2, 3}
	set_ops().sadd(test_key_1, {"1", "2", "3"});

	// 1. Test SREM (remove existing and non-existent: 2, 4)
	std::vector<value_type> to_remove = {"2", "4"};
	long long removed_count = set_ops().srem(test_key_1, to_remove);
	EXPECT_EQ(removed_count, 1) << "SREM should only count actually removed members.";

	// 2. Test SMEMBERS (should contain {1, 3})
	std::vector<value_type> members = set_ops().smembers(test_key_1);

	// Use unordered_set for robust comparison
	std::unordered_set<value_type> member_set(members.begin(), members.end());
	EXPECT_EQ(member_set.size(), 2);
	EXPECT_TRUE(member_set.count("1"));
	EXPECT_TRUE(member_set.count("3"));
	EXPECT_FALSE(member_set.count("2"));
}

TEST_F(set_operations_test, sinter) {
	// Setup Set A: {1, 2, 3}
	set_ops().sadd(test_key_1, {"1", "2", "3"});
	// Setup Set B: {2, 3, 4}
	set_ops().sadd(test_key_2, {"2", "3", "4"});

	// 1. Test SINTER (Expected: {2, 3})
	std::vector<key_type> keys_to_intersect = {test_key_1, test_key_2};
	std::vector<value_type> intersection = set_ops().sinter(keys_to_intersect);

	std::unordered_set<value_type> result_set(intersection.begin(), intersection.end());
	EXPECT_EQ(result_set.size(), 2);
	EXPECT_TRUE(result_set.count("2"));
	EXPECT_TRUE(result_set.count("3"));
	EXPECT_FALSE(result_set.count("1"));
}

TEST_F(set_operations_test, spop) {
	set_ops().sadd(test_key_1, {"x", "y", "z"});

	// 1. Test SPOP (1st pop)
	std::optional<value_type> popped_member = set_ops().spop(test_key_1);
	ASSERT_TRUE(popped_member);
	EXPECT_EQ(set_ops().scard(test_key_1), 2);

	// 2. Pop remaining two
	set_ops().spop(test_key_1);
	set_ops().spop(test_key_1);

	// 3. Test SPOP on empty set
	EXPECT_FALSE(set_ops().spop(test_key_1));
	EXPECT_EQ(set_ops().scard(test_key_1), 0);
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
