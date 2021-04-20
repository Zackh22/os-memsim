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
    int arrToTrackOpenFrames[(67108864 / _page_size)] = {0};
    if(_table.count(entry) == 0){ //entry is NOT in table yet
        int frame = 0; 
        // Find free frame
        if(!_table.empty()){
            for(const auto &i : _table){
                arrToTrackOpenFrames[i.second] = 1; // marking this frame in arr as one that is used in page table
                if(i.second >= frame){
                    frame = i.second;
                }
            }
            frame++;
        }

        for(int i = 0; i < frame; i++){
            if(arrToTrackOpenFrames[i] == 0){
                //frame is not in current table entries so it should be used first
                frame = i;
            }
        }

        _table[entry] = frame; 
    }
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    int page_number = getPageNumber(virtual_address); 
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

int PageTable::getPageSize(){
    return _page_size;
}

void PageTable::removeEntry(uint32_t pid, int page_number){
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    _table.erase(entry);
}
