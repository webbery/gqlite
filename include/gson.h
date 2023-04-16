#pragma once
#include "Type/GQLType.h"

namespace gql {
	class gson {
	public:
		enum class value_t : std::uint8_t {
			null,
			object,
			array,
			boolean,
			number_integer,
			number_unsigned,
			number_float,
			binary,
			discarded
		};

		gson() {}

		bool empty() {
			if (_valueType != value_t::null) {

			}
			return true;
		}

		std::string dump() {}

		value_t operator()() {
			return _valueType;
		}

	private:
		value_t _valueType{ value_t::null };
		using objects_t = std::multimap<std::string, attribute_t>;
		Variant<objects_t, bool, gql::vector_uint8> _data;
	};
}