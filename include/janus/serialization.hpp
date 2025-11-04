#pragma once

#include <sstream>
#include <string>

template<typename T>
class serializer {
public:
	virtual std::string serialize(const T &t) const = 0;

	virtual T deserialize(const std::string &data) const = 0;

	virtual ~serializer() = default;
};

template<typename T>
class string_serializable {
public:
	static std::string to_string(const T &t) {
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	static T from_string(const std::string &s) {
		std::stringstream ss(s);
		T t;
		ss >> t;
		return t;
	}
};

template<typename T>
class string_serializer: public serializer<T> {
public:
	std::string serialize(const T &obj) const override {
		return string_serializable<T>::to_string(obj);
	}

	T deserialize(const std::string &bytes) const override {
		return string_serializable<T>::from_string(bytes);
	}
};
