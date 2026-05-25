#include <iostream>
#include <fstream>
#include <ctime> // for seeding with time

struct {
    int userId;
    int daysSinceJoined;   
} User;

int main(){
    std::srand(std::time(0));
    int numToGen;
    int id = 1;

    std::cout << "How many users would you like to generate?\n";
    std::cin >> numToGen;

    // create the file 
    std::fstream file;
    file.open("users.csv", std::ios::out | std::ios::trunc);

    // write to file
    for(int x = 0; x<numToGen; x++){
        int randomDays = rand() % 501;      // example 500 as the upper end, this generates between 0 and 500
        file << id << ", "
             << randomDays
             << "\n";
        id++;
    }
    
    // close the file
    file.close();
    return 0;
}

