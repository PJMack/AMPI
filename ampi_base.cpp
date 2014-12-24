

#include "ampi_base.h"
#include <sstream>
#include <ostream>
#include "ampi.h"
#include "mpi.h"

/**
 * @brief AMPI_base::AMPI_base Initilize the class
 */
AMPI_base::AMPI_base(){
    rParam=false;
    valid=true;
}

/**
 * @brief AMPI_base::AMPI_typeName This function is to be used by AMPI for
 * transmitting, receiving, and handling type names.  It serves as a unique
 * identify for the class, and is required to be rewriten for inherited classes
 * @return The name of the class
 */
char* AMPI_base::AMPI_typeName(){
    return "AMPI_base";
}

/**
 * @brief AMPI_base::AMPI_locked this optional function is called when the class
 * has been transmitted and it will be modified by another machine.
 */
void AMPI_base::AMPI_locked() {
    return;
}
/**
 * @brief AMPI_base::AMPI_unlocked this optional function is called when the
 * class has been returned from a remote function call.
 */
void AMPI_base::AMPI_unlocked() {
    return;
}
/**
 * @brief AMPI_base::AMPI_send This is called by AMPI to send data from
 * the class to new copy on another machine.  This can be left alone if using
 * AMPI_base::AMPI_input, however it is left virtual so advaced users may use
 * this function and MPI_Send to send the data more efficietly
 * @param dest The destination to be sent to
 * @param tag The MPI tag to send with
 * @param comm the MPI communicant
 * @return
 */
int AMPI_base::AMPI_send(int dest, int tag, MPI_Comm comm){
    char *buf;
    int size;
    buf = AMPI_output(&size);
    MPI_Send(&size,1,MPI_INT,dest,AMPI::AMPI_TAG_SIZE, comm);
    MPI_Send(buf,size,MPI_CHAR,dest,tag,comm);
    return 0;
}

/**
 * @brief AMPI_base::AMPI_recv This is called by AMPI to receive new data from
 * another class on another machine.  this can be left alone if using
 * AMPI_base::AMPI_input, however it is left virtual so advanced users my use
 * this function and MPI_Recv to recive the data more efficently
 * @param source The source of the data
 * @param tag The MPI tag that to receive from
 * @param comm The MPI communicant
 * @param status Pointer to an external status variable
 * @return
 */
int AMPI_base::AMPI_recv(int source, int tag, MPI_Comm comm,
                         MPI_Status *status){
    int size;
    MPI_Recv(&size,1,MPI_INT,source,AMPI::AMPI_TAG_SIZE,comm,status);
    char *buf = new char[size];
    MPI_Recv(buf,size,MPI_CHAR,source,tag,comm,status);
    AMPI_input(buf,size);
    return 0;
}

/**
 * @brief AMPI_base::AMPI_input This function is called by the default
 * AMPI_base::AMPI_recv function to convert the character array received into
 * the inherited class's data format.  Rewriting this function in the inherited
 * class is required unless using AMPI_base::AMPI_recv, however it should be
 * nearly identical to a function to read the class froma file.
 * @param buf The character array to read from
 * @param size the size of the array
 */
void AMPI_base::AMPI_input(char *buf, int size){
    return;
}

/**
 * @brief AMPI_base::AMPI_output This function is called by the default
 * AMPI_base::AMPI_send function to convert the inherted class's data format to
 * a character array.  Rewirting this function in the inherited class is
 * required unless using AMPI_base::AMPI_recv, however it should be nearly
 * identicle to a function to write the class to a file.
 * @param size pointer to and integer to store the size of the character array
 * @return the character array
 */
char* AMPI_base::AMPI_output(int *size){
    return "NULL";
}

/**
 * @brief AMPI_base::AMPI_returnParameter setting rP to true indecates to AMPI
 * that the class will be modified during a remote call and need to be sent back
 * @param rP
 * @return
 */
bool AMPI_base::AMPI_returnParameter(bool rP){
    rParam=rP;
    return AMPI_returnParameter();
}
/**
 * @brief AMPI_base::AMPI_returnParameter this indecates weather or not the
 * class will be returned after a remote call
 * @return true if the class will be returned
 */
bool AMPI_base::AMPI_returnParameter(){
    return rParam;
}

void AMPI_base::Validate(){
    valid=false;
    AMPI_locked();
}

void AMPI_base::deValidate(){
    valid=true;
    AMPI_unlocked();
}

/**
 * @brief AMPI_base::AMPI_debug this function is used for debuging AMPI
 * it is not need for applications.
 */
void AMPI_base::AMPI_debug(){
    std::cerr << AMPI_typeName()
              << ": "
              << rParam << valid << "\n";
}
