#ifndef AUDIO_H
#define AUDIO_H

#include "ampi_base.h"
#include <vector>
#include <mutex>
#include <condition_variable>

class simulant : public AMPI_base
{
public:
    simulant();
    simulant(const int s);
    simulant(const simulant&);

    int AMPI_send(int dest, int tag, MPI_Comm comm);
    int AMPI_recv(int source, int tag, MPI_Comm comm, MPI_Status *status);
    void readfile(std::string);
    void writefile(std::string);
    float& operator[](int i){return data[i];}
    long Size(){return size;}
    char* AMPI_typeName();
    void resize(int s);
    static void simsleep(int time);

private:
    float *data;
    long size;
    static int threadcount;
    static std::mutex mtx;
    static std::condition_variable cv;
};



#endif // AUDIO_H
