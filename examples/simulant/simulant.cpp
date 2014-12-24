#include "simulant.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <ctime>

simulant::simulant()
{
    size=0;
    data=0;
}

simulant::simulant(const int s){
    size=s;
    data=new float[size];
    for(int i = 0; i < size; i++){
        data[i]=0;
    }
}

void simulant::resize(int s){
    delete data;
    size=s;
    data=new float[size];
    for(int i = 0; i < size; i++){
        data[i]=0;
    }
}

simulant::simulant(const simulant &a){
    size=a.size;
    data=new float[size];
    for(int i = 0; i < size; i++){
        data[i]=a.data[i];
    }
}

char* simulant::AMPI_typeName(){
    return "simulant";
}

void simulant::readfile(std::string filename){
    if(data){
        delete data;
    }
    std::ifstream file (filename.c_str(),std::ios::binary);
    if(file.is_open()){
        size=file.tellg()/sizeof(float);
        data = new float[size];
        file.seekg(0,std::ios_base::beg);
        file.read(reinterpret_cast<char*>(data),size*sizeof(float));
        file.close();
    }
}

void simulant::writefile(std::string filename){
    if(data){
        std::ofstream file(filename.c_str(),std::ios::binary);
        if(file.is_open()){
            file.write(reinterpret_cast<char*>(data),size*sizeof(float));
            file.close();
        }
    }
}

int simulant::AMPI_send(int dest, int tag, MPI_Comm comm){
    //simsleep(1);
    MPI_Send(&size,
             1,
             MPI_LONG,
             dest,
             tag,
             comm);
    MPI_Send(data,
             size,
             MPI_FLOAT,
             dest,
             tag,
             comm);
    return 0;
}

int simulant::AMPI_recv(int source, int tag, MPI_Comm comm, MPI_Status *status){
    MPI_Recv(&size,
             1,
             MPI_LONG,
             source,
             tag,
             comm,
             status);
    delete data;
    data = new float[size];
    MPI_Recv(data,
             size,
             MPI_FLOAT,
             source,
             tag,
             comm,
             status);
    //simsleep(1);
    return 0;
}
std::mutex simulant::mtx;
std::condition_variable simulant::cv;
int simulant::threadcount = 4;
void simulant::simsleep(int stime){
  int i=0;
  time_t t = time(0);
  while(difftime(time(0),t)<(float)stime){i++;}
}
