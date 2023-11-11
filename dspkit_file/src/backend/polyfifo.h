#ifndef POLYFIFO_H_
#define POLYFIFO_H_

#include <stddef.h>

/** @file Basic polymorphic FIFO buffer implementation using the C preprocessor
 * to give crude polymorphism. This allows for creating buffers of arbitrary
 * types and lengths using the same "functions". Note that all polyfifo "functions"
 * are really just macro directives.
 * 
 * Simple example:

 #include <stdio.h>
 #include "polyfifo.h"
 int main(void){
    //Set up buffers
    //One slot reserved, so up to 9 elements can be stored in intfifo and 19
    //elements in floatfifo.
    int intbuf[10]; 
    float floatbuf[20];
    POLYFIFO_DECLARE(int);
    POLYFIFO_DECLARE(float);
    POLYFIFO_DEFINE(int, intfifo, 10, intbuf);
    POLYFIFO_DEFINE(float, floatfifo, 20, floatbuf);

    //Add data of associated types to buffers
    POLYFIFO_WRITE(intfifo, 5);
    POLYFIFO_WRITE(floatfifo, 3.14);

    //Read one element from each buffer
    int result_int;
    float result_float;
    POLYFIFO_READ(intfifo, &result_int);
    POLYFIFO_READ(floatfifo, &result_float);

    //Print results
    printf("Got integer %d, float %g\n", result_int, result_float);
}

 * Prints:
 * 'Got integer 5, float 3.14' */


/** @brief Define to enable a function that prints status details on a given buffer */
//#define POLYFIFO_DEBUG

#ifdef POLYFIFO_DEBUG
#include <stdio.h>
#define POLYFIFO_PRINTSTATS(name)                                                   \
do{                                                                                 \
    printf("Head: %d, Tail: %d\n", (int) name.head, (int) name.tail);               \
    printf("Empty: %d, Full: %d\n", POLYFIFO_ISEMPTY(name), POLYFIFO_ISFULL(name)); \
    for(size_t i = 0; i < name.len; i++){                                           \
        printf("D%d: %d\n", i, name.data[i]);                                       \
    }                                                                               \
}while(0)
#endif

/** @brief Declare data storage type for a given element type
 * Usage example: POLYFIFO_DECLARE(int)
 */
#define POLYFIFO_DECLARE(t)     \
struct polyfifo_ ## t ## _s {   \
    size_t head;                \
    size_t tail;                \
    size_t const len;           \
    t * const data;             \
}

/** @brief Define and initialize one buffer of given type, name, and buffer location
 * Usage example:
 * {
 *      int buf[10];
 *      POLYFIFO_DEFINE(int, myfifo, 10, buf);
 * }
 */
#define POLYFIFO_DEFINE(t, name, len, buf) struct polyfifo_ ## t ## _s name = {0, 0, len, buf}

/** @brief Returns true if a given buffer is empty */
#define POLYFIFO_ISEMPTY(name) (name.head == name.tail)

/** @brief Returns true if a given buffer is full */
#define POLYFIFO_ISFULL(name) (((name.head + 1) % name.len) == name.tail)

/** @brief Returns the number of free elements in a given buffer */
#define POLYFIFO_NUMEMPTY(name) (name.len - POLYFIFO_NUMFULL(name) - 1)

/** @brief Returne the number of full elements in a given buffer */
#define POLYFIFO_NUMFULL(name) ((name.head - name.tail + name.len) % name.len)

/** @brief Writes a value into a buffer, doing nothing if buffer full
 * @param name  Buffer name
 * @param val   Value to write
 */
#define POLYFIFO_WRITE(name, val)               \
do{                                             \
    if(!POLYFIFO_ISFULL(name)){                 \
        name.data[name.head] = (val);           \
        name.head = (name.head + 1) % name.len; \
    }                                           \
}while(0)

/** @brief Reads a value into a buffer, doing nothing if buffer empty
 * @param name  Buffer name
 * @param res   Pointer to variable to store result in
 */
#define POLYFIFO_READ(name, res)                \
do{                                             \
    if(!POLYFIFO_ISEMPTY(name)){                \
        (*res) = name.data[name.tail];          \
        name.tail = (name.tail + 1) % name.len; \
    }                                           \
}while(0)

#endif /* POLYFIFO_H_ */
