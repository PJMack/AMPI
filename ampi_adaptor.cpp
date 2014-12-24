#include "ampi_adaptor.h"

int AMPI_adaptor::AMPI_send(int dest, int tag, MPI_Comm comm){
    MPI_Send(&MPI_type,
             1,
             MPI_INT,
             dest,
             tag,
             comm);
    MPI_Send(getPointer(),
             1,
             MPI_type,
             dest,
             tag,
             comm);
    return 0;
}

int AMPI_adaptor::AMPI_recv(int source, int tag, MPI_Comm comm,
                            MPI_Status * status){
    MPI_Recv(&MPI_type,
             1,
             MPI_INT,
             source,
             tag,
             comm,
             status);
    MPI_Recv(getPointer(),
             1,
             MPI_type,
             source,
             tag,
             comm,
             status);
    return 0;
}

char* AMPI_adaptor::AMPI_typeName(){
    return "AMPI_adaptor";
}

void* AMPI_adaptor::getPointer(){
    void *buf;
    switch(MPI_type){
    case MPI_CHAR:
        buf=&(AMPI_type.AMPI_char);
        break;
    case MPI_SHORT:
        buf=&(AMPI_type.AMPI_short);
        break;
    case MPI_INT:
        buf=&(AMPI_type.AMPI_int);
        break;
    case MPI_LONG:
        buf=&(AMPI_type.AMPI_long);
        break;
    case MPI_LONG_LONG:
        buf=&(AMPI_type.AMPI_longlong);
        break;
    case MPI_UNSIGNED_SHORT:
        buf=&(AMPI_type.AMPI_ushort);
        break;
    case MPI_UNSIGNED:
        buf=&(AMPI_type.AMPI_uint);
        break;
    case MPI_UNSIGNED_LONG:
        buf=&(AMPI_type.AMPI_ulong);
        break;
    case MPI_UNSIGNED_LONG_LONG:
        buf=&(AMPI_type.AMPI_ulonglong);
        break;
    case MPI_FLOAT:
        buf=&(AMPI_type.AMPI_float);
        break;
    case MPI_DOUBLE:
        buf=&(AMPI_type.AMPI_double);
        break;
    case MPI_LONG_DOUBLE:
        buf=&(AMPI_type.AMPI_longdouble);
        break;
    default:
        buf=NULL;
    }
    return buf;
}

void AMPI_adaptor::AMPI_debug(){
    std::cerr << AMPI_typeName() <<
                 ": ";
    std::cerr << rParam << valid;
    switch(MPI_type){
    case MPI_CHAR:
        std::cerr << " char " << AMPI_type.AMPI_char << "\n";
        break;
    case MPI_SHORT:
        std::cerr << " short " << AMPI_type.AMPI_short << "\n";
        break;
    case MPI_INT:
        std::cerr << " int " << AMPI_type.AMPI_int << "\n";
        break;
    case MPI_LONG:
        std::cerr << " long " << AMPI_type.AMPI_long << "\n";
        break;
    case MPI_LONG_LONG:
        std::cerr << " longlong " << AMPI_type.AMPI_longlong << "\n";
        break;
    case MPI_UNSIGNED_SHORT:
        std::cerr << " ushort " << AMPI_type.AMPI_ushort << "\n";
        break;
    case MPI_UNSIGNED:
        std::cerr << " unsigned " << AMPI_type.AMPI_uint << "\n";
        break;
    case MPI_UNSIGNED_LONG:
        std::cerr << " ulong " << AMPI_type.AMPI_ulong   << "\n";
        break;
    case MPI_UNSIGNED_LONG_LONG:
        std::cerr << " ulonglong " << AMPI_type.AMPI_ulonglong << "\n";
        break;
    case MPI_FLOAT:
        std::cerr << " float " << AMPI_type.AMPI_float << "\n";
        break;
    case MPI_DOUBLE:
        std::cerr << " double " << AMPI_type.AMPI_double << "\n";
        break;
    case MPI_LONG_DOUBLE:
        std::cerr << " longdouble " << AMPI_type.AMPI_longdouble << "\n";
        break;
    default:
        std::cerr << " null\n";
    }
}
