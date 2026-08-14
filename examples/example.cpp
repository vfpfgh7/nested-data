#include "..\include\json.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

using nested_data::NestedData;
using nested_data::preorder;
using nested_data::postorder;
using nested_data::inorder;

int main()
{
    try
    {
        // Read JSON from standard input.
        nested_data::json::JSON obj =
            nested_data::json::json(std::cin);

        // Read JSON from a file.
        std::ifstream file("test.json");

        if (!file)
            throw nested_data::json::JSONError(
                "Could not open test.json"
            );

        nested_data::json::JSON obj2 =
            nested_data::json::json(file);

        // Create an empty JSON object.
        nested_data::json::JSON obj3 =
            nested_data::json::json();

        obj3["name"] = "Steve";
        obj3["age"] = 42;
        obj3["active"] = true;
        obj3["score"] = 98.5;
        obj3["nothing"] = nullptr;

        std::cout << obj3["name"] << '\n';
        std::cout << obj3["age"] << '\n';
        std::cout << obj3["active"] << '\n';
        std::cout << obj3["score"] << '\n';
        std::cout << obj3["nothing"] << '\n';

        // Create JSON directly from a string.
        nested_data::json::JSON obj4 =
            nested_data::json::json(
                R"({
                    "name": "Alex",
                    "age": 30,
                    "developer": true,
                    "scores": [100, 94, 32, 65],
                    "person": {
                        "name": "Bob",
                        "age": 20
                    }
                })"
            );

        std::cout << "\nName: "
            << obj4["name"] << '\n';

        std::cout << "Age: "
            << obj4["age"] << '\n';

        std::cout << "Developer: "
            << obj4["developer"] << '\n';

        // Access a nested object.
        NestedData& person = obj4.child("person");

        std::cout << "\nPerson:\n";

        for (const auto& name : person.children_names())
            std::cout << "  " << name << '\n';

        // Modify an existing value.
        obj4["name"] = "Charlie";

        std::cout << "\nModified name: "
            << obj4["name"] << '\n';

        // Demonstrate the stream input operator.
        std::stringstream input("123");

        input >> obj3["age"];

        std::cout << "New age: "
            << obj3["age"] << '\n';

        // Demonstrate ordered navigation.
        std::cout << "\nChildren in document order:\n";

        for (NestedData* child = obj4.first();
            child != nullptr;
            child = child->next())
        {
            if (child->value())
                std::cout << *child->value() << '\n';
            else
                std::cout << "Object / array\n";
        }

        // Preorder traversal.
        std::cout << "\n=== Preorder ===\n";

        obj4.traverse(
            preorder,
            [](NestedData& data)
            {
                if (data.value())
                    std::cout
                    << "Value: "
                    << *data.value()
                    << '\n';
                else
                    std::cout
                    << "Object / array\n";
            }
        );

        // Postorder traversal.
        std::cout << "\n=== Postorder ===\n";

        obj4.traverse(
            postorder,
            [](NestedData& data)
            {
                if (data.value())
                    std::cout
                    << "Value: "
                    << *data.value()
                    << '\n';
                else
                    std::cout
                    << "Object / array\n";
            }
        );

        // Inorder traversal.
        std::cout << "\n=== Inorder ===\n";

        obj4.traverse(
            inorder,
            [](NestedData& data)
            {
                if (data.value())
                    std::cout
                    << "Value: "
                    << *data.value()
                    << '\n';
                else
                    std::cout
                    << "Object / array\n";
            }
        );

        // Demonstrate polymorphism through NestedData.
        std::cout << "\n=== NestedData polymorphism ===\n";

        NestedData* data = &obj4;

        data->traverse(
            preorder,
            [](NestedData& node)
            {
                if (node.value())
                    std::cout
                    << "Traversed value: "
                    << *node.value()
                    << '\n';
            }
        );

        // Demonstrate istream polymorphism with stringstream.
        std::stringstream stream(
            R"({
                "message": "Hello",
                "number": 123
            })"
        );

        nested_data::json::JSON obj5 =
            nested_data::json::json(stream);

        std::cout << "\n=== Stringstream ===\n";

        obj5.traverse(
            preorder,
            [](NestedData& data)
            {
                if (data.value())
                    std::cout << *data.value() << '\n';
            }
        );
    }
    catch (const nested_data::json::JSONError& error)
    {
        std::cerr
            << "JSON error: "
            << error.what()
            << '\n';

        return 1;
    }

    return 0;
}