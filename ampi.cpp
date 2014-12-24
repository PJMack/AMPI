#include "ampi.h"
#include <cassert>
#include <thread>
#include <vector>
#include <iostream>
#include "unistd.h"
int AMPI::rank;
int AMPI::size;
const char* AMPI::AMPI_NULL_TYPE = "NULL";
std::set<AMPI_base*> AMPI::waitfor;
std::mutex AMPI::waitmtx;
std::condition_variable AMPI::waitcv;

AMPI::AMPI()
{
}

/**
 * @brief AMPI::init initialize MPI, scheduler, and listeners
 * @param argc pointer to argc from main
 * @param argv pointer to **argv from main
 */
void AMPI::init(int *argc, char ***argv){

    //Initialize MPI
    int provided;
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE,&provided);
    assert(provided>=MPI_THREAD_MULTIPLE);

    //determine size and rank number
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //print debuging information
    //std::cerr << "Rank " << rank << "is PID " << getpid() << "\n";

    if(size<=2){
        std::cerr << "ERROR: not enough ranks\n";
    }
    if(rank==AMPI_RANK_MAIN){
        std::thread(listen).detach();
        return;
    }else if(rank==AMPI_RANK_SCHED){
        std::thread(listen).detach();
        schedule();
    }else{
        listen();
    }

}

/**
 * @brief AMPI::deInit send broadcast signal to other proceses to close and
 * deinitialize MPI.  This is needed to prevent zombies.
 */
void AMPI::deInit(){
    int callerid[3];
    callerid[2]=AMPI_LISTENER_EXIT;
    for(int i = 0; i < size; i++){
        MPI_Send(callerid,
                 3,
                 MPI_INT,
                 i,
                 AMPI_TAG_LISTENER,
                 MPI_COMM_WORLD);
    }
}

/**
 * @brief AMPI::schedule coordinate all processes and keep track of loads
 */
void AMPI::schedule(){
    std::vector<int> tasks(size,0); //number of running tasks
    tasks[0]=1; //main and scheduler get low priority
    tasks[1]=1;
    MPI_Status status;
    int callNumber=AMPI_TAG_OTHER;
    //std::cerr << "Scheduler Initialized\n";
    for(;;){
        //receive request from rank
        int rrank;
        MPI_Recv(&rrank,
                 1,
                 MPI_INT,
                 MPI_ANY_SOURCE,
                 AMPI_TAG_SCHED_REQ,
                 MPI_COMM_WORLD,
                 &status);
        //rank more than size indecates done
        if(rrank>size){
            //std::cerr << "Scheduler received opening\n";
            rrank = rrank - size - 1;
            tasks[rrank]-=1;
        }else{
            //std::cerr << "Scheduler received request\n";
            //find least buissie rank
            int mrank[2];
            mrank[0]=2;
            for(int i = 0; i < size; i++){
                if(tasks[i]<=tasks[mrank[0]]){
                    mrank[0]=i;
                }
            }
            //std::cerr << "Sheduler: mrank is " << mrank[0] << "\n";
            mrank[1]=callNumber;
            callNumber+=1;
            //send rank number
            MPI_Send(mrank,
                     2,
                     MPI_INT,
                     rrank,
                     AMPI_TAG_SCHED_REPL,
                     MPI_COMM_WORLD);
            //std::cerr << "Scheduler sent reply\n";

            //increment tasks
            tasks[mrank[0]]+=1;
            //std::cerr << "S cheduler processed request\n";

        }


    }
}

/**
 * @brief AMPI::listen listen for function requests, returned data,
 * and exit requists
 */
void AMPI::listen(){
    int callerid[3];
    MPI_Status status;
    for(;;){
        MPI_Recv(callerid,
                 3,
                 MPI_INT,
                 MPI_ANY_SOURCE,
                 AMPI_TAG_LISTENER,
                 MPI_COMM_WORLD,
                 &status);
        //std::cerr << "Rank " << rank << "received call " << callerid[0] << ' ' << callerid[1] << ' ' << callerid[2] << '\n';

        //if request
        if(callerid[2]==AMPI_LISTENER_CALL){
            //  call get_call function in new thread;
            std::thread(get_call,callerid[0],callerid[1]).detach();
        }else if(callerid[2]==AMPI_LISTENER_DATA){
            std::thread(Rerecv_data,callerid[0],callerid[1]).detach();
        }else if(callerid[2]==AMPI_LISTENER_EXIT){
            MPI_Finalize();
            exit(0);
        }else{
            MPI_Finalize();
            exit(1);
        }

    }
}
/**
 * @brief AMPI::Recv_data receive data for call
 * @param p pointer to date to receiver
 * @param IDa rank of sender
 * @param IDb tag
 * @param status
 * @return the pointer to the the data on the other rank
 */
unsigned long AMPI::Recv_data(AMPI_base **p, int IDa, int IDb, MPI_Status *status){
    char typebuf[AMPI_MAX_LEN];
    MPI_Recv(typebuf,
             AMPI_MAX_LEN,
             MPI_CHAR,
             IDa,
             IDb,
             MPI_COMM_WORLD,
             status);
    //std::cerr << "Recv_data: received type\n";
    if(strcmp(typebuf,AMPI_NULL_TYPE)==0){
        *p=0;
        return 0;
    }else{
        *p=newByName(std::string(typebuf));
        //std::cerr << "Recv_data: Created type by name\n";
        MPI_Recv(typebuf,
                 1,
                 MPI_CHAR,
                 IDa,
                 IDb,
                 MPI_COMM_WORLD,
                 status);
        (*p)->rParam=typebuf[0];
        //std::cerr << "recv_data: reveived rParam\n";
        (*p)->AMPI_recv(IDa,IDb,MPI_COMM_WORLD,status);
        //std::cerr << "recv_data: reveived data\n";
        unsigned long point;
        MPI_Recv(&point,
                 1,
                 MPI_UNSIGNED_LONG,
                 IDa,
                 IDb,
                 MPI_COMM_WORLD,
                 status);
        //std::cerr << "redv_data: recived point\n";
        return point;
    }
}

/**
 * @brief AMPI::get_call receive call and parameter, call function and send
 * data.  Also, keeps scheduler informed.
 * @param tag serial number for call.
 */
void AMPI::get_call(int IDa, int IDb){
    MPI_Status status;
    char fn[AMPI_MAX_LEN];
    //std::cerr << "getcall" << IDb << "\n";
    //receive function name
    MPI_Recv(fn,
             AMPI_MAX_LEN,
             MPI_CHAR,
             IDa,
             IDb,
             MPI_COMM_WORLD,
             &status);
    //std::cerr << "Getcall " << IDb << " received function name " << fn << "\n";

    //reveive parameters

    AMPI_base *p0,*p1,*p2,*p3,*p4;

    unsigned long point[5];
    point[0] = Recv_data(&p0,IDa, IDb, &status);
    point[1] = Recv_data(&p1,IDa, IDb, &status);
    point[2] = Recv_data(&p2,IDa, IDb, &status);
    point[3] = Recv_data(&p3,IDa, IDb, &status);
    point[4] = Recv_data(&p4,IDa, IDb, &status);

    //std::cerr << "Getcall " << IDb << " received Parameters\n";
    //call function by name with parameters
    callByName(fn,p0,p1,p2,p3,p4);

    //std::cerr << "Getcall " << IDb << " called function\n";

    //send return parameters
    Resend_data(p0,IDa,IDb,point[0]);
    Resend_data(p1,IDa,IDb,point[1]);
    Resend_data(p2,IDa,IDb,point[2]);
    Resend_data(p3,IDa,IDb,point[3]);
    Resend_data(p4,IDa,IDb,point[4]);

    //std::cerr << "Getcall " << IDb << " called returned parameters\n";

    //send status to scheduler
    int rrank = rank + size + 1;
    MPI_Send(&rrank,
             1,
             MPI_INT,
             AMPI_RANK_SCHED,
             IDb,
             MPI_COMM_WORLD);

    //std::cerr << "Getcall " << IDb << " sent done signal to scheduler\n";


    //cleanup memory
    if(p0) delete p0;
    if(p1) delete p1;
    if(p2) delete p2;
    if(p3) delete p3;
    if(p4) delete p4;

}

/**
 * @brief AMPI::Resend_data send return parameters
 * @param p data to send
 * @param IDa rank of receiver
 * @param IDb tag
 * @param point place in receiver to put data
 */
void AMPI::Resend_data(AMPI_base *p, int IDa, int IDb, unsigned long point){
    if(p==0) return;
    int phonenumber[3];
    phonenumber[0]=rank;
    phonenumber[1]=IDb;
    phonenumber[2]=AMPI_LISTENER_DATA;

    MPI_Send(phonenumber,
             3,
             MPI_INT,
             IDa,
             AMPI_TAG_LISTENER,
             MPI_COMM_WORLD);

    MPI_Send(&point,
             1,
             MPI_UNSIGNED_LONG,
             IDa,
             IDb,
             MPI_COMM_WORLD);
    //std::cerr << "Resend_data " << IDb << "sending data" ;
    //p->AMPI_debug();
    p->AMPI_send(IDa,IDb, MPI_COMM_WORLD);
    //std::cerr << "Resend_data " << IDb << "sent data\n";

}

/**
 * @brief AMPI::Rerecv_data receive returned data
 * @param IDa rank of sender
 * @param IDb tag
 */
void AMPI::Rerecv_data(int IDa, int IDb){
    //std::cerr << "Rercv_data " << getpid() << "receiving point\n";
    std::unique_lock<std::mutex> waitlck(waitmtx);

    MPI_Status status;
    unsigned long point;

    MPI_Recv(&point,
             1,
             MPI_UNSIGNED_LONG,
             IDa,
             IDb,
             MPI_COMM_WORLD,
             &status);
    //std::cerr << "Rercv_data " << getpid() << "received point\n";

    AMPI_base *p = reinterpret_cast<AMPI_base*>(point);
    p->AMPI_recv(IDa,IDb, MPI_COMM_WORLD,&status);
    //std::cerr << "Rercv_data " << getpid() << "received data\n";
    p->AMPI_debug();
    p->Validate();
    //std::cerr << "Rercv_data " << getpid() << "received validated data\n";
    waitfor.erase(p);
    //std::cerr << "Rercv_data " << getpid() << "removed data from list\n";
    waitcv.notify_all();
    //std::cerr << "Rercv_data " << getpid() << "notified all\n";

}

/**
 * @brief AMPI::remote_call call function remotely
 * @param fn function name
 * @param p0
 * @param p1
 * @param p2
 * @param p3
 * @param p4
 */
void AMPI::remote_call(std::string fn,
                       AMPI_base* p0,
                       AMPI_base* p1,
                       AMPI_base* p2,
                       AMPI_base* p3,
                       AMPI_base* p4){
    if(p0!=0 && p0->rParam){
        p0->deValidate();
        waitfor.insert(p0);
    }
    if(p1!=0 && p1->rParam){
        p1->deValidate();
        waitfor.insert(p1);
    }
    if(p2!=0 && p2->rParam){
        p2->deValidate();
        waitfor.insert(p2);
    }
    if(p3!=0 && p3->rParam){
        p3->deValidate();
        waitfor.insert(p3);
    }
    if(p4!=0 && p4->rParam){
        p4->deValidate();
        waitfor.insert(p4);
    }

    MPI_Status status;
    int phonenumber[3];
    //send call to schedule
    MPI_Send(&rank,
             1,
             MPI_INT,
             AMPI_RANK_SCHED,
             AMPI_TAG_SCHED_REQ,
             MPI_COMM_WORLD);
    //schedule returns rank# and callNumber?
    MPI_Recv(phonenumber,
             2,
             MPI_INT,
             AMPI_RANK_SCHED,
             AMPI_TAG_SCHED_REPL,
             MPI_COMM_WORLD,
             &status);
    //request function to rank# (with callNumber?)
    phonenumber[2]=AMPI_LISTENER_CALL;
    int sendto = phonenumber[0];
    phonenumber[0]=rank;
    MPI_Send(phonenumber,
             3,
             MPI_INT,
             sendto,
             AMPI_TAG_LISTENER,
             MPI_COMM_WORLD);
    //send stuff to rank#
    MPI_Send(fn.c_str(),
             fn.length()+1,
             MPI_CHAR,
             sendto,
             phonenumber[1],
             MPI_COMM_WORLD);

    Send_data(p0,sendto,phonenumber[1]);
    Send_data(p1,sendto,phonenumber[1]);
    Send_data(p2,sendto,phonenumber[1]);
    Send_data(p3,sendto,phonenumber[1]);
    Send_data(p4,sendto,phonenumber[1]);


    return;
}

/**
 * @brief AMPI::Send_data send parameters for call
 * @param p parameter to send
 * @param IDa rank to send to
 * @param IDb tag
 */
void AMPI::Send_data(AMPI_base *p, int IDa, int IDb){
    if(p){
        //std::cerr << "Send_data: typename\n";
        MPI_Send(p->AMPI_typeName(),
                 strlen(p->AMPI_typeName())+1,
                 MPI_CHAR,
                 IDa,
                 IDb,
                 MPI_COMM_WORLD);
        //std::cerr << "Send_data: rParam\n";

        char retpar = p->rParam;
        MPI_Send(&retpar,
                 1,
                 MPI_CHAR,
                 IDa,
                 IDb,
                 MPI_COMM_WORLD);

        p->AMPI_send(IDa,IDb,MPI_COMM_WORLD);
        //std::cerr << "Send_data: pointer\n";
        unsigned long point = reinterpret_cast<unsigned long>(p);
        MPI_Send(&point,
                 1,
                 MPI_UNSIGNED_LONG,
                 IDa,
                 IDb,
                 MPI_COMM_WORLD);
    }else{
        MPI_Send(AMPI_NULL_TYPE,
                 strlen(AMPI_NULL_TYPE)+1,
                 MPI_CHAR,
                 IDa,
                 IDb,
                 MPI_COMM_WORLD);
    }
}

/**
 * @brief AMPI::Ranks return the number of ranks
 * @return number of ranks
 */
int AMPI::Ranks(){
    return size;
}

/**
 * @brief AMPI::wait_for hold execution until data is returned
 * @param p the pointer to the data that is being waited for
 */
void AMPI::wait_for(AMPI_base *p){
    std::unique_lock<std::mutex> waitlck(waitmtx);
    while(waitfor.count(p)){
        //std::cerr << "Waiting for\n";
        waitcv.wait(waitlck);
    }
}

