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
        // TODO: implement this!

        if(commandSplit.at(0) == "create"){ //create <text_size> <data_size>
            std::cout << "CREATE" << std::endl;
            //assign a process id
            //allocate some amount of startup memory for the process
                //text/code: size of binary executable - user specified number (2048-16384 bytes)
                //Data/Globals: size of global variables - user specified number (0 - 1024 bytes)
                //Stack: constant (65536 bytes)
            //prints the PID
        }else if(commandSplit.at(0) == "allocate"){ //allocate <PID> <var_name> <data_type> <number_of_elements>
            //Allocated memory on the heap (how mcuch depends on the data type and the number of elements)
                //N chars (N bytes)
                //N shorts (N * 2 bytes)
                //N ints/floats (N * 4 bytes)
                //N longs/doubles (N * 8 bytes)
            //print the virtual memory address
        }else if(commandSplit.at(0) == "set"){ //set <PID> <var_name> <offset> <value_0> <value_2> ... <value_N>
            //sote integer, float, or character values in memeory
            //Set the value for the variable <var_name> starting at <offset>
            //NOTE: multiple contiguous values can be set with one command
        }else if(commandSplit.at(0) == "print"){ //print <object>
            //if <object> is "mmu", print the MMU memory table
            //if <object> is "page", print the page table (do not need to print anything for free frames)
            //if <object> is "process", print a list of PID's for processes that are still running
            //if <object> is a "<PID>":<var_name>", print the value of the variable for that process"
                //If variable has more than 4 elements, just print the first 4 followed by "... [N items]" (where N is the number of elements)
        }else if(commandSplit.at(0) == "free"){ //free <PID> <var_name>
            //Deallocate memory on the heap that is associated with <var_name>
                //N chars (N bytes)
                //N shorts (N * 2 bytes)
                //N ints/floats (N * 4 bytes)
                //N longs/doubles (N * 8 bytes)
                //Can multiple contiguous vales be deallocated with one command?
        }else if(commandSplit.at(0) == "terminate"){ //terminate <PID>
            //Kill the specified process
            //Free all memory associated with this process
            //Deallocate all memory associated with the process
        }else{ //error
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << std::endl;
        std::cout << "> ";
        std::getline (std::cin, command);
        splitString(command, ' ', commandSplit);
    }

    // Cean up
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
    // TODO: implement this!
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
    // TODO: implement this!
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
            freeSpace->size = (freeSpace->size - newVarSize);
            freeSpace->virtual_address = (freeSpace->virtual_address + newVarSize);
            mmu->addVariableToProcess(pid, var_name, type, newVarSize, newVarAddress);
            //update page table
            //page_table->addEntry(pid, page_table->getPageNumber(newVarAddress));

            //print virtual address
            std::cout << newVarAddress;
        }else {
            // throw error -> out of space! No allocation happens!
        }
    }



    //addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)

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
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
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