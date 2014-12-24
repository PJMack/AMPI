#ifndef AMPI_H
#define AMPI_H

#include "ampi_adaptor.h"
#include "ampi_base.h"
#include <cstdio>
#include <set>
#include <mutex>
#include <condition_variable>

#include <string>
#include <cstdarg>
#include <cstdlib>
#include "mpi.h"
#include <thread>
#include <cstring>


/**
 * @brief The AMPI class contains static functions for initilizing and making
 * remote calls
 */
class AMPI
{
public:
    AMPI();

    static void init(int *argc, char ***argv);     //initilize MPI
    static void deInit();   //deinitialize MPI

    //remotely call function with parameters
    static void remote_call(std::string fn,
                            AMPI_base* p0 = 0,
                            AMPI_base* p1 = 0,
                            AMPI_base* p2 = 0,
                            AMPI_base* p3 = 0,
                            AMPI_base* p4 = 0);

    static void callByName(std::string fn,
                           AMPI_base* p0 = 0,
                           AMPI_base* p1 = 0,
                           AMPI_base* p2 = 0,
                           AMPI_base* p3 = 0,
                           AMPI_base* p4 = 0);

    static AMPI_base* newByName(std::string type);

    //wait for class to be valid
    static void wait_for(AMPI_base*);

    //number of Ranks
    static int Ranks();

private:
    static void schedule();
    static void listen();

    static void get_call(int IDa, int IDb);
    static unsigned long Recv_data(AMPI_base **p, int IDa, int IDb,
                                   MPI_Status *status);
    static void Send_data(AMPI_base *p, int IDa, int IDb);
    static void Resend_data(AMPI_base *p, int IDa, int IDb,
                            unsigned long point);
    static void Rerecv_data(int IDa, int IDb);
    static int rank;
    static int size;

    static std::set<AMPI_base*> waitfor;
    static std::mutex waitmtx;
    static std::condition_variable waitcv;


public:
    //tags
    static const int AMPI_TAG_SCHED_REQ = 0;
    static const int AMPI_TAG_SIZE = 1;
    static const int AMPI_TAG_SCHED_REPL = 2;
    static const int AMPI_TAG_OTHER = 5;
    static const int AMPI_TAG_LISTENER = 3;

    //ranks
    static const int AMPI_RANK_MAIN = 0;
    static const int AMPI_RANK_SCHED = 1;

    //listerner packet types
    static const int AMPI_LISTENER_CALL = 0;
    static const int AMPI_LISTENER_DATA = 1;
    static const int AMPI_LISTENER_EXIT = 2;

    //other
    static const int AMPI_MAX_LEN = 0xFF;
    static const char *AMPI_NULL_TYPE;
};

#endif // AMPI_H
