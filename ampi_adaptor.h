#ifndef AMPI_ADAPTOR_H
#define AMPI_ADAPTOR_H

#include "ampi_base.h"
#include "mpi.h"

/**
 * @brief The AMPI_adaptor class is to allow simple standard data types to be
 * passed to remote functions.  It contains constructors for converting from
 * a standard data type, and operator overloads for converting to, as well as
 * an operator overload for converting to an AMPI_base pointer.
 */
class AMPI_adaptor : public AMPI_base
{
private:
    union AMPI_types {
        char AMPI_char;
        short AMPI_short;
        int AMPI_int;
        long AMPI_long;
        long long AMPI_longlong;
        unsigned short AMPI_ushort;
        unsigned AMPI_uint;
        unsigned long AMPI_ulong;
        unsigned long long AMPI_ulonglong;
        float AMPI_float;
        double AMPI_double;
        long double AMPI_longdouble;
    };

public:
    AMPI_adaptor()
        {AMPI_type.AMPI_longlong=0xCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC; MPI_type=0;}

    AMPI_adaptor(const char &val)
        {AMPI_type.AMPI_char = val;MPI_type=MPI_CHAR;}
    AMPI_adaptor(const short &val)
        {AMPI_type.AMPI_short = val;MPI_type=MPI_SHORT;}
    AMPI_adaptor(const int &val)
        {AMPI_type.AMPI_int = val;MPI_type=MPI_INT;}
    AMPI_adaptor(const long &val)
        {AMPI_type.AMPI_long = val;MPI_type=MPI_LONG;}
    AMPI_adaptor(const long long &val)
        {AMPI_type.AMPI_longlong = val;MPI_type=MPI_LONG_LONG;}
    AMPI_adaptor(const unsigned short &val)
        {AMPI_type.AMPI_ushort = val;MPI_type=MPI_UNSIGNED_SHORT;}
    AMPI_adaptor(const unsigned int &val)
        {AMPI_type.AMPI_uint = val;MPI_type=MPI_UNSIGNED;}
    AMPI_adaptor(const unsigned long &val)
        {AMPI_type.AMPI_ulong = val;MPI_type=MPI_UNSIGNED_LONG;}
    AMPI_adaptor(const unsigned long long &val)
        {AMPI_type.AMPI_ulonglong = val;MPI_type=MPI_UNSIGNED_LONG_LONG;}
    AMPI_adaptor(const float &val)
        {AMPI_type.AMPI_float = val;MPI_type=MPI_FLOAT;}
    AMPI_adaptor(const double &val)
        {AMPI_type.AMPI_double = val;MPI_type=MPI_DOUBLE;}
    AMPI_adaptor(const long double &val)
        {AMPI_type.AMPI_longdouble = val;MPI_type=MPI_LONG_DOUBLE;}

    operator char() {return AMPI_type.AMPI_char;}
    operator short() {return AMPI_type.AMPI_short;}
    operator int() {return AMPI_type.AMPI_int;}
    operator long() {return AMPI_type.AMPI_long;}
    operator long long() {return AMPI_type.AMPI_longlong;}
    operator unsigned short() {return AMPI_type.AMPI_ushort;}
    operator unsigned int() {return AMPI_type.AMPI_uint;}
    operator unsigned long() {return AMPI_type.AMPI_ulong;}
    operator unsigned long long() {return AMPI_type.AMPI_ulonglong;}
    operator float() {return AMPI_type.AMPI_float;}
    operator double() {return AMPI_type.AMPI_double;}
    operator long double() {return AMPI_type.AMPI_longdouble;}
    operator AMPI_base*() {return this;}

    void AMPI_debug();
private:
    MPI_Datatype MPI_type;
    AMPI_types AMPI_type;
    void* getPointer();

    int AMPI_send(int dest, int tag, MPI_Comm comm);
    int AMPI_recv(int source, int tag, MPI_Comm comm, MPI_Status *status);

    char* AMPI_typeName();


};

#endif // AMPI_ADAPTOR_H
