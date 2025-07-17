#include <unordered_map>
#include <iostream>
#include <string>

using namespace std;

int main()
{
  unordered_map<string, int> mp;
  mp["Hello"] = 10;
  mp["World"] = 20;

  for (auto key : mp) {
    cout << key.first << endl;
  }

  return 0;
}
