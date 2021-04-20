#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    } 

    Variable *var = new Variable();
    var->name = var_name;
    var->type = type;
    var->virtual_address = address;
    var->size = size;
    if (proc != NULL)
    {
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        uint32_t pid = _processes[i]->pid;
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            //print all variables (excluding <FREE_SPACE> entries)
            std::string varName = _processes[i]->variables[j]->name;
            uint32_t virtualAddress = _processes[i]->variables[j]->virtual_address;
            uint32_t varSize = _processes[i]->variables[j]->size;
            if(varName != "<FREE_SPACE>"){
                printf(" %4u | %-13s |   0x%08X | %10u \n", pid, varName.c_str(), virtualAddress, varSize);     
            }
        }
    }
}

Variable* Mmu::getVariable(uint32_t pid, std::string var_name){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                if(_processes[i]->variables[j]->name == var_name){
                    return _processes[i]->variables[j];
                }
            }
        }
    }
    return NULL;
}

bool Mmu::processExists(uint32_t pid){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            return true;
        }
    }
    return false;
}

void Mmu::printProcesses(){
    for(int i = 0; i < _processes.size(); i++){
        std::cout<<_processes[i]->pid<<std::endl;
    }
}

bool Mmu::spaceLeft(int size){
    uint32_t mem_size = 67108864;
    uint32_t memUsed = size;

    for(int i = 0; i < _processes.size(); i++){
        for(int j = 0; j < _processes[i]->variables.size(); j++){
            std::string varName = _processes[i]->variables[j]->name;
            if(varName != "<FREE_SPACE>"){
                memUsed = memUsed + _processes[i]->variables[j]->size;
            }
        }
    }

    if(memUsed <= mem_size){
        return true;
    }else {
        return false;
    }
}

void Mmu::mergeFreeSpace(uint32_t address, uint32_t size, uint32_t pid){
    // if newly created freespace has freespace directly before or after then merge them

    uint32_t beforeAddress = -1;
    uint32_t afterAddress = address;
    bool otherFreeSpace = false;

    //find freespace before 
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                std::string varName = _processes[i]->variables[j]->name;
                if(varName == "<FREE_SPACE>"){
                    if(_processes[i]->variables[j]->virtual_address + _processes[i]->variables[j]->size == address){ //freespace directly before new freespace
                        _processes[i]->variables[j]->size += size;
                        otherFreeSpace = true;
                    }

                    if(_processes[i]->variables[j]->virtual_address == (address + size)){ //freespace directly after new freespace
                        _processes[i]->variables[j]->size += size;
                        otherFreeSpace = true;
                    }
                }
            }
        }
    }

    //remove new free space from var list if merged
    if(otherFreeSpace){
        for(int i = 0; i < _processes.size(); i++){
            if(_processes[i]->pid == pid){
                for(int j = 0; j < _processes[i]->variables.size(); j++){
                    uint32_t varAddress = _processes[i]->variables[j]->virtual_address;
                    if(varAddress == address){
                        _processes[i]->variables.erase((_processes[i]->variables.begin() + j));
                    }
                }
            }
        }
    }
}

std::vector<Variable*> Mmu::getAllVars(uint32_t pid){
    std::vector<Variable*> toReturn;
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            return _processes[i]->variables;
        }
    }
    return toReturn;
}

int Mmu::getRemainingSpaceOnPage(uint32_t pid, uint32_t virtual_address, int page_size, int page_num){
    int remainingPageSpace = page_size;
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                std::string varName = _processes[i]->variables[j]->name;
                uint32_t varAddress = _processes[i]->variables[j]->virtual_address;
                int varPageNum = varAddress / page_size;
                int varSize = _processes[i]->variables[j]->size;
                if(varName != "<FREE_SPACE>" && (varPageNum == page_num)){
                    remainingPageSpace = remainingPageSpace - varSize;
                }
            }
        }
    }

    if(remainingPageSpace <= 0){
        return 0;
    }else {
        return remainingPageSpace;
    }
}

void Mmu::removeProcess(uint32_t pid){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            _processes.erase((_processes.begin() + i));
        }
    }
}