
#include "postgresql/12/server/postgres.h"
#include "postgresql/12/server/fmgr.h"
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define IPADDRESS "127.0.0.1"

#define DECODE_ON 

PG_MODULE_MAGIC;

/**
 * Represents the Type textenc
 **/
typedef struct
{
    int32 length;
    char content[FLEXIBLE_ARRAY_MEMBER];
} Textenc;

/**
 * Connects to the Server at localhost:3001
 **/
static int connect_to_server(int *connection, struct sockaddr_in *address)
{
    unsigned short int port = 3001;

    *connection = socket(AF_INET, SOCK_STREAM, 0);
    address->sin_family = AF_INET;
    inet_pton(AF_INET, IPADDRESS, &address->sin_addr.s_addr);
    address->sin_port = htons(port);

    return connect(*connection, (struct sockaddr *)address, sizeof(*address));
}

PG_FUNCTION_INFO_V1(textenc_in);

/**
 * Input function for the datatype textenc
 **/
Datum textenc_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    int32 length = strlen(str);
    Textenc *result;

    /**
     * Establish Connection
     **/
    int connection, res;
    struct sockaddr_in address;

    res = connect_to_server(&connection, &address);

    if (res == -1)
    {
        ereport(ERROR, (errcode(ERRCODE_CANNOT_CONNECT_NOW), errmsg("Can not connect to the external server")));
    }
    else
    {
        char *buffer, *header, *ptr;
        int internal_id;
        header = (char *)palloc0(100);
        buffer = (char *)palloc0(length);

        sprintf(header, "Operation: 1\nContent-Length: %i\n\n", length);
        write(connection, header, strlen(header));
        write(connection, str, length);

        res = 0;
        while (res < length)
        {
            res += read(connection, &buffer[res], 1);
        }
        close(connection);

        /**
         * write to database 
         **/
        result = (Textenc *)palloc0(VARHDRSZ + length);
        SET_VARSIZE(result, VARHDRSZ + length);
        memcpy(result->content, buffer, length);

        pfree((void *)buffer);
        pfree((void *)header);

        PG_RETURN_POINTER(result);
    }
}

PG_FUNCTION_INFO_V1(textenc_out);

/**
 * Output function for datatype textenc
 **/
Datum textenc_out(PG_FUNCTION_ARGS)
{

    Textenc *textenc = (Textenc *)PG_GETARG_POINTER(0);
    int length = VARSIZE(textenc) - VARHDRSZ;


    #ifndef DECODE_ON
    /**
     * WITHOT ENCODING 
     **/
    PG_RETURN_CSTRING(textenc->content);

    #else
    /**
     * WITH ENCODING
     **/
    /**
     * Establish Connection
     **/
    int connection, res;
    struct sockaddr_in address;

    char *header, *buffer;

    res = connect_to_server(&connection, &address);

    if (res == -1)
    {
        ereport(ERROR, (errcode(ERRCODE_CANNOT_CONNECT_NOW), errmsg("Can not connect to the external server")));
    }
    else
    {
        header = (char *)palloc0(100);
        buffer = (char *)palloc0(length + 1);

        sprintf(header, "Operation: 2\nContent-Length: %i\n\n", length);
        write(connection, header, strlen(header));
        write(connection, textenc->content, length);

        res = 0;
        while (res < length)
        {
            res += read(connection, &buffer[res], 1);
        }
        buffer[res] = 0;
        close(connection);

        pfree((void *)header);

        PG_RETURN_CSTRING(buffer);

        
    }
    #endif
}

    
/**
 * Support function for b-tree Index. This compares two textenc values.
 **/
static int textenc_compare_internal(Textenc *a, Textenc *b)
{
    /**
     * Establish Connection
     **/
    int connection, res;
    struct sockaddr_in address;
    unsigned short int port = 3001;

    char *header, *buffer;
    int a_length, b_length, buffer_length;

    res = connect_to_server(&connection, &address);

    if (res == -1)
    {
        ereport(ERROR, (errcode(ERRCODE_CANNOT_CONNECT_NOW), errmsg("Can not connect to the external server")));
    }
    else
    {
        a_length = VARSIZE(a) - VARHDRSZ;
        b_length = VARSIZE(b) - VARHDRSZ;
        buffer_length = a_length + b_length;

        header = (char *)palloc0(150);
        buffer = (char *)palloc0(buffer_length + 1);

        sprintf(header, "Content-Length: %i\nOperation: 3\nDivider: %i\n\n", buffer_length, a_length);

        memcpy(buffer, a->content, a_length);
        memcpy(&buffer[a_length], b->content, b_length);

        write(connection, header, strlen(header));
        write(connection, buffer, buffer_length);

        memset(buffer, 0, buffer_length);

        res = 0;
        while (res < 3)
        {
            res += read(connection, &buffer[res], 1);
        }
        buffer[res] = 0;
        close(connection);
        return atoi(buffer);
    }
}

/**
 * implemented strategies for the b-tree extension
 **/

PG_FUNCTION_INFO_V1(textenc_lt);

Datum
    textenc_lt(PG_FUNCTION_ARGS)
{
    Textenc *a = (Textenc *)PG_GETARG_POINTER(0);
    Textenc *b = (Textenc *)PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(textenc_compare_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(textenc_lt_eq);

Datum textenc_lt_eq(PG_FUNCTION_ARGS)
{
    Textenc *a = (Textenc *)PG_GETARG_POINTER(0);
    Textenc *b = (Textenc *)PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(textenc_compare_internal(a, b) <= 0);
}
PG_FUNCTION_INFO_V1(textenc_gt);

Datum textenc_gt(PG_FUNCTION_ARGS)
{
    Textenc *a = (Textenc *)PG_GETARG_POINTER(0);
    Textenc *b = (Textenc *)PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(textenc_compare_internal(a, b) > 0);
}
PG_FUNCTION_INFO_V1(textenc_gt_eq);
Datum textenc_gt_eq(PG_FUNCTION_ARGS)
{
    Textenc *a = (Textenc *)PG_GETARG_POINTER(0);
    Textenc *b = (Textenc *)PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(textenc_compare_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(textenc_eq);
Datum textenc_eq(PG_FUNCTION_ARGS)
{
    Textenc *a = (Textenc *)PG_GETARG_POINTER(0);
    Textenc *b = (Textenc *)PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(textenc_compare_internal(a, b) == 0);
}

PG_FUNCTION_INFO_V1(textenc_compare);
Datum textenc_compare(PG_FUNCTION_ARGS)
{
    Textenc *a = (Textenc *)PG_GETARG_POINTER(0);
    Textenc *b = (Textenc *)PG_GETARG_POINTER(1);

    PG_RETURN_INT32(textenc_compare_internal(a, b));
}
