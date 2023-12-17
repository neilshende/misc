#include <iostream>
#include <string>
#include <vector>
#include <json/json.h>

using namespace std;

// Define a simple struct
struct Person {
  string name;
  int age;
  vector<string> hobbies;
};

// JSON marshaling function
string json_marshal(const Person& person) {
  Json::Value root;

  root["name"] = person.name;
  root["age"] = person.age;

  // Add hobbies as an array
  Json::Value hobbiesArray(Json::arrayValue);
  for (const string& hobby : person.hobbies) {
    hobbiesArray.append(hobby);
  }
  root["hobbies"] = hobbiesArray;

  // Convert the JSON object to a string
  Json::StreamWriterBuilder builder;
  string jsonString = Json::writeString(builder, root);

  return jsonString;
}

int main() {
  // Create a person object
  Person person;
  person.name = "John Doe";
  person.age = 30;
  person.hobbies = {"Reading", "Hiking", "Coding"};

  // Convert the person object to JSON
  string jsonString = json_marshal(person);

  // Print the JSON string
  cout << jsonString << endl;

  return 0;
}
