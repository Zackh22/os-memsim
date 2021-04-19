#include <iostream>
#include <string>
#include <cstring>
#include "mmu.h"
#include "pagetable.h"

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);
void splitString(std::string text, char d, std::vector<std::string>& result);
int allNums(std::string checkString);
void printVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table, void *memory);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory'
    uint32_t mem_size = 67108864;
    void *memory = malloc(mem_size); // 64 MB (64 * 1024 * 1024)
    //for setting or printing a variable

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(mem_size);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline (std::cin, command);
    std::vector<std::string> commandSplit;
    splitString(command, ' ', commandSplit);
    while (commandSplit.at(0) != "exit") {
        // Handle command
        if(commandSplit.at(0) == "create"){ //create <text_size> <data_size>
            int textSize = allNums(commandSplit.at(1));
            int dataSize = allNums(commandSplit.at(2));
            createProcess(textSize, dataSize, mmu, page_table);
            
            //assign a process id
            //allocate some amount of startup memory for the process
                //text/code: size of binary executable - user specified number (2048-16384 bytes)
                //Data/Globals: size of global variables - user specified number (0 - 1024 bytes)
                //Stack: constant (65536 bytes)
            //prints the PID
            std::cout << std::endl;
        }else if(commandSplit.at(0) == "allocate"){ //allocate <PID> <var_name> <data_type> <number_of_elements>
            //Allocated memory on the heap (how mcuch depends on the data type and the number of elements)
                //N chars (N bytes)
                //N shorts (N * 2 bytes)
                //N ints/floats (N * 4 bytes)
                //N longs/doubles (N * 8 bytes)
            //print the virtual memory address
            uint32_t pid = allNums(commandSplit.at(1));
            if(mmu->processExists(pid)){
                std::string varName = commandSplit.at(2);
                Variable *var = mmu->getVariable(pid, varName);
                if(var == NULL){
                    DataType type;
                    if(commandSplit.at(3) == "char"){
                        type = DataType::Char;
                    }else if(commandSplit.at(3) == "double"){
                        type = DataType::Double;
                    }else if(commandSplit.at(3) == "Float"){
                        type = DataType::Float;
                    }else if(commandSplit.at(3) == "int"){
                        type = DataType::Int;
                    }else if(commandSplit.at(3) == "long"){
                        type = DataType::Long;
                    }else if(commandSplit.at(3) == "short"){
                        type = DataType::Short;
                    }
                    uint32_t numElements = allNums(commandSplit.at(4));
                    allocateVariable(pid, varName, type, numElements, mmu, page_table);
                }else {
                    std::cout << "error: variable already exists";
                }                
            }else {
                std::cout << "error: process not found";
            }
            std::cout << std::endl;
        }else if(commandSplit.at(0) == "set"){ //set <PID> <var_name> <offset> <value_0> <value_2> ... <value_N>
            uint32_t pid = allNums(commandSplit.at(1));
            if(mmu->processExists(pid)){
                std::string varName = commandSplit.at(2);
                Variable *var = mmu->getVariable(pid, varName);
                if(var != NULL){
                    //do setting process here
                    uint32_t offset = allNums(commandSplit.at(3));
                    uint32_t offsetInc = offset;
                    for(int i = 4; i < commandSplit.size(); i++){
                        std::string value = commandSplit.at(i);
                        setVariable(pid, varName, offsetInc, &value, mmu, page_table, memory);
                        offsetInc++;
                    }
                }else {
                    std::cout << "error: variable not found"<<std::endl;
                }
            }else {
                std::cout << "error: process not found" <<std::endl;
            }
            //sote integer, float, or character values in memeory
            //Set the value for the variable <var_name> starting at <offset>
            //NOTE: multiple contiguous values can be set with one command
        }else if(commandSplit.at(0) == "print"){ //print <object>
            //if <object> is "mmu", print the MMU memory table
            if(commandSplit.at(1) == "mmu"){
                mmu->print();
            }else if(commandSplit.at(1) == "page"){  //if <object> is "page", print the page table (do not need to print anything for free frames)
                page_table->print();
            }else if(commandSplit.at(1) == "processes"){//if <object> is "process", print a list of PID's for processes that are still running
                mmu->printProcesses();
            }else{ 
                //if <object> is a "<PID>":<var_name>", print the value of the variable for that process"
                //If variable has more than 4 elements, just print the first 4 followed by "... [N items]" (where N is the number of elements)
                std::string toSplit = ":";
                std::string tempString = commandSplit.at(1);
                size_t position = tempString.find(toSplit);
                std::string stringPid = tempString.substr(0,position);
                uint32_t pid = allNums(stringPid);
                tempString.erase(0, (position + toSplit.length()));
                printVariable(pid, tempString, mmu, page_table, memory);
            }
        }else if(commandSplit.at(0) == "free"){ //free <PID> <var_name>
            uint32_t pid = allNums(commandSplit.at(1));
            if(mmu->processExists(pid)){
                std::string varName = commandSplit.at(2);
                Variable *var = mmu->getVariable(pid, varName);
                if(var != NULL){
                    //do freeing process here
                }else {
                    std::cout << "error: variable not found";
                }
            }else {
                std::cout << "error: process not found";
            }
            //Deallocate memory on the heap that is associated with <var_name>
                //N chars (N bytes)
                //N shorts (N * 2 bytes)
                //N ints/floats (N * 4 bytes)
                //N longs/doubles (N * 8 bytes)
                //Can multiple contiguous vales be deallocated with one command?
            std::cout << std::endl;
        }else if(commandSplit.at(0) == "terminate"){ //terminate <PID>
            uint32_t pid = allNums(commandSplit.at(1));
            if(mmu->processExists(pid)){
                //do termination process here
            }else {
                std::cout << "error: process not found";
            }
            //Kill the specified process
            //Free all memory associated with this process
            //Deallocate all memory associated with the process
            std::cout << std::endl;
        }else{ //error
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
        splitString(command, ' ', commandSplit);
    }

    // Clean up
    free(memory);
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    //   - create new process in the MMU 
    uint32_t pid = mmu->createProcess();
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    //find first space that is big enough for them (1 + pages)
    allocateVariable(pid, "<TEXT>", DataType::Char, text_size, mmu, page_table);
    allocateVariable(pid, "<GLOBALS>", DataType::Char, data_size, mmu, page_table);
    allocateVariable(pid, "<STACK>", DataType::Char, 65536, mmu, page_table);
    //   - print pid
    std::cout << pid;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    //CHECK FOR IF THIS ALLOCATION WOULD EXCEED SYSTEM MEMOMRY (in bytes) 67108864!!
    // mmu->_processes->variables --> see if this equals free space then get the variable and call the size. Have a check fo PID;
    // If space is big enough then put variable there. 
    uint32_t newVarSize;
    if(type == DataType::Char){
        newVarSize = num_elements;
    }else if(type == DataType::Double || type == DataType::Long){
        newVarSize = num_elements * 8;
    }else if(type == DataType::Float || type == DataType::Int){
        newVarSize = num_elements * 4;
    }else if(type == DataType::Short){
        newVarSize = num_elements * 2;
    }

    Variable* freeSpace = mmu->getVariable(pid, "<FREE_SPACE>");

    if(!(freeSpace == NULL)){
        uint32_t newVarAddress = freeSpace->virtual_address;
        if(freeSpace->size >= newVarSize){
            //add var to mmu
            if(mmu->spaceLeft(newVarSize)){
                freeSpace->size = (freeSpace->size - newVarSize);
                freeSpace->virtual_address = (freeSpace->virtual_address + newVarSize);
                mmu->addVariableToProcess(pid, var_name, type, newVarSize, newVarAddress);
                //update page table
                int startPage = page_table->getPageNumberRounded(newVarAddress);
                int endPage = page_table->getPageNumberRounded(freeSpace->virtual_address);
                int difference = endPage - startPage;
                if(!difference == 0){
                    for(int i = startPage; i < endPage; i++){
                        page_table->addEntry(pid, i);
                    }
                }

                //print virtual address
                if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                    std::cout << newVarAddress;
                }
            }else {
                std::cout << "Allocation would exceed system memory"<<std::endl;
            }
        }
    }

    //first search the page table to see if there is a location your variable will fit w/o allocating a new page.
    //if there is space then dont need to touch page table at all.
    //2 bytes left on page but trying to allocate 2 byte integer -> DO NOt SPLit an individual item accros pages. need to have contiguous memory addresses
    //in mmu loop through adn find the free space variables.  
    //to know what page to pass in to the page table.addentry -> add methods to page table
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address (adding on the size in hex)
    // only use actual memory when setting and printing a variable.
}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    //   - look up physical address for variable based on its virtual address / offset
    Variable *var = mmu->getVariable(pid, var_name);
    DataType type = var->type;
    uint32_t size;
    std::string *x = static_cast<std::string*>(value);
    std::string::size_type sz;
    
    if(type == DataType::Char){
        size = 1;
        char val = *x->c_str();
        int offsetMultiplied = offset * size;
        int physicalAddress = page_table->getPhysicalAddress(pid, (var->virtual_address + offsetMultiplied));
        memcpy((uint8_t*)memory + physicalAddress, &val, size);
    }else if(type == DataType::Double){
        size = 8;
        double val = std::stod(*x, &sz);
        int offsetMultiplied = offset * size;
        int physicalAddress = page_table->getPhysicalAddress(pid, (var->virtual_address + offsetMultiplied));
        memcpy((uint8_t*)memory + physicalAddress, &val, size);
    }else if(type == DataType::Long){
        size = 8;
        long val = std::stol(*x, &sz);
        int offsetMultiplied = offset * size;
        uint32_t virtAddress = (var->virtual_address + offsetMultiplied);
        int physicalAddress = page_table->getPhysicalAddress(pid, virtAddress);
        int t = page_table->getPhysicalAddress(pid, var->virtual_address);

        memcpy((uint8_t*)memory + physicalAddress, &val, size);
    }
    else if(type == DataType::Float){
        size = 4;
        float val = std::stof(*x, &sz);
        int offsetMultiplied = offset * size;
        int physicalAddress = page_table->getPhysicalAddress(pid, (var->virtual_address + offsetMultiplied));
        memcpy((uint8_t*)memory + physicalAddress, &val, size);
    }else if(type == DataType::Int){
        size = 4;
        int val = std::stoi(*x, &sz);
        int offsetMultiplied = offset * size;
        int physicalAddress = page_table->getPhysicalAddress(pid, (var->virtual_address + offsetMultiplied));
        memcpy((uint8_t*)memory + physicalAddress, &val, size);
    }else if(type == DataType::Short){
        size = 2;
        short val = std::stoi(*x, &sz);
        int offsetMultiplied = offset * size;
        int physicalAddress = page_table->getPhysicalAddress(pid, (var->virtual_address + offsetMultiplied));
        memcpy((uint8_t*)memory + physicalAddress, &val, size);
    }
    //std::string *x = static_cast<std::string*>(value);
    //int *x = static_cast<int*>(value);
    //int val = *x;
    // int val = allNums(*x);
    
    //int val = *(int*)value;
    // std::cout << "value is: " << value << " &value is " << &value <<std::endl;
    // std::cout << "VAL is: "<< val << " &VAL is: "<< &val <<std::endl;
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    //   - free all pages associated with given process
}

void printVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table, void *memory){
    Variable *var = mmu->getVariable(pid, var_name);
    DataType type = var->type;
    uint32_t size;
    uint32_t add;
    int physicalAddress = page_table->getPhysicalAddress(pid, var->virtual_address);
    uint32_t totalVarSize = var->size;
    int count = 0;

    if(type == DataType::Char){
        size = 1;
        char value;
        for(int i = 0; i < totalVarSize; i+=size){
            int physicalAddressOffset = physicalAddress + i;
            if(count < 4){
                memcpy(&value, (uint8_t*)memory + physicalAddressOffset, size);
                std::cout << value << ", ";
            }
            count++;
        }
        if(count > 4){
            std::cout << "... ["<<(count-4)<<" items]";
        }
    }else if(type == DataType::Double){
        size = 8;
        double value;
        for(int i = 0; i < totalVarSize; i+=size){
            int physicalAddressOffset = physicalAddress + i;
            if(count < 4){
                memcpy(&value, (uint8_t*)memory + physicalAddressOffset, size);
                std::cout << value << ", ";
            }
            count++;
        }
        if(count > 4){
            std::cout << "... ["<<(count-4)<<" items]";
        }
    }else if(type == DataType::Long){
        size = 8;
        long value;
        for(int i = 0; i < totalVarSize; i+=size){
            int physicalAddressOffset = physicalAddress + i;
            if(count < 4){
                memcpy(&value, (uint8_t*)memory + physicalAddressOffset, size);
                std::cout << value << ", ";
            }
            count++;
        }
        if(count > 4){
            std::cout << "... ["<<(count)<<" items]";
        }
    }else if(type == DataType::Float){
        size = 4;
        float value;
        for(int i = 0; i < totalVarSize; i+=size){
            int physicalAddressOffset = physicalAddress + i;
            if(count < 4){
                memcpy(&value, (uint8_t*)memory + physicalAddressOffset, size);
                std::cout << value << ", ";
            }
            count++;
        }
        if(count > 4){
            std::cout << "... ["<<(count-4)<<" items]";
        }
    }else if(type == DataType::Int){
        size = 4;
        int value;
        for(int i = 0; i < totalVarSize; i+=size){
            int physicalAddressOffset = physicalAddress + i;
            if(count < 4){
                memcpy(&value, (uint8_t*)memory + physicalAddressOffset, size);
                std::cout << value << ", ";
            }
            count++;
        }
        if(count > 4){
            std::cout << "... ["<<(count-4)<<" items]";
        }
    }else if(type == DataType::Short){
        size = 2;
        short value;
        for(int i = 0; i < totalVarSize; i+=size){
            int physicalAddressOffset = physicalAddress + i;
            if(count < 4){
                memcpy(&value, (uint8_t*)memory + physicalAddressOffset, size);
                std::cout << value << ", ";
            }
            count++;
        }
        if(count > 4){
            std::cout << "... ["<<(count-4)<<" items]";
        }
    }
    std::cout << std::endl;
}


void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
    checkString: text to check if it is all numbers
    returns the string as an int if checkString is an int and -1 if it is not
*/
int allNums(std::string checkString){
    for(int i = 0; i < checkString.length(); i++){
        if(isdigit(checkString[i]) == false){
            return -1;
        }
    }
    return atoi(checkString.c_str());
}