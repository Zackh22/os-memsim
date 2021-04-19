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
