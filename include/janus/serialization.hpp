#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility> // For std::move
#include <vector>

/**
 * @brief A container wrapping std::vector<std::byte> for representing binary data buffers.
 * This class provides convenient methods and iterators for raw byte access.
 */
class byte_array {
private:
	std::vector<std::byte> buffer;

public:
	byte_array() = default;

	/**
	 * @brief Constructs a byte_array with a specified initial size.
	 * @param initial_size The size of the buffer in bytes.
	 */
	explicit byte_array(std::size_t initial_size) : buffer(initial_size) {
	}

	/**
	 * @brief Constructs a byte_array by copying data from a raw byte pointer.
	 * @param data_ptr Pointer to the start of the data.
	 * @param length The number of bytes to copy.
	 */
	byte_array(const std::byte *data_ptr, std::size_t length) : buffer(data_ptr, data_ptr + length) {
	}

	/**
	 * @brief Constructs a byte_array by moving an existing vector<std::byte>.
	 * @param input_buffer The vector<std::byte> to move from.
	 */
	explicit byte_array(std::vector<std::byte> &&input_buffer) : buffer(std::move(input_buffer)) {
	}

	/**
	 * @brief Constructs a byte_array by copying an existing vector<std::byte>.
	 * @param input_buffer The vector<std::byte> to copy from.
	 */
	explicit byte_array(const std::vector<std::byte> &input_buffer) : buffer(input_buffer) {
	}

	/**
	 * @brief Returns a non-const pointer to the underlying byte data.
	 */
	std::byte *data() {
		return buffer.data();
	}

	/**
	 * @brief Returns a const pointer to the underlying byte data.
	 */
	[[nodiscard]] const std::byte *data() const {
		return buffer.data();
	}

	/**
	 * @brief Returns the total number of bytes in the array.
	 */
	[[nodiscard]] std::size_t length() const {
		return buffer.size();
	}

	/**
	 * @brief Checks if the array is empty.
	 */
	[[nodiscard]] bool is_empty() const {
		return buffer.empty();
	}

	/**
	 * @brief Clears the contents of the byte array.
	 */
	void clear() {
		buffer.clear();
	}

	// Access operators
	std::byte &operator[](std::size_t index) {
		return buffer.at(index);
	}

	const std::byte &operator[](std::size_t index) const {
		return buffer.at(index);
	}

	// Iterator definitions
	using iterator = std::vector<std::byte>::iterator;
	using const_iterator = std::vector<std::byte>::const_iterator;

	iterator begin() {
		return buffer.begin();
	}

	iterator end() {
		return buffer.end();
	}

	[[nodiscard]] const_iterator begin() const {
		return buffer.cbegin();
	}

	[[nodiscard]] const_iterator end() const {
		return buffer.cend();
	}

	[[nodiscard]] const_iterator cbegin() const {
		return buffer.cbegin();
	}

	[[nodiscard]] const_iterator cend() const {
		return buffer.cend();
	}
};

/**
 * @brief Abstract interface for binary serialization (T <-> byte_array).
 * @tparam T The business type to be serialized.
 */
template<typename T>
class serializer {
public:
	/**
	 * @brief Serializes an object into a byte_array.
	 * @param t The object to serialize.
	 * @return The binary data buffer.
	 */
	virtual byte_array serialize(const T &t) const = 0;

	/**
	 * @brief Deserializes a byte_array back into an object.
	 * @param data The binary data buffer.
	 * @return The deserialized object.
	 */
	virtual T deserialize(const byte_array &data) const = 0;
	virtual ~serializer() = default;
};

// --- Stream Operator Based Text Serialization ---

namespace janus {

	/**
	 * @brief Public facing string serializer, using standard C++ stream operators (<< and >>)
	 * which users must overload for their custom types T.
	 * * NOTE: For standard types (int, string, etc.), standard library overloads are automatically used.
	 * @tparam T The type to be serialized.
	 */
	template<typename T>
	class string_serializer final {
	public:
		/**
		 * @brief Serializes T into a std::string representation by using operator<< on a stringstream.
		 * @param obj The object to serialize.
		 * @return The text representation.
		 */
		static std::string serialize(const T &obj) {
			std::stringstream ss;
			// Calls the non-member operator<<(std::ostream&, const T&).
			ss << obj;

			if (ss.fail() || ss.bad()) {
				throw std::runtime_error("Stream serialization failed for object.");
			}

			return ss.str();
		}

		/**
		 * @brief Deserializes a std::string back into T by using operator>> on a stringstream.
		 * @param str The string representation.
		 * @return The deserialized object.
		 */
		static T deserialize(const std::string &str) {
			std::stringstream ss(str);
			T obj{}; // Requires T to be Default Constructible

			// Calls the non-member operator>>(std::istream&, T&).
			ss >> obj;

			// Check for common stream failure states
			if (ss.fail() && !ss.eof()) {
				// Failed, but didn't reach end-of-file (e.g., bad format)
				throw std::runtime_error("Stream deserialization failed due to format error.");
			}
			if (ss.bad()) {
				// Bad state (e.g., I/O error)
				throw std::runtime_error("Stream deserialization failed due to I/O error.");
			}

			return obj;
		}
	};
} // namespace janus

// --- Adapter Pattern (string_serializer -> binary serializer) ---

/**
 * @brief Adapter class to convert janus::string_serializer<T> (T <-> std::string)
 * to the binary serializer<T> (T <-> byte_array) interface.
 * @tparam T The business type being serialized.
 */
template<typename T>
class string_serializer_adapter final: public serializer<T> {
public:
	/**
	 * @brief Serializes T into byte_array via string conversion.
	 * @param obj The object to serialize.
	 * @return The binary data buffer.
	 */
	byte_array serialize(const T &obj) const override {
		std::string text = janus::string_serializer<T>::serialize(obj);

		const std::byte *byte_ptr = reinterpret_cast<const std::byte *>(text.data());
		return {byte_ptr, text.size()};
	}

	/**
	 * @brief Deserializes byte_array into T via string conversion.
	 * @param bytes The binary data buffer.
	 * @return The deserialized object.
	 */
	T deserialize(const byte_array &bytes) const override {
		const char *char_ptr = reinterpret_cast<const char *>(bytes.data());
		std::string text(char_ptr, bytes.length());
		return janus::string_serializer<T>::deserialize(text);
	}
};
