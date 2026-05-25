#include <climits>
#include <cstdint>
#include<iostream>
#include<fstream>
#include<vector>
#include<sstream>
#include<functional>
#include<algorithm>

std::fstream file;
std::fstream segment;

struct Bitmap {
    std::vector<uint32_t> bits;

    void add(int userId) {
        int wordIndex = userId / 32;
        int bitIndex = userId % 32;

        if (wordIndex >= bits.size())
            bits.resize(wordIndex + 1, 0);

        bits[wordIndex] |= (1u << bitIndex);
    }

    bool contains(int userId) const {
        int wordIndex = userId / 32;
        int bitIndex = userId % 32;

        if (wordIndex >= bits.size())
            return false;

        return (bits[wordIndex] & (1u << bitIndex)) != 0;
    }
};

struct Run {
    int start;
    int length;
};

struct RunContainer {
    std::vector<Run> runs;

    void add(int x) {
        if (runs.empty()) {
            runs.push_back({x, 1});
            return;
        }

        Run &last = runs.back();

        if (x == last.start + last.length) {
            last.length++;
        } else {
            runs.push_back({x, 1});
        }
    }
    bool contains(int x) const {
        for (const auto &r : runs) {
            if (x >= r.start && x < r.start + r.length)
                return true;
        }
        return false;
    }

};
struct ArrayContainer{
    std::vector<uint32_t> values;

    void add(int x) {
        values.insert(std::lower_bound(values.begin(), values.end(), x),x);
    }

    bool contains(int x) const{
        return std::binary_search(values.begin(), values.end(), x);
    }
};

struct Segment {
    enum Type{ 
        ARRAY,
        BITMAP,
        RUN
    };
    Type type;

    ArrayContainer array;
    Bitmap bitmap;
    RunContainer run;
};
void segmentAccounts(int age,
        const std::string& filename,
        std::function<bool(int, int)> condition);

int main(){
    int choice, age;
    std::cout << "Would you like to segment accounts greator than(0), equal(1) or less(2) than a specific account age?\n";
    std::cin >> choice;

    std::cout << "Enter the account age to segment\n";
    std::cin >> age;

    file.open("users.csv", std::ios::in);
    Bitmap* bitm;
    switch(choice){
        case 0:
            segmentAccounts(
                    age,
                    "segment_greater_than_" + std::to_string(age) + ".csv",
                    [](int days, int age){
                    return days > age;
                    }
                    );
            break;

        case 1:
            segmentAccounts(
                    age,
                    "segment_equal_to_" + std::to_string(age) + ".csv",
                    [](int days, int age){
                    return days == age;
                    }
                    );
            break;

        case 2:
            segmentAccounts(
                    age,
                    "segment_less_than_" + std::to_string(age) + ".csv",
                    [](int days, int age){
                    return days < age;
                    }
                    );
            break;
    }

    return 0;
}

// generic function that takes the comparison as a parameter
void segmentAccounts(int age,
        const std::string& filename,
        std::function<bool(int, int)> condition){

    segment.open("output/" + filename, std::ios::out | std::ios::trunc);

    std::string line, word;
    int userId, days;

    std::vector<int> temp;
    int count = 0;
    int numRuns = 0;
    bool prevMatched = false;
    Segment seg;

    int minId = -1;
    int maxId = -1;

    file.clear();
    file.seekg(0);
    
    // Analyze:
    while (std::getline(file, line)) {
        std::stringstream s(line);

        getline(s, word, ',');
        userId = std::stoi(word);

        getline(s, word, ',');
        days = std::stoi(word);

        bool matched = condition(days, age);
        if (matched) {
            temp.push_back(userId);

            count++;
            if (!prevMatched) numRuns++;

            if (minId == -1) minId = userId;
            maxId = std::max(maxId, userId);
        }
        prevMatched = matched;
    }

    // This is just for theory. To actually get a Run to be chosen Roaring Bitmaps need to be implemented. 
    // Should be a placeholder for now 
    int costArray = count * 4;
    int costBitmap = count > 0 ? ((maxId / 32) + 1) * 4 : INT_MAX;
    int costRun = numRuns * 8;

    // Pick:
    if (costArray <= costBitmap && costArray <= costRun) {
        seg.type = Segment::ARRAY;
    }
    else if (costRun <= costBitmap && costRun <= costArray) {
        seg.type = Segment::RUN;
    }
    else {
        seg.type = Segment::BITMAP;
    }
 
    // might be needed for Run later
    std::sort(temp.begin(), temp.end());

    segment << "Segmentation method: ";
    if (seg.type == Segment::ARRAY) {
        segment << "ARRAY\n";
    }
    else if (seg.type == Segment::RUN) {
        segment << "RUN\n";
    }
    else {
        segment << "BITMAP\n";
    }

    // Populate
    for (int id : temp) {
        if (seg.type == Segment::ARRAY)
            seg.array.add(id);
        else if (seg.type == Segment::RUN)
            seg.run.add(id);
        else
            seg.bitmap.add(id);
    }

    // Output to file
    segment << "Values:\n";

    if (seg.type == Segment::ARRAY) {
        for (auto v : seg.array.values)
            segment << v << "\n";
    }
    else if (seg.type == Segment::RUN) {
        for (auto &r : seg.run.runs)
            segment << "[" << r.start << ", " << (r.start + r.length - 1) << "]\n";
    }
    else {
        segment << "(bitmap stored, no direct print)\n";
    }

    segment.close();
}
