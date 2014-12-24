#include <iostream>
#include "ampi.h"
#include "unistd.h"
#include "simulant.h"
#include <cmath>
#include <ctime>
//#define usethreads


void simfunc(simulant a, simulant *b){
    simulant::simsleep(10);
}

/**
 * @brief AMPI::callByName this function is to be writen by the user
 * so call a function using the name of the function and up to five
 * AMPI_base pointers as parameters that could be cast accordingly
 * @param fn The function name
 * @param p0 Function Parameter
 * @param p1 Function Parameter
 * @param p2 Function Parameter
 * @param p3 Function Parameter
 * @param p4 Function Parameter
 */
void AMPI::callByName(std::string fn,
                             AMPI_base *p0,
                             AMPI_base *p1,
                             AMPI_base *p2,
                             AMPI_base *p3,
                             AMPI_base *p4){
    if(fn==std::string("simfunc")){
        simulant *a = (reinterpret_cast<simulant*>(p0));
        simulant *b = (reinterpret_cast<simulant*>(p1));
        simfunc(*a,b);
        return;
    }
    std::cerr << "ERROR: function name " << fn << " not recognized\n";
}

/**
 * @brief AMPI::newByName this function is to be writen
 * (cut and pasted) by the user to create a new AMPI_base
 * by the name of it
 * @param type this is the string for the name
 * @return The pointer to the new variable
 */
AMPI_base* AMPI::newByName(std::string type){
    if(type==std::string("AMPI_adaptor")){
        AMPI_adaptor* n = new AMPI_adaptor();
        return static_cast<AMPI_base*>(n);
    }else if(type==std::string("simulant")){
        simulant *n = new simulant();
        return static_cast<AMPI_base*>(n);
    }
    return 0;
}

int main(int argc, char **argv)
{
    time_t t = time(0);
#ifndef usethreads
    AMPI::init(&argc,&argv);
#endif
    int its = std::stoi(argv[1]);

    simulant input(10e6);
    simulant output[its];
#ifdef usethreads
    std::thread th[its];
#endif
    for(int i = 0; i < its; i++){
        output[i].resize(1e3);
        output[i].AMPI_returnParameter(true);
    }
    for(int i = 0; i < its; i++){
#ifdef usethreads
        th[i]=std::thread(simfunc,input,&(output[i]));
#else
        AMPI::remote_call("simfunc",
                          &input,
                          &(output[i]));
#endif
    }
    for(int i = 0; i < its; i++){
#ifdef usethreads
        th[i].join();
        th[i] = std::thread(simfunc,input,&(output[i]));
#else
        AMPI::wait_for(&output[i]);
        AMPI::remote_call("simfunc",
                          &input,
                          &(output[i]));
#endif
    }
    for(int i = 0; i < its; i++){
#ifdef usethreads
        th[i].join();
#else
        AMPI::wait_for(&(output[i]));
#endif
    }


#ifndef usethreads
    AMPI::deInit();
#endif
    std::cout << "Total Time: " << difftime(time(0),t) << " seconds\n";

    return 0;
}

