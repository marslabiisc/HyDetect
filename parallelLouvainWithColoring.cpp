// ***********************************************************************
//
//            Grappolo: A C++ library for graph clustering
//               Mahantesh Halappanavar (hala@pnnl.gov)
//               Pacific Northwest National Laboratory     
//
// ***********************************************************************
//
//       Copyright (2014) Battelle Memorial Institute
//                      All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
//
// 1. Redistributions of source code must retain the above copyright 
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
// notice, this list of conditions and the following disclaimer in the 
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its 
// contributors may be used to endorse or promote products derived from 
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// ************************************************************************

#include "defs.h"
#include "utilityClusteringFunctions.h"

using namespace std;

double algoLouvainWithDistOneColoring(graph* G, unsigned int *C, int nThreads, int* color, 
			int numColor, double Lower, double thresh, double *totTime, int *numItr) {
#ifdef PRINT_DETAILED_STATS_  
	printf("Within algoLouvainWithDistOneColoring()\n");
#endif
	if (nThreads < 1)
		omp_set_num_threads(1);
	else
		omp_set_num_threads(nThreads);
	int nT;
#pragma omp parallel
	{
		nT = omp_get_num_threads();
	}
#ifdef PRINT_DETAILED_STATS_  
	printf("Actual number of threads: %d (requested: %d)\n", nT, nThreads);
#endif

	double time1, time2, time3, time4; //For timing purposes  
	double total = 0, totItr = 0;	
	/* Indexs are vertex */
	unsigned int* pastCommAss;	//Store previous iteration's community assignment
	unsigned int* currCommAss;	//Store current community assignment
	//unsigned int* targetCommAss;	//Store the target of community assignment
	unsigned int* vDegree;	//Store each vertex's degree
	unsigned int* clusterWeightInternal;//use for Modularity calculation (eii)
	
	/* Indexs are community */
	Comm* cInfo;	 //Community info. (ai and size)
	Comm* cUpdate; //use for updating Community
	
	/* Book keeping variables */
	unsigned int    NV        = G->numVertices;
	unsigned int    NS        = G->sVertices;      
	unsigned int    NE        = G->numEdges;
	unsigned int    *vtxPtr   = G->edgeListPtrs;
	edge    *vtxInd   = G->edgeList;
	
	/* Modularity Needed variables */
	unsigned int totalEdgeWeightTwice;
	double constantForSecondTerm;
	double prevMod=Lower;
	double currMod=-1;
	double thresMod = thresh;
	int numItrs = 0;
		
	/********************** Initialization **************************/
	time1 = omp_get_wtime();
	vDegree = (unsigned int *) malloc (NV * sizeof(unsigned int)); assert(vDegree != 0);
	cInfo = (Comm *) malloc (NV * sizeof(Comm)); assert(cInfo != 0);
	cUpdate = (Comm*)malloc(NV*sizeof(Comm)); assert(cUpdate != 0);

	sumVertexDegree(vtxInd, vtxPtr, vDegree, NV , cInfo);	// Sum up the vertex degree
	/*** Compute the total edge weight (2m) and 1/2m ***/
	constantForSecondTerm = calConstantForSecondTerm(vDegree, NV);	// 1 over sum of the degree
	
	pastCommAss = (unsigned int *) malloc (NV * sizeof(unsigned int)); assert(pastCommAss != 0);     
	//Community provided as input:
	currCommAss = C; assert(currCommAss != 0);

        /*** Assign each vertex to its own Community ***/
	initCommAss( pastCommAss, currCommAss, NV);

	clusterWeightInternal = (unsigned int*) malloc (NV*sizeof(unsigned int)); assert(clusterWeightInternal != 0);
	
	/*** Create a CSR-like datastructure for vertex-colors ***/
	unsigned int * colorPtr = (unsigned int *) malloc ((numColor+1) * sizeof(unsigned int));
	unsigned int * colorIndex = (unsigned int *) malloc (NV * sizeof(unsigned int));
	unsigned int * colorAdded = (unsigned int *)malloc (numColor*sizeof(unsigned int));
	assert(colorPtr != 0);
        assert(colorIndex != 0);
	assert(colorAdded != 0);
	// Initialization
#pragma omp parallel for
	for(unsigned int i = 0; i < numColor; i++) {	
		colorPtr[i] = 0;
		colorAdded[i] = 0;
	}
	colorPtr[numColor] = 0;
	// Count the size of each color
#pragma omp parallel for
	for(unsigned int i = 0; i < NV; i++) {
		__sync_fetch_and_add(&colorPtr[(unsigned int)color[i]+1],1);
	}
	//Prefix sum:
	for(unsigned int i=0; i<numColor; i++) {
		colorPtr[i+1] += colorPtr[i];
	}	
	//Group vertices with the same color in particular order
#pragma omp parallel for
	for (unsigned int i=0; i<NV; i++) {
		unsigned int tc = (unsigned int)color[i];
		unsigned int Where = colorPtr[tc] + __sync_fetch_and_add(&(colorAdded[tc]), 1);
		colorIndex[Where] = i;
	}
	time2 = omp_get_wtime();
	printf("Time to initialize: %3.3lf\n", time2-time1);
#ifdef PRINT_DETAILED_STATS_	
	printf("========================================================================================================\n");
	printf("Itr      E_xx            A_x2           Curr-Mod         Time-1(s)       Time-2(s)        T/Itr(s)\n");
	printf("========================================================================================================\n");
#endif
#ifdef PRINT_TERSE_STATS_
  	printf("=====================================================\n");
	printf("Itr      Curr-Mod         T/Itr(s)      T-Cumulative\n");
	printf("=====================================================\n");
#endif
	while(true) {
		numItrs++;
		
		time1 = omp_get_wtime();
		for( unsigned int ci = 0; ci < numColor; ci++) // Begin of color loop
		{
#pragma omp parallel for
			for (unsigned int i=0; i<NV; i++) {
				clusterWeightInternal[i] = 0; //Initialize to zero
				cUpdate[i].degree =0;
				cUpdate[i].size =0;
			}
			unsigned int coloradj1 = colorPtr[ci];
			unsigned int coloradj2 = colorPtr[ci+1];
			
#pragma omp parallel for  
			for (unsigned int K = coloradj1; K<coloradj2; K++) {
				unsigned int i = colorIndex[K];
				unsigned int localTarget = -1;
				unsigned int adj1 = vtxPtr[i];
				unsigned int adj2 = vtxPtr[i+1];
				unsigned int selfLoop = 0;
				//Build a datastructure to hold the cluster structure of its neighbors:      	
				map<unsigned int, unsigned int> clusterLocalMap; //Map each neighbor's cluster to a local number
				map<unsigned int, unsigned int>::iterator storedAlready;
				vector<double> Counter; //Number of edges to each unique cluster
				
				if(adj1 != adj2) {
					//Add v's current cluster:
					clusterLocalMap[currCommAss[i]] = 0;
					Counter.push_back(0); //Initialize the counter to ZERO (no edges incident yet)
					//Find unique cluster ids and #of edges incident (eicj) to them
					selfLoop = buildLocalMapCounter(adj1, adj2, clusterLocalMap, Counter, vtxInd, currCommAss, i);
					//Calculate the max
					localTarget = max(clusterLocalMap, Counter, selfLoop, cInfo, vDegree[i], currCommAss[i], constantForSecondTerm);
				} else {
					localTarget = -1;
				}					
				//Update prepare
				if(localTarget != currCommAss[i] && localTarget != -1) {
					__sync_fetch_and_add(&cUpdate[localTarget].degree, vDegree[i]);
	         			__sync_fetch_and_add(&cUpdate[localTarget].size, 1);
					__sync_fetch_and_sub(&cUpdate[currCommAss[i]].degree, vDegree[i]);
					__sync_fetch_and_sub(&cUpdate[currCommAss[i]].size, 1);
				}//End of If()
				currCommAss[i] = localTarget;      
				clusterLocalMap.clear();      
			}//End of for(i)
			
			// UPDATE
#pragma omp parallel for  
			for (unsigned int i=0; i<NV; i++) {
				cInfo[i].size += cUpdate[i].size;
				cInfo[i].degree += cUpdate[i].degree;
			}
		}//End of Color loop						
		time2 = omp_get_wtime();
		
		time3 = omp_get_wtime();    
		double e_xx = 0;
		double a2_x = 0;	
		
		// CALCULATE MOD
#pragma omp parallel for  //Parallelize on each vertex
		for (unsigned int i =0; i<NV;i++){
			clusterWeightInternal[i] = 0;
		}
#pragma omp parallel for  //Parallelize on each vertex
		for (unsigned int i=0; i<NV; i++) {
			unsigned int adj1 = vtxPtr[i];
			unsigned int adj2 = vtxPtr[i+1];
			for(unsigned int j=adj1; j<adj2; j++) {
				if(currCommAss[vtxInd[j].tail] == currCommAss[i]){
					clusterWeightInternal[i] += (unsigned int)vtxInd[j].weight;
				}
			}
		}		
		
#pragma omp parallel for \
reduction(+:e_xx) reduction(+:a2_x)
		for (unsigned int i=0; i<NV; i++) {
			e_xx += clusterWeightInternal[i];
			a2_x += (cInfo[i].degree)*(cInfo[i].degree);
		}
		time4 = omp_get_wtime();
		
		currMod = e_xx*(double)constantForSecondTerm  - a2_x*(double)constantForSecondTerm*(double)constantForSecondTerm;
		
		totItr = (time2-time1) + (time4-time3);
		total += totItr;

#ifdef PRINT_DETAILED_STATS_  
		printf("%d \t %g \t %g \t %lf \t %3.3lf \t %3.3lf  \t %3.3lf\n",numItrs, e_xx, a2_x, currMod, (time2-time1), (time4-time3), totItr );    
#endif
#ifdef PRINT_TERSE_STATS_
   		printf("%d \t %lf \t %3.3lf  \t %3.3lf\n",numItrs, currMod, totItr, total);
#endif
		if((currMod - prevMod) < thresMod) {	
			break;
		}
		
		prevMod = currMod;
	}//End of while(true)
	*totTime = total; //Return back the total time
        *numItr  = numItrs;

#ifdef PRINT_DETAILED_STATS_  
	printf("========================================================================================================\n");
	printf("Total time for %d iterations is: %lf\n",numItrs, total);  
	printf("========================================================================================================\n");
#endif
#ifdef PRINT_TERSE_STATS_  
	printf("========================================================================================================\n");
	printf("Total time for %d iterations is: %lf\n",numItrs, total);  
	printf("========================================================================================================\n");
#endif
	//Cleanup:
        free(vDegree); free(cInfo); free(cUpdate); free(clusterWeightInternal);
        free(colorPtr); free(colorIndex); free(colorAdded);
	free(pastCommAss);
	
	return prevMod;
	
}//End of algoLouvainWithDistOneColoring()
