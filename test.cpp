#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "linked_list.tpp"

using namespace std;

int main()
{
    linked_list_root<int> root;
    root.push_back(1);
    root.push_back(2);
    cout<<root;
    return 0;
}