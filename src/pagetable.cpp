#include "pagetable.h"
#include <math.h>

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    int frame = 0; 
    // Find free frame
    if(!_table.empty()){
        for(const auto &i : _table){
            if(i.second >= frame){
                frame = i.second;
            }
        }
        frame++;
    }
    _table[entry] = frame; 
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    int page_number = virtual_address / _page_size; 
    int page_offset = virtual_address % _page_size; 

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        address = _table[entry] * page_number + page_offset; 
    }

    return address;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++)
    {
        std::string toSplit = "|";
        std::string tempString = keys.at(i);
        size_t position = tempString.find(toSplit);
        std::string pid = tempString.substr(0,position);
        tempString.erase(0, (position + toSplit.length()));
        int frameNum = _table[keys.at(i)];
        printf(" %4s | %11s | %12d \n", pid.c_str(), tempString.c_str(), frameNum);     
    }
}

int PageTable::getPageNumber(uint32_t virtual_address){ 
    return virtual_address / _page_size;
}

int PageTable::getPageNumberRounded(uint32_t virtual_address){
    return ceil(((float)virtual_address / _page_size));
}