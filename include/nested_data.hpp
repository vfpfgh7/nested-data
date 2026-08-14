#pragma once
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <deque>
#include <string>
#include <functional>
#include <variant>
namespace nested_data {
	class NestedValue 
	{
	public:
		using Value = std::variant<
			std::nullptr_t,
			bool,
			int,
			double,
			std::string
		>;

		virtual ~NestedValue() = default;
		virtual const Value get() const = 0;

		virtual void set(std::string _value) = 0;
		virtual void set(bool _value) = 0;
		virtual void set(int _value) = 0;
		virtual void set(double _value) = 0;
		virtual void set(const char* _value) = 0;

		virtual Value get() = 0;

		friend std::ostream& operator<<(std::ostream&, const NestedValue&);
		friend std::istream& operator>>(std::istream&, const NestedValue&);
	};
	class NestedData 
	{
	public:
		NestedData() = default;
		virtual ~NestedData() = default;

		virtual NestedData* current() = 0;
		virtual NestedData* next() = 0;
		virtual NestedData* first() = 0;
		virtual NestedData* previous() = 0;
		virtual NestedData* last() = 0;

		virtual std::deque<NestedData*> children() = 0;
        virtual std::deque<std::string> children_names() = 0;

        virtual void traverse(
            const std::function<void(
                NestedData&,
                const std::function<void(NestedData&)>&
                )>& _strategy,
            const std::function<void(NestedData&)>& _action = {}) = 0;

        virtual NestedValue* value() = 0;
        virtual const NestedValue* value() const = 0;
	};
	std::ostream& operator<<(std::ostream& _stream, const nested_data::NestedValue& _value)
	{
        std::cout << std::boolalpha;
		std::visit([&_stream](const auto& _item)
			{
				if constexpr (std::is_same_v<std::decay_t<decltype(_item)>, std::nullptr_t>)
					_stream << "null";
				else
					_stream << _item;
			}, _value.get());

		return _stream;
	}
    std::istream& operator>>(
        std::istream& _stream,
        NestedValue& _value)
    {
        std::string _input;
        _stream >> _input;

        if (_input == "null")
        {
            _value.set(nullptr);
            return _stream;
        }

        if (_input == "true")
        {
            _value.set(true);
            return _stream;
        }

        if (_input == "false")
        {
            _value.set(false);
            return _stream;
        }

        try
        {
            size_t _position{};

            int _integer =
                std::stoi(_input, &_position);

            if (_position == _input.size())
            {
                _value.set(_integer);
                return _stream;
            }
        }
        catch (...)
        {
        }

        try
        {
            size_t _position{};

            double _double =
                std::stod(_input, &_position);

            if (_position == _input.size())
            {
                _value.set(_double);
                return _stream;
            }
        } catch (...)
        {
        }

        _value.set(_input);

        return _stream;
    }

    void preorder(
        NestedData& _data,
        const std::function<void(NestedData&)>& _action)
    {
        if (_action)
            _action(_data);

        for (NestedData* _child = _data.first();
            _child != nullptr;
            _child = _child->next())
        {
            preorder(*_child, _action);
        }
    }

    void postorder(
        NestedData& _data,
        const std::function<void(NestedData&)>& _action)
    {
        for (NestedData* _child = _data.first();
            _child != nullptr;
            _child = _child->next())
        {
            postorder(*_child, _action);
        }

        if (_action)
            _action(_data);
    }

    void inorder(
        NestedData& _data,
        const std::function<void(NestedData&)>& _action)
    {
        auto _children = _data.children();

        if (_children.empty())
        {
            if (_action)
                _action(_data);

            return;
        }

        size_t _middle = _children.size() / 2;

        for (size_t _i = 0; _i < _children.size(); ++_i)
        {
            if (_i == _middle && _action)
                _action(_data);

            if (_children[_i])
                inorder(*_children[_i], _action);
        }
    }
}