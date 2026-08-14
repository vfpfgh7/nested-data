#pragma once
#include <deque>
#include <map>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>
#include "nested_data.hpp"

namespace nested_data::json
{
	class JSONError : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};
	class JSONValue : public NestedValue
	{
	public:
		JSONValue(std::nullptr_t _value)
			: _value(_value)
		{
		}

		JSONValue(bool _value)
			: _value(_value)
		{
		}

		JSONValue(int _value)
			: _value(_value)
		{
		}

		JSONValue(double _value)
			: _value(_value)
		{
		}

		JSONValue(std::string _value)
			: _value(std::move(_value))
		{
		}

		void set(std::nullptr_t _value) {
			this->_value = _value;
		}
		void set(std::string _value) override{
			this->_value = _value;
		}
		void set(bool _value) override{
			this->_value = _value;
		}
		void set(int _value) override{
			this->_value = _value;
		}
		void set(double _value) override{
			this->_value = _value;
		}
		void set(const char* _value) override
		{
			this->_value = std::string(_value);
		}

		JSONValue& operator=(std::nullptr_t _value)
		{
			this->_value = _value;
			return *this;
		}

		JSONValue& operator=(const std::string& _value)
		{
			this->_value = _value;
			return *this;
		}

		JSONValue& operator=(const char* _value)
		{
			this->_value = std::string(_value);
			return *this;
		}

		JSONValue& operator=(bool _value)
		{
			this->_value = _value;
			return *this;
		}

		JSONValue& operator=(int _value)
		{
			this->_value = _value;
			return *this;
		}

		JSONValue& operator=(double _value)
		{
			this->_value = _value;
			return *this;
		}

		const Value get() const override
		{
			return _value;
		}

		Value get() override
		{
			return _value;
		}

	private:
		Value _value;
	};
	class JSON : public NestedData
	{
	public:
		class Reference;
		JSON() = default;
		~JSON() override = default;

		JSON(const JSON&) = delete;
		JSON& operator=(const JSON&) = delete;

		JSON(JSON&&) noexcept = default;
		JSON& operator=(JSON&&) noexcept = default;

		NestedData* first() override;
		NestedData* current() override;
		NestedData* next() override;
		NestedData* last() override;
		NestedData* previous() override;

		std::deque<NestedData*> children() override;
		std::deque<std::string> children_names();

		Reference operator[](const std::string&);
		const NestedValue& operator[](
			const std::string& _name) const;

		JSON& prepend(std::pair<std::string, NestedValue*>);
		JSON& prepend(std::string, NestedValue*);

		JSON& append(std::pair<std::string, NestedValue*>);
		JSON& append(std::string, NestedValue*);
		JSON& append(
			std::string _name,
			JSON* _child)
		{
			if (_child == nullptr)
				throw JSONError("Cannot append null JSON child");

			if (last_sibling)
			{
				_child->prev_sibling = last_sibling;
				last_sibling->next_sibling = _child;
			}
			else
			{
				first_sibling = _child;
			}

			last_sibling = _child;

			_children[std::move(_name)] =
				std::unique_ptr<JSON>(_child);

			return *this;
		}

		JSON& insert(std::pair<std::string, NestedValue*>, size_t);
		JSON& insert(std::string, NestedValue*, size_t);

		JSON& insert_after(std::pair<std::string, NestedValue*>, std::string);
		JSON& insert_after(std::string, NestedValue*, std::string);

		JSON& insert_before(std::pair<std::string, NestedValue*>, std::string);
		JSON& insert_before(std::string, NestedValue*, std::string);

		JSON& child(const std::string& _name)
		{
			auto _child = _children.find(_name);

			if (_child == _children.end())
				throw JSONError("JSON child not found");

			return *_child->second;
		}

		void traverse(
			const std::function<void(
				NestedData&,
				const std::function<void(NestedData&)>&
				)>& _strategy,
			const std::function<void(NestedData&)>& _action) override
		{
			_strategy(*this, _action);
		}
		NestedValue* value() override
		{
			return _value;
		}

		const NestedValue* value() const override
		{
			return _value;
		}
		class Reference
		{
		public:
			Reference(JSON& _json, std::string _name)
				: _json(_json),
				_name(std::move(_name))
			{
			}

			Reference& operator=(std::nullptr_t _value)
			{
				value().set(_value);
				return *this;
			}

			Reference& operator=(bool _value)
			{
				value().set(_value);
				return *this;
			}

			Reference& operator=(int _value)
			{
				value().set(_value);
				return *this;
			}

			Reference& operator=(double _value)
			{
				value().set(_value);
				return *this;
			}

			Reference& operator=(std::string _value)
			{
				value().set(std::move(_value));
				return *this;
			}

			Reference& operator=(const char* _value)
			{
				value().set(std::string(_value));
				return *this;
			}

			operator NestedValue& ()
			{
				return value();
			}

			friend std::istream& operator>>(
				std::istream& _input,
				Reference _reference)
			{
				_reference.value();

				auto& _value = _reference.value();

				std::string _input_value;
				_input >> _input_value;

				if (_input_value == "null")
					_value.set(nullptr);
				else if (_input_value == "true")
					_value.set(true);
				else if (_input_value == "false")
					_value.set(false);
				else
				{
					try
					{
						size_t _position{};
						int _integer = std::stoi(
							_input_value,
							&_position
						);

						if (_position == _input_value.size())
						{
							_value.set(_integer);
							return _input;
						}
					}
					catch (...)
					{
					}

					try
					{
						size_t _position{};
						double _double = std::stod(
							_input_value,
							&_position
						);

						if (_position == _input_value.size())
						{
							_value.set(_double);
							return _input;
						}
					}
					catch (...)
					{
					}

					_value.set(std::move(_input_value));
				}

				return _input;
			}
			friend std::ostream& operator<<(
				std::ostream& _output,
				const Reference& _reference)
			{
				_output << _reference.value();
				return _output;
			}
		private:
			NestedValue& value()
			{
				auto _it = _json._children.find(_name);

				if (_it == _json._children.end())
				{
					_json.append(
						_name,
						new JSONValue(nullptr)
					);

					_it = _json._children.find(_name);
				}

				if (!_it->second->_value)
					_it->second->_value =
					new JSONValue(nullptr);

				return *_it->second->_value;
			}
			const NestedValue& value() const
			{
				auto _it = _json._children.find(_name);

				if (_it == _json._children.end())
				{
					_json.append(
						_name,
						new JSONValue(nullptr)
					);

					_it = _json._children.find(_name);
				}

				if (!_it->second->_value)
					_it->second->_value =
					new JSONValue(nullptr);

				return *_it->second->_value;
			}

			JSON& _json;
			std::string _name;
		};
	private:
		std::map<std::string, std::unique_ptr<JSON>> _children{};
		NestedValue* _value{};
		JSON* next_sibling{};
		JSON* prev_sibling{};
		JSON* first_sibling{};
		JSON* last_sibling{};
	};

	NestedData* JSON::current()
	{
		return this;
	}

	NestedData* JSON::next()
	{
		return next_sibling;
	}

	NestedData* JSON::previous()
	{
		return prev_sibling;
	}

	NestedData* JSON::first()
	{
		return first_sibling;
	}

	NestedData* JSON::last()
	{
		return last_sibling;
	}

	std::deque<std::string> JSON::children_names()
	{
		std::deque<std::string> _names;

		for (const auto& _child : _children)
			_names.push_back(_child.first);

		return _names;
	}

	std::deque<NestedData*> JSON::children()
	{
		std::deque<NestedData*> _return_children;

		for (auto& _child : _children)
			_return_children.push_back(_child.second.get());

		return _return_children;
	}

	JSON::Reference JSON::operator[](const std::string& _name)
	{
		return Reference(*this, _name);
	}

	const NestedValue& JSON::operator[](
		const std::string& _name) const
	{
		auto _it = _children.find(_name);

		if (_it == _children.end())
			throw JSONError("JSON child not found");

		if (!_it->second->_value)
			throw JSONError("JSON child has no value");

		return *_it->second->_value;
	}

	JSON& JSON::prepend(std::pair<std::string, NestedValue*> _pair)
	{
		auto _json = std::make_unique<JSON>();
		_json->_value = _pair.second;

		JSON* _child = _json.get();

		if (first_sibling)
		{
			_child->next_sibling = first_sibling;
			first_sibling->prev_sibling = _child;
		}
		else
		{
			last_sibling = _child;
		}

		first_sibling = _child;

		_children.emplace(
			std::move(_pair.first),
			std::move(_json)
		);

		return *this;
	}

	JSON& JSON::append(std::pair<std::string, NestedValue*> _pair)
	{
		auto _json = std::make_unique<JSON>();
		_json->_value = _pair.second;

		JSON* _child = _json.get();

		if (last_sibling)
		{
			_child->prev_sibling = last_sibling;
			last_sibling->next_sibling = _child;
		}
		else
		{
			first_sibling = _child;
		}

		last_sibling = _child;

		_children.emplace(
			std::move(_pair.first),
			std::move(_json)
		);

		return *this;
	}

	JSON& JSON::prepend(std::string _name, NestedValue* _value)
	{
		return prepend(std::make_pair(
			std::move(_name),
			_value
		));
	}

	JSON& JSON::append(std::string _name, NestedValue* _value)
	{
		return append(std::make_pair(
			std::move(_name),
			_value
		));
	}

	JSON& JSON::insert(
		std::pair<std::string, NestedValue*> _pair,
		size_t _index)
	{
		if (_index > _children.size())
			throw JSONError("JSON insertion index out of range");

		if (_index == 0)
			return prepend(std::move(_pair));

		if (_index == _children.size())
			return append(std::move(_pair));

		JSON* _current = first_sibling;

		for (size_t _i = 0; _i < _index; ++_i)
			_current = _current->next_sibling;

		auto _json = std::make_unique<JSON>();
		_json->_value = _pair.second;

		JSON* _child = _json.get();
		JSON* _previous = _current->prev_sibling;

		_child->prev_sibling = _previous;
		_child->next_sibling = _current;

		_previous->next_sibling = _child;
		_current->prev_sibling = _child;

		_children.emplace(
			std::move(_pair.first),
			std::move(_json)
		);

		return *this;
	}

	JSON& JSON::insert(
		std::string _name,
		NestedValue* _value,
		size_t _index)
	{
		return insert(
			std::make_pair(
				std::move(_name),
				_value
			),
			_index
		);
	}

	JSON& JSON::insert_after(
		std::pair<std::string, NestedValue*> _pair,
		std::string _name)
	{
		auto _it = _children.find(_name);

		if (_it == _children.end())
			throw JSONError("JSON child not found");

		JSON* _current = _it->second.get();

		auto _json = std::make_unique<JSON>();
		_json->_value = _pair.second;

		JSON* _child = _json.get();
		JSON* _next = _current->next_sibling;

		_child->prev_sibling = _current;
		_child->next_sibling = _next;

		_current->next_sibling = _child;

		if (_next)
			_next->prev_sibling = _child;
		else
			last_sibling = _child;

		_children.emplace(
			std::move(_pair.first),
			std::move(_json)
		);

		return *this;
	}

	JSON& JSON::insert_after(
		std::string _name,
		NestedValue* _value,
		std::string _after)
	{
		return insert_after(
			std::make_pair(
				std::move(_name),
				_value
			),
			std::move(_after)
		);
	}

	JSON& JSON::insert_before(
		std::pair<std::string, NestedValue*> _pair,
		std::string _name)
	{
		auto _it = _children.find(_name);

		if (_it == _children.end())
			throw JSONError("JSON child not found");

		JSON* _current = _it->second.get();

		if (_current == first_sibling)
			return prepend(std::move(_pair));

		auto _json = std::make_unique<JSON>();
		_json->_value = _pair.second;

		JSON* _child = _json.get();
		JSON* _previous = _current->prev_sibling;

		_child->prev_sibling = _previous;
		_child->next_sibling = _current;

		_previous->next_sibling = _child;
		_current->prev_sibling = _child;

		_children.emplace(
			std::move(_pair.first),
			std::move(_json)
		);

		return *this;
	}

	JSON& JSON::insert_before(
		std::string _name,
		NestedValue* _value,
		std::string _before)
	{
		return insert_before(
			std::make_pair(
				std::move(_name),
				_value
			),
			std::move(_before)
		);
	}
	JSON json()
	{
		return JSON{};
	}

	namespace detail
	{
		void parse_object(
			JSON& _json,
			std::string_view _input,
			size_t& _index);
		void skip_whitespace(
			std::string_view _input,
			size_t& _index)
		{
			while (_index < _input.size() &&
				(_input[_index] == ' ' ||
					_input[_index] == '\t' ||
					_input[_index] == '\n' ||
					_input[_index] == '\r'))
			{
				++_index;
			}
		}

		std::string parse_string(
			std::string_view _input,
			size_t& _index)
		{
			if (_index >= _input.size() ||
				_input[_index] != '"')
			{
				throw JSONError("Expected \" character");
			}

			++_index;

			std::string _value;

			while (_index < _input.size())
			{
				if (_input[_index] == '"')
				{
					++_index;
					return _value;
				}

				if (_input[_index] == '\\')
				{
					++_index;

					if (_index >= _input.size())
						throw JSONError("Invalid escape sequence");

					switch (_input[_index])
					{
					case '"':
						_value += '"';
						break;

					case '\\':
						_value += '\\';
						break;

					case '/':
						_value += '/';
						break;

					case 'b':
						_value += '\b';
						break;

					case 'f':
						_value += '\f';
						break;

					case 'n':
						_value += '\n';
						break;

					case 'r':
						_value += '\r';
						break;

					case 't':
						_value += '\t';
						break;

					default:
						throw JSONError("Invalid escape sequence");
					}

					++_index;
					continue;
				}

				_value += _input[_index];
				++_index;
			}

			throw JSONError("Expected \" character");
		}

		void expect(
			std::string_view _input,
			size_t& _index,
			char _character)
		{
			skip_whitespace(_input, _index);

			if (_index >= _input.size() ||
				_input[_index] != _character)
			{
				throw JSONError(
					std::string("Expected ") + _character
				);
			}

			++_index;
		}

		std::string parse_key(
			std::string_view _input,
			size_t& _index)
		{
			skip_whitespace(_input, _index);
			return parse_string(_input, _index);
		}

		NestedValue* parse_value(
			std::string_view _input,
			size_t& _index)
		{
			skip_whitespace(_input, _index);

			if (_index >= _input.size())
				throw JSONError("Expected JSON value");

			if (_input[_index] == '"')
			{
				return new JSONValue(
					parse_string(_input, _index)
				);
			}

			if (_input.substr(_index, 4) == "true")
			{
				_index += 4;
				return new JSONValue(true);
			}

			if (_input.substr(_index, 5) == "false")
			{
				_index += 5;
				return new JSONValue(false);
			}

			if (_input.substr(_index, 4) == "null")
			{
				_index += 4;
				return new JSONValue(nullptr);
			}

			size_t _start = _index;

			if (_input[_index] == '-')
				++_index;

			bool _decimal{ false };

			while (_index < _input.size())
			{
				char _character = _input[_index];

				if (_character >= '0' &&
					_character <= '9')
				{
					++_index;
					continue;
				}

				if (_character == '.' &&
					!_decimal)
				{
					_decimal = true;
					++_index;
					continue;
				}

				break;
			}

			if (_index == _start ||
				(_index == _start + 1 &&
					_input[_start] == '-'))
			{
				throw JSONError("Invalid JSON value");
			}

			std::string _number{
				_input.substr(
					_start,
					_index - _start
				)
			};

			try
			{
				if (_decimal)
				{
					return new JSONValue(
						std::stod(_number)
					);
				}

				return new JSONValue(
					std::stoi(_number)
				);
			}
			catch (...)
			{
				throw JSONError("Invalid JSON number");
			}
		}

		void parse_array(
			JSON& _json,
			std::string_view _input,
			size_t& _index)
		{
			expect(_input, _index, '[');

			skip_whitespace(_input, _index);

			if (_index >= _input.size())
				throw JSONError("Expected ] character");

			if (_input[_index] == ']')
			{
				++_index;
				return;
			}

			size_t _element{ 0 };

			while (true)
			{
				skip_whitespace(_input, _index);

				if (_index >= _input.size())
					throw JSONError("Expected ] character");

				std::string _key =
					std::to_string(_element++);

				if (_input[_index] == '{')
				{
					auto _child =
						std::make_unique<JSON>();

					parse_object(
						*_child,
						_input,
						_index
					);

					_json.append(
						std::move(_key),
						_child.release()
					);
				}
				else if (_input[_index] == '[')
				{
					auto _child =
						std::make_unique<JSON>();

					parse_array(
						*_child,
						_input,
						_index
					);

					_json.append(
						std::move(_key),
						_child.release()
					);
				}
				else
				{
					NestedValue* _value =
						parse_value(
							_input,
							_index
						);

					_json.append(
						std::move(_key),
						_value
					);
				}

				skip_whitespace(_input, _index);

				if (_index >= _input.size())
					throw JSONError(
						"Expected , or ] character"
					);

				if (_input[_index] == ']')
				{
					++_index;
					return;
				}

				if (_input[_index] != ',')
				{
					throw JSONError(
						"Expected , or ] character"
					);
				}

				++_index;

				skip_whitespace(_input, _index);

				if (_index >= _input.size())
					throw JSONError(
						"Expected JSON array value"
					);

				if (_input[_index] == ']')
				{
					throw JSONError(
						"Trailing comma in JSON array"
					);
				}
			}
		}

		void parse_object(
			JSON& _json,
			std::string_view _input,
			size_t& _index)
		{
			expect(_input, _index, '{');

			skip_whitespace(_input, _index);

			if (_index >= _input.size())
				throw JSONError("Expected } character");

			if (_input[_index] == '}')
			{
				++_index;
				return;
			}

			while (true)
			{
				if (_index >= _input.size() ||
					_input[_index] != '"')
				{
					throw JSONError(
						"Expected JSON object key"
					);
				}

				std::string _key =
					parse_key(_input, _index);

				expect(_input, _index, ':');

				skip_whitespace(_input, _index);

				if (_index >= _input.size())
					throw JSONError("Expected JSON value");

				if (_input[_index] == '{')
				{
					auto _child =
						std::make_unique<JSON>();

					parse_object(
						*_child,
						_input,
						_index
					);

					_json.append(
						std::move(_key),
						_child.release()
					);
				}
				else if (_input[_index] == '[')
				{
					auto _child =
						std::make_unique<JSON>();

					parse_array(
						*_child,
						_input,
						_index
					);

					_json.append(
						std::move(_key),
						_child.release()
					);
				}
				else
				{
					NestedValue* _value =
						parse_value(
							_input,
							_index
						);

					_json.append(
						std::move(_key),
						_value
					);
				}

				skip_whitespace(_input, _index);

				if (_index >= _input.size())
					throw JSONError(
						"Expected , or } character"
					);

				if (_input[_index] == '}')
				{
					++_index;
					return;
				}

				if (_input[_index] != ',')
				{
					throw JSONError(
						"Expected , or } character"
					);
				}

				++_index;

				skip_whitespace(_input, _index);

				if (_index >= _input.size())
					throw JSONError(
						"Expected JSON object key"
					);

				if (_input[_index] == '}')
				{
					throw JSONError(
						"Trailing comma in JSON object"
					);
				}
			}
		}
	}

	JSON parse_jsonln(std::string_view _jsonln)
	{
		size_t _index{ 0 };

		detail::skip_whitespace(
			_jsonln,
			_index
		);

		JSON _json{};

		detail::parse_object(
			_json,
			_jsonln,
			_index
		);

		detail::skip_whitespace(
			_jsonln,
			_index
		);

		if (_index != _jsonln.size())
			throw JSONError(
				"Unexpected characters after JSON value"
			);

		return _json;
	}

	JSON parse_json(std::string_view _json)
	{
		size_t _index{ 0 };

		detail::skip_whitespace(
			_json,
			_index
		);

		JSON _json_result{};

		detail::parse_object(
			_json_result,
			_json,
			_index
		);

		detail::skip_whitespace(
			_json,
			_index
		);

		if (_index != _json.size())
			throw JSONError(
				"Unexpected characters after JSON value"
			);

		return _json_result;
	}

	JSON json(std::string_view _input)
	{
		return parse_json(_input);
	}

	JSON json(const char* _input)
	{
		return parse_json(
			std::string_view{ _input }
		);
	}

	JSON json(std::istream& _input)
	{
		std::stringstream _stream;

		_stream << _input.rdbuf();

		return parse_json(_stream.str());
	}
}