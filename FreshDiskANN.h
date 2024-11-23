#ifndef FRESHDISKANN_H
#define FRESHDISKANN_H

#pragma once
#include <cstddef>
#include <cassert>
#include <string>

#include "FreshDiskANN Components/CompressedLTI.h"
#include "FreshDiskANN Components/PrecisionLTI.h"
#include "FreshDiskANN Components/DeleteList.h"

#include "FreshVamanaIndex.h"

class FreshDiskANN {

private:
    const double alpha;
    const size_t outDegreeBound;

    CompressedLTI compressedLTI; //RAM
    PrecisionLTI precisionLTI; //SSD

    //Instancias del FreshVamanaIndex
    std::shared_ptr<FreshVamanaIndex> roTempIndex; //RAM
    std::shared_ptr<FreshVamanaIndex> rwTempIndex; //RAM

    DeleteList deleteList; //RAM


    //RO-TempIndex (RAM)
    //RW-TempIndex (RAM)
    //DeleteList (RAM)


public:

    static const std::string DEFAULT_FILE_PATH_PRECISION_LTI;
    static const size_t DEFAULT_OUT_DEGREE_BOUND;
    static const double DEFAULT_ALPHA;
    static const double DEFAULT_DELETE_ACCUMULATION_FACTOR;
    static const size_t DEFAULT_SEARCH_LIST_SIZE;

    //Constructors
    FreshDiskANN(const double alpha = DEFAULT_ALPHA , const size_t outDegreeBound = DEFAULT_OUT_DEGREE_BOUND) : alpha(alpha), outDegreeBound(outDegreeBound), precisionLTI(DEFAULT_FILE_PATH_PRECISION_LTI, outDegreeBound) {}

    //Methods

    //Insert (xp): Afecta solo al RW-TempIndex. Algoritmo 2
    void insert(std::shared_ptr<GraphNode> p, std::shared_ptr<GraphNode> s, size_t searchListSize = DEFAULT_SEARCH_LIST_SIZE, double alpha = DEFAULT_ALPHA, size_t outDegreeBound = DEFAULT_OUT_DEGREE_BOUND);


    //Insert (xp): Afecta solo al RW-TempIndex. Algoritmo 2
    //Delete (p): Los puntos que se quieren eliminar se añaden a la DeleteList. No se eliminan inmediatamente del LTI ni del TempIndex, pero no aparecerán en resultados de búsqueda.
    //GreedySearch (xq, K, L): La búsqueda se realiza consultando el LTI, el RW-TempIndex y todos los RO-TempIndex. Los resultados se agregan y se eliminan aquellos puntos que estén en la DeleteList (es decir, los puntos eliminados no se devuelven en los resultados).

    /*
    StreamingMerge():
    1. Delete Phase:
        //Recuerda que el LTI contiene puntos en su version comprimida
        1.1. LTI es grande, por lo que necesitamos cargarlo en memoria en bloques.     Por eso, el proceso carga pequeños bloques del índice desde el SSD (por ejemplo, un subconjunto de los puntos y sus conexiones en el gráfico) y trabaja con ellos de manera incremental.
        1.2. Para cada bloque, se eliminan los puntos en DeleteList.


        1.1. Delete (Algo 4) from LTI all points in DeleteList.
        1.2.

     */

};


/*LTI RAM-SSD
 * Insertions and deletions do not affect the LTI in real time.
 *Tiene 2 partes
    *   CompressedLTI (RAM)
        *   Almacena los puntos en su versión comprimida (Product Quantization: 25-32 bytes).
    *   PrecisionLTI (SSD)
        *   Almacena los puntos en su versión de precisión completa.
        *   Almacena las conexiones de cada punto en el grafo.
        *   Dividido en bloques de tamaño fijo (4kB).
            *   El vector de coordenadas completo de cada punto se almacena en el bloque.
            *   Los outNeighbours de cada punto se almacenan en el resto del bloque.
                *   Si un punto tiene menos vecinos (outNeighbours) que el maximo permitido (outDegreeBound), se rellena con 0s para completar los 4kB.

 */




#endif //FRESHDISKANN_H
