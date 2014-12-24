#ifndef AMPI_BASE_H
#define AMPI_BASE_H

#include <iostream>
#include "mpi.h"
#include <cstring>

class AMPI;
class AMPI_adaptor;
/**
 * @brief The AMPI_base class is the base for any classes used as parameters
 * for remote function calls
 */
class AMPI_base
{
public:
    friend class AMPI;
    friend class AMPI_adaptor;
    AMPI_base();

    virtual void AMPI_input(char *buf, int size);   //convert from array
    virtual char* AMPI_output(int *size); //convert to array

    bool AMPI_isValid(); /* is the data valid (on local machine)
                            or invalid (processed by remote function */

    bool AMPI_returnParameter(bool);    //set or get wether this class is
    bool AMPI_returnParameter();        //  modified during remote call

    virtual void AMPI_debug();
protected:
    virtual void AMPI_locked();      //functions called when data
    virtual void AMPI_unlocked();   //  valididity changes

    virtual int AMPI_send(int dest, int tag, MPI_Comm comm);
    virtual int AMPI_recv(int source, int tag, MPI_Comm comm,
                           MPI_Status *status);
    virtual char* AMPI_typeName();



private:
    bool rParam;
    bool valid;


    void Validate();
    void deValidate();
};

#endif // AMPI_BASE_H
