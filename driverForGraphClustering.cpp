


#include "defs.h"

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include "graphHOST.h"
#include "graphGPU.h"
#include "communityGPU.h"
#include <iostream>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string>
#include "Controller.h"
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <string>
#include <iostream>
#include <limits>
#include <string.h>

#include <unistd.h>
#include <cassert>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <inttypes.h>
#include <vector>
#include <omp.h>
using namespace std;
double calculateRatioCD(graph graph, unsigned int NUM_VER){
        bool *ghostVertex;
        ghostVertex = (bool*) calloc(graph.nnodes, sizeof(bool));
        //mark other vertices as ghost so that while calculation it doesn't take care of these vertices
        #pragma omp parallel for num_threads(16) schedule(static)
        for(unsigned int i = NUM_VER; i < graph.nnodes; i++)
                ghostVertex[i] = true;

        unsigned int *currCtx, currActiveNodes, currActiveEdges;
        unsigned int *Parents;
        currCtx = (unsigned int*) malloc( NUM_VER * sizeof(unsigned int));
        std::copy(graph.srcsrc,  graph.srcsrc+NUM_VER, currCtx);
        Parents = (unsigned int*) malloc(graph.nnodes * sizeof(unsigned int));
        std::copy(graph.srcsrc, graph.srcsrc + graph.nnodes, Parents);

        currActiveNodes = NUM_VER;
        currActiveEdges = graph.psrc[NUM_VER];

        double cpuTime = 0, gpuTime = 0;
        unsigned int *hostParents, *deviceParents;
        unsigned int *cpuCurrCtx, *gpuCurrCtx;
        omp_set_num_threads(2);
        omp_set_nested(1);
        int part = -1;
	#pragma omp parallel sections
        {
                #pragma omp section
                {
                        hostParents = (unsigned int*) malloc( graph.nnodes * sizeof(unsigned int));
                        std::copy( Parents, Parents + graph.nnodes,  hostParents);                  
                        cpuCurrCtx = (unsigned int*) malloc( currActiveNodes * sizeof(unsigned int));
                        std::copy( currCtx, currCtx + currActiveNodes, cpuCurrCtx);
                        //cpuTime = cdCPU( graph, hostParents, cpuCurrCtx, currActiveNodes, currActiveEdges, ghostVertex, part);
			
                        free( cpuCurrCtx);
                        free( hostParents);
                }
                #pragma omp section
                {
                        deviceParents = (unsigned int*) malloc( graph.nnodes * sizeof(unsigned int));
                        std::copy( Parents, Parents + graph.nnodes,  deviceParents);
                        gpuCurrCtx = (unsigned int*) malloc( currActiveNodes * sizeof(unsigned int));
                        std::copy( currCtx, currCtx + currActiveNodes, gpuCurrCtx);
                        //gpuTime = cdGPU( graph, deviceParents, gpuCurrCtx, currActiveNodes, currActiveEdges, ghostVertex, part);
                        free( gpuCurrCtx);
                        free( deviceParents);

                }
        }
        std::cout<<"CPU Exe Time: "<<cpuTime<<" GPU Exe Time: "<<gpuTime<<"\n";
        free( Parents);
        free( currCtx);
        free( ghostVertex);

return gpuTime/(cpuTime+gpuTime);
}
double partGraphCD( graph graph, int parts, double *sizeofData){
     /*   int NUM_VER = 10000;
        if( graph.nnodes < NUM_VER)
                  NUM_VER =  graph.nnodes / 2;
        double ST = get_time();
        double partitionRatio = calculateRatioCD(graph, NUM_VER);
        std::cout<<"CPU & GPU Exe time: "<<get_time()-ST<<"\n";
        *sizeofData += 3 * graph.nnodes * sizeof(unsigned int);
        *sizeofData += graph.nedges * sizeof(unsigned int);
        *sizeofData/= (1024 * 1024);
        std::cout<<"Size of data: "<<*sizeofData<<" MB\n";
	if( *sizeofData > 3200){
                double otherPart = (double)(1 - partitionRatio);
                if( (otherPart * graph.nedges) < 500000000){
                        while( (otherPart * graph.nedges) < 500000000){
                                partitionRatio -= 0.01;
                                otherPart = (double)(1 - partitionRatio);
                   //     std::cout<<"Ratio: "<<partitionRatio<<"\n";
                        }
                }
        }*/
	double partitionRatio=0.5;
	graph.partGraph( 2, partitionRatio);
        return partitionRatio;
}

/*void indCompCC( Graph graph, unsigned int *&Parents){
        unsigned int *hostParents, *deviceParents;
        omp_set_num_threads(2);
        omp_set_nested(1);
        unsigned int *cpuCurrCtx, *gpuCurrCtx;
        unsigned int cpuCurrNodes, gpuCurrNodes;
        unsigned int cpuCurrEdges, gpuCurrEdges;
        bool *cpuGhostNodes, *gpuGhostNodes;
        int cpuPart = 0, gpuPart = 1;
        #pragma omp parallel sections
        {
                #pragma omp section
                {
                        cpuCurrNodes = graph.PerPartNodes[ cpuPart];
                        cpuCurrEdges = graph.PerPartEdges[ cpuPart];
                        std::cout<<"CPU Part Nodes: "<<cpuCurrNodes<<" Edges: "<<cpuCurrEdges<<"\n";
                        hostParents = (unsigned int *) malloc( graph.nnodes * sizeof(unsigned int));
                        cpuCurrCtx = (unsigned int *) malloc(cpuCurrNodes * sizeof(unsigned int));
                        cpuGhostNodes = (bool*) calloc( graph.nnodes, sizeof(bool));

                        std::copy(Parents, Parents+graph.nnodes, hostParents);
                        std::copy( graph.srcsrc+graph.startVertex[ cpuPart], graph.srcsrc+graph.endVertex[ cpuPart], cpuCurrCtx);
                        #pragma omp parallel for schedule(static) num_threads(8)
                        for(unsigned int i = graph.endVertex[ cpuPart]; i < graph.nnodes; i++)
                                cpuGhostNodes[i] = true;
			ccCPU( graph, hostParents, cpuCurrCtx, cpuCurrNodes, cpuCurrEdges, cpuGhostNodes, cpuPart);
                        std::copy(hostParents, hostParents+cpuCurrNodes, Parents);
                        
			free( hostParents);
                        free( cpuCurrCtx);
                        free( cpuGhostNodes);
                }

		#pragma omp section
                {
                        gpuCurrNodes = graph.PerPartNodes[ gpuPart];
                        gpuCurrEdges = graph.PerPartEdges[ gpuPart];
                        std::cout<<"GPU Part Nodes: "<<gpuCurrNodes<<" Edges: "<<gpuCurrEdges<<"\n";
                        deviceParents = (unsigned int*) malloc( graph.nnodes * sizeof(unsigned int));
                        gpuCurrCtx = (unsigned int *) malloc( gpuCurrNodes * sizeof(unsigned int));
                        gpuGhostNodes =  (bool*) calloc( graph.nnodes, sizeof(bool));

                        std::copy( Parents, Parents+graph.nnodes, deviceParents);
                        std::copy( graph.srcsrc+graph.startVertex[ gpuPart], graph.srcsrc+graph.endVertex[ gpuPart], gpuCurrCtx);
                        #pragma omp parallel for schedule(static) num_threads(8)
                        for(unsigned int i = 0; i < graph.startVertex[ gpuPart]; i++)
                                gpuGhostNodes[i] = true;

//                        deviceFindMst( graph, deviceParents, gpuCurrCtx, gpuCurrNodes, gpuCurrEdges, gpuGhostNodes, gpuPart);
			ccGPU( graph, deviceParents, gpuCurrCtx, gpuCurrNodes, gpuCurrEdges, gpuGhostNodes, gpuPart);
                        std::copy(deviceParents+cpuCurrNodes, deviceParents +graph.nnodes, Parents+cpuCurrNodes);
                        //Free Mem
                        free( deviceParents);
                        free( gpuCurrCtx);
                        free( gpuGhostNodes);
                }
      }
}
*/

int main(int argc, char** argv) {
 


  graph* G = (graph *) malloc (sizeof(graph));
  graph* G1 = (graph *) malloc (sizeof(graph));
  graph* G2 = (graph *) malloc (sizeof(graph));
  graph* Gnew=(graph *) malloc (sizeof(graph));
	
  graph* Gnew1 = (graph *) malloc (sizeof(graph));
  
  graph g1;
  g1.readFromGR(argv[1]);
  double exeTime = get_time();
  double partTime = get_time();
  double sizeofData = 0;
  int parts = 2;
  double partitionRatio=.75;
  unsigned int mid=g1.partGraph( 2, partitionRatio);

  unsigned int total=g1.nnodes;
  unsigned int NV=mid;
	
  g1.numVertices=mid;
  g1.numEdges=g1.no1/2;
//  cout<<NV<<" "<<g.no1<<endl;
  bool *dirty=new bool[NV];
  bool *dirtycpu=new bool[NV];
  bool *dirtygpu=new bool[total-NV]; //need to change
  bool *dirtyg=new bool[total-NV]; //need to change
  //cout<<"4"<<endl;
  long *C_orig=(long *)malloc(sizeof(long)*NV);
  for(long i=0;i<NV;i++)
	dirtycpu[i]=false;
  for(long i=0;i<total-NV;i++){
	dirtygpu[i]=false;
	dirtyg[i]=false;}
  int coloring = 0;
  G=&g1;
//	cout<<"5"<<endl;
/*	ofstream fout("deg.txt");
	ofstream fout1("edge.txt");
 for(int i=0;i<NV;i++)
	fout<<g.edgeListPtrs[i]<<endl;
	for(int i=0;i<g.no1;i++)
	fout1<<g.edgeList[i].head<<" "<<g.edgeList[i].tail<<" "<<g.edgeList[i].weight<<endl;*/
  displayGraphCharacteristics(&g1);
  duplicateGivenGraph(G,G1);
 // displayGraphCharacteristics(G1);
//return 0;
 // duplicateGivenGraph(G,G2);

  for(long i=0;i<NV;i++)
	C_orig[i]=-1;
  /*for(int i=0;i<g.numVertices;i++)
	cout<<g.edgeListPtrs[i]<<" ";*/

   	GraphHOST input_graph;
      	input_graph.nb_nodes = total-mid;
      	input_graph.degrees.resize(input_graph.nb_nodes);

      	for(int i=0;i<input_graph.nb_nodes;i++)
          input_graph.degrees.at(i)=g1.edgeListPtrs1[i];
   
 	unsigned int *statIndices=(unsigned int *)malloc(input_graph.nb_nodes*sizeof(unsigned int));
        unsigned int *edges=(unsigned int*)malloc(g1.no2*sizeof(unsigned int));

 	input_graph.nb_links =g1.no2;
    	input_graph.links.resize(input_graph.nb_links);
    	//input_graph.links=g.links;
	        for(int i=0;i<input_graph.nb_links;i++)
          input_graph.links.at(i)=g1.links.at(i);
	input_graph.weights.resize(input_graph.nb_links);
	for(int i=0;i<input_graph.nb_links;i++)
		input_graph.weights.at(i)=1;
/*nput_graph.total_weight = 0;
    for (unsigned int i = 0; i < input_graph.nb_nodes; i++) {
        total_weight += (double) weighted_degree(i);
    }*/
        double threshold = 0.000001;
	Community *dev1_community;
	Community dev_community(input_graph, -1, .000001);
	dev1_community=&dev_community;
long *c=(long *)malloc((total-mid)*sizeof(long));
long *cact=(long *)malloc((total-mid)*sizeof(long));
for(int i=0;i<total-NV;i++){
	c[i]=-1;cact[i]=0;}
	int i=0;

	for(std::vector<unsigned long>::iterator it=input_graph.degrees.begin();it<input_graph.degrees.end();it++)
                {*(statIndices+i)=*it;
                        i++;
                }
	i=0;
//	return 0;
 int f=0;
	for(std::vector<unsigned int>::iterator it=input_graph.links.begin();it<input_graph.links.end();it++)
                {*(edges+i)=*it;
                        i++;
                }

	#pragma omp parallel sections
        {
                #pragma omp section
                {
                 
		f=gpuonly(input_graph,c,statIndices,edges,dev1_community,dirtyg,dirtygpu,0,G1,mid);
	cout<<(*dev1_community).g.nb_nodes<<endl;
//	print_vector((*dev1_community).g.weights,"weights");
//	print_vector((*dev1_community).g.links,"links");
	cout<<"gpu ends"<<endl;
		}

		#pragma omp section
                {
                  Gnew=cpuonly(G, Gnew,G1, C_orig,dirty,dirtycpu,0);
//	cout<<Gnew->numVertices<<endl;
		//	cout<<Gnew->numEdges<<endl;
		cout<<"cpu ends"<<endl;
		}

	}
//for(int i=0;i<input_graph.nb_nodes;i++)
//	cout<<"comc"<<" "<<c[i]<<endl;
//graph *Gnewc=(graph *)malloc(sizeof(graph));
//Gnewc=cpuonly(Gnew,Gnew,Gnew,C_orig,dirty,dirtycpu,1);
//return 0;
(Gnew->bord)=(bool *)malloc(sizeof(bool)*Gnew->numVertices);
(Gnew->bordno)=(unsigned int *)malloc(sizeof(unsigned int)*Gnew->numVertices);
 Gnew->bordvalue=new vector<unsigned int>[Gnew->numVertices];
 // duplicateGivenGraph(&g1,G);

//mo is the structure to move to GPU partition
unsigned int edgec=0;
unsigned int newV=0;
unsigned int newVd=0;
for(int i=0;i<NV;i++)
{
        if(dirtycpu[i]){
        newV++;
	if(dirty[i] && !dirtycpu[i])
	newVd++;
			}
}



int nn=newV;
move1 mo;


verticesToMoveToGPU(G1,dirtycpu,&mo,c,mid,(*dev1_community).g.nb_nodes,C_orig);
cout<<"!"<<endl;
//return 0;
//cout<<mo.vertex<<" "<<mo.edg<<endl;

//cout<<mo.edgesa[0]<<endl;

//return 0;
//mo1 structure to move from GPU to CPU
//unsigned int edgec=0;
//verticesToMoveToCPU((*dev1_community),dirtygpu,mo1,C_orig,total,NV);
unsigned int newV1=0;
unsigned int newV1d=0;
for(int i=0;i<total-NV;i++)
{
       // if(dirtygpu[i])
      //  newV1++;
	
	if(!dirtygpu[i] && dirtyg[i])
	newV1d++;
	
		//	}	
}
move2 mo1 ;

verticesToMoveToCPU(statIndices,edges,dirtygpu,&mo1,C_orig,total,NV,&g1,Gnew,mid);
int it=1;
for(int i=0;i<Gnew->numVertices;i++)
	{
		Gnew->bord[i]=G->bord[i];
		Gnew->bordno[i]=G->bordno[i];
		Gnew->bordvalue[i]=G->bordvalue[i];
	}
NV=G1->numVertices;
unsigned int  NE=G1->numEdges;
unsigned int * vtxPtr=(G1->edgeListPtrs);
//cout<<vtxPtr<<endl;
cout<<G1->edgeListPtrs[1]<<endl;
edge *vtxInd=(G1->edgeList);
//cout<<vtxInd<<endl;
bool *f1=(bool*)malloc(sizeof(bool)*NV);
unsigned int *count1=(unsigned int *)malloc(sizeof(unsigned int )*(NV+1));
//vector<long> *borderval=new vector<long>[NV+1];
bool *bord1=(bool *)malloc(sizeof(bool)*Gnew->numVertices);
bool *dirty3=(bool *)malloc(sizeof(bool)*Gnew->numVertices);
bool *dirty2=(bool *)malloc(sizeof(bool)*Gnew->numVertices);
graph* Gnew2=(graph *)malloc(sizeof(graph));
duplicateGivenGraph(Gnew,Gnew2);
unsigned int *bordno1=(unsigned int *)malloc(sizeof(unsigned int)*Gnew->numVertices);
vector<unsigned int> *bordvalue1=new vector<unsigned int>[Gnew->numVertices];
for(int i=0;i<Gnew->numVertices;i++){
	bord1[i]=false;
	bordno1[i]=0;
	dirty3[i]=false;
	dirty2[i]=true;
	}
cout<<"((("<<endl;
for(long i=0;i<NV;i++){
        f1[i]=false;
        count1[i]=0;}
int j=1;
for(long i=0;i<NV;i++)
	{
		if(dirtycpu[i])
		count1[i]=j;
		j++;

	}
//bool *dirty3=(bool *)malloc(sizeof(bool)*Gnew->numVertices);
//bool *dirty2=(bool *)malloc(sizeof(bool)*Gnew->numVertices);

int k=0;
//cout<<")))"<<endl;

long* c1=(long *)malloc(sizeof(long)*((*dev1_community).g.nb_nodes+newV1d+mo.vertex));
for( int i=0;i<(*dev1_community).g.nb_nodes+newV1d+mo.vertex;i++)
        c1[i]=-1;
//turn 0;
cout<<"our calculation"<<(*dev1_community).g.nb_nodes+newV1d+mo.vertex<<endl;
long *co1=(long *)malloc(sizeof(long)*(Gnew->numVertices+newVd+mo1.vertex));
for(int i=0;i<Gnew->numVertices+newVd+mo1.vertex;i++)
	co1[i]=0;
Gnew1=modifyCPUstructure(Gnew,G1,dirty,C_orig,co1,&mo1,mid,(*dev1_community).g.nb_nodes,dirty3,dirtycpu);
//return 0;
/*for(int i=0;i<total-NV;i++)
	cact[i]=c[i];*/
Community cu=modifyGPUstructure(dev1_community,statIndices,edges,dirtyg,dirtygpu,c,total,NV,&mo,Gnew,mid,c1,1,cact);
statIndices=(unsigned int *)malloc(sizeof(unsigned int)*((*dev1_community).g.nb_nodes));
edges=(unsigned int *)malloc(sizeof(unsigned int)*(*dev1_community).g.nb_links);
for(int i=0;i<(*dev1_community).g.nb_nodes;i++)
        statIndices[i]=(*dev1_community).g.indices[i];
for(int i=0;i<(*dev1_community).g.nb_links;i++)
        edges[i]=(*dev1_community).g.links[i];
cout<<"done"<<" "<<newV1d<<" "<<newV1<<endl;
//return 0;
newV1=0;
//for(int i=0;i<Gnew1->numVertices;i++)
//	dirty3[i]=true;
//to change
for(int i=0;i<cu.g.nb_nodes;i++){
	dirtyg[i]=false;
	dirtygpu[i]=false;}
/*return 0;
for(int i=0;i<(*dev1_community).g.nb_nodes;i++)
	{

		if(dirtyg[i]==true)
			newV1++;
	}
*/
/*for(int i=0;i<(*dev1_community).g.nb_nodes+newV1;i++)
	cout<<"community"<<" "<<c1[i]<<endl;*/
//return 0;
//#pragma omp parallel for
/*for(long i=0;i<NV;i++)
        {
                long adj1=vtxPtr[i];
                long adj2=vtxPtr[i+1];
//		cout<<adj1<<" "<<adj2<<endl;
//		cout<<"value of k"<<" "<<k<<endl;
      	 for(long j=adj1;j<adj2;j++)
                {
                        if(dirtycpu[(vtxInd[j].tail)]){
                                bord1[C_orig[i]]=true;
                                bordno1[C_orig[i]]=bordno1[C_orig[i]]+1;
                                bordvalue1[C_orig[i]].push_back(count1[(vtxInd[j].tail)]+newV1+((*dev1_community).g.nb_nodes));
	
			  }
			else if(!dirtycpu[(vtxInd[j].tail)] && G1->bord[(vtxInd[j].tail+1)-1])
			{
				bord1[C_orig[i]]=true;
				bordno1[C_orig[i]]+=G1->bordvalue[i].size();
			//	cout<<G1->bordvalue[i].size()<<endl;
			for(std::vector<unsigned int> :: iterator it=G1->bordvalue[i].begin();it!=G1->bordvalue[i].end();it++)
			{	bordvalue1[C_orig[i]].push_back(c[*it-mid]);
			//	fout<<c[*it-mid]<<endl;
			//
			}

			}
				
                }
		k++;
        }
*/
/*for(long i=0;i<NV;i++)
	{

		if(dirtycpu[i])
		{

			long adj1=vtxPtr[i];
			long adj2=vtxPtr[i+1];
			for(long j=adj1;j<adj2;j++)
			{

				bord1[C_orig[vtxInd[j].tail]]=true;
				bordno1[C_orig[vtxInd[j].tail]]+=1;
			        bordvalue1[C_orig[vtxInd[j].tail]].push_back();
			}
		}
	}		
*/			
//cout<<"DD"<<endl;
//move2 mo1 ;
//verticesToMoveToCPU(statIndices,edges,dirtygpu,&mo1,C_orig,total,NV,&g1,Gnew,mid);
//cout<<mo1.vertex<<" "<<mo1.edgeno<<endl;
//cout<<"@"<<endl;
//return 0;
//return 0;
//return 0;
//cout<<"after"<<G1->edgeListPtrs[0]<<" "<<G1->edgeListPtrs[1]<<endl;
//unsigned int *edgeListPtrsM=(unsigned int *)malloc(newV1*sizeof(unsigned int));
//Gnew1=modifyCPUstructure(Gnew,G1,dirty,C_orig,&mo1,mid,(*dev1_community).g.nb_nodes,dirty3);
//cout<<"*"<<newV1<<" "<<(cu).g.nb_nodes<<endl;
long *c2=(long *)malloc(sizeof(long)*(cu).g.nb_nodes+newV1);
/*for( int i=0;i<(*dev1_community).g.nb_nodes+newV1d+mo.vertex;i++)
        cout<<"community"<<" "<<c1[i]<<" "<<Gnew2->bord[0]<<endl;
*/
verticesToMoveToGPU1(Gnew2,Gnew1,dirty3,&mo,c1,mid,(cu).g.nb_nodes,f,co1);
cout<<"ddone"<<endl;
//return 0;
/*for( int i=0;i<(*dev1_community).g.nb_nodes+newV1d+mo.vertex;i++)
        cout<<"community"<<" "<<c1[i]<<endl;
*/
//return 0;
total=mo.vertex+(cu).g.nb_nodes;
it=0;
Community cu1=modifyGPUstructure(&cu,statIndices,edges,dirtyg,dirtygpu,c1,total,(cu).g.nb_nodes,&mo,Gnew2,mid,c2,it,c);
cout<<"////"<<endl;
return 0;
//return 0;
/*unsigned int* c1=(unsigned int *)malloc(sizeof(unsigned int)*((*dev1_community).g.nb_nodes+newV1));
//return 0;
for( int i=0;i<(*dev1_community).g.nb_nodes+newV1;i++)
	c1[i]=0;
/*void modifyGPUstructure(Community *dev1_community,unsigned int *statIndices,unsigned int*edges,bool *dirtyg,int *c)

//GraphHOST ag;
//Community cu;*/
//return 0;
//Community cu=modifyGPUstructure(dev1_community,statIndices,edges,dirtyg,c,total,NV,&mo,Gnew,mid,c1);
int new1=0;
//return 0;
//cout<<"done community"<<endl;
/*for(int i=0;i<Gnew->numVertices;i++)
        {

                if(dirty3[i])
                        new1++;
		dirty2[i]=true;
        }
*/
cout<<"**********"<<endl;
/*NV=Gnew->numVertices;
unsigned int NV1=NV+new1;
move1 *mo2=new move1[NV1];
movefinal(Gnew2,Gnew1,dirty3,dirty2,mo2,c1,mid,((*dev1_community).g.nb_nodes+newV1),bord1,bordno1,bordvalue1);
//modifyGPUstructure(dev11_community,statIndices,edges,dirtyg1,c1,total,NV,mo2,Gnew1,mid,c2);
//cout<<(cu).g.nb_nodes<<" "<<(cu).g.nb_links<<endl;
Community *dev11_community;
       // Community dev112_community(ag, -1, .000001);
        dev11_community=&cu;

cout<<"&"<<endl;
total=Gnew->numVertices+(*dev1_community).g.nb_nodes;
NV=Gnew->numVertices;
//modifyGPUstructure(dev11_community,statIndices,edges,dirtyg,c1,total,NV,mo2,Gnew1,mid,c1);

unsigned int newV11=newV;
unsigned int newV12=newV1;
 edgec=0;
 newV1=0;




//edge count 


//Gnew->numVertices=Gnew->numVertices+newV-newV11;
//Gnew->numEdges=Gnew->edgeListPtrs[Gnew->numVertices];
//return 0;
//cpu graph structure changed now change gpu graph structure
 

/*unsigned int * C_orig1=(unsigned int*)malloc((Gnew->numVertices)*sizeof(unsigned int));

graph* Gnew1 = (graph *) malloc (sizeof(graph));
for(long i=0;i<Gnew->numVertices;i++)
	C_orig1[i]=-1;
//cout<<Gnew->numVertices<<" "<<Gnew->numEdges<<endl;
/*for(int i=0;i<=Gnew->numVertices;i++)
	cout<<Gnew->edgeListPtrs[i]<<" ";*/
//check GPU modified graph and previous cpu weight  update
//according to mo and mo1 change Gnew and statVertices and edges
//Gnew1=runMultiPhaseLouvainAlgorithm(Gnew, C_orig1,0, 1000, 0.0001, .0001, 5);


return 0;


//cout << typeid((*dev_community).g.nb_links).name() << endl;
/*
unsigned int *edgeP1=(unsigned int*)malloc(sizeof(unsigned int)*((*dev_community).g.nb_nodes+newV1));
for(unsigned int i=0;i<(*dev_community).g.nb_nodes;i++)
        edgeP1[i]=statIndices[i];


unsigned int  *edge21=(unsigned int*)malloc(sizeof(unsigned int )*(edgec1+(*dev_community).g.nb_links));
for(unsigned int i=0;i<(statIndices[(*dev_community).g.nb_links]);i++)
        {

     //   edge2[i].head=Gnew->edgeList[i].head;
      //  edge2[i].tail=Gnew->edgeList[i].tail;
      //  edge2[i].weight=Gnew->edgeList[i].weight;
      edge21[i]=edges[i];
        }







//unsigned 


/*Gnew->edgeList1=(edge *)malloc(sizeof(edge)*(Gnew->edgeListPtrs[Gnew->numVertices+1]));
for(int i=0;i<Gnew->numVertices+1;i++)
	cout<<Gnew->edgeListPtrs[i]<<" ";
cout<<endl;
for(int i=0;i<Gnew->edgeListPtrs[Gnew->numVertices];i++)
	cout<<Gnew->edgeList[i].head<<" "<<Gnew->edgeList[i].tail<<" "<<Gnew->edgeList[i].weight<<endl;
int j=0;
unsigned int newV=0;
unsigned int newE=0;
for(unsigned int i=0;i<G->numVertices;i++)
	{

		if(dirty[i])
		{	
			newV++;
			
		}

	}

for(unsigned int i=0;i<(Gnew->edgeListPtrs[Gnew->numVertices+1]);i++)
{
		if(!dirty[i]) //if the vertex is doubtful
		{
		
			//	Gnew->edgeListPtrs[i]=Gnew->edgeListPtrs[i-1];
			Gnew->edgeList1[j].head=Gnew->edgeList[i].head;
			Gnew->edgeList1[j].tail=Gnew->edgeList[i].tail;
			Gnew->edgeList1[j].weight=Gnew->edgeList[i].weight;
			j++;
			
		}
	
unsigned int*edgeListPtrs1=(unsigned int *)malloc(sizeof(unsigned int)*(Gnew->numVertices+1));
for(unsigned int i=0;i<Gnew->numVertices+1;i++)
	edgeListPtrs1[i]=Gnew->edgeListPtrs[i];
Gnew->edgeListPtrs=(unsigned int *)malloc(sizeof(unsigned int)*(Gnew->numVertices+newV+1));
for(int i=0;i<Gnew->numVertices+1;i++)
	Gnew->edgeListPtrs[i]=edgeListPtrs1[i];
for(int i=Gnew->numVertices+2;i<Gnew->numVertices+1+newV;i++)
	Gnew->edgeListPtrs[i]=0;
//add doubtful vertex edges
int cc=1;//to check cumulative degree sequence
for(unsigned int i=0;i<2*G->numEdges;i++)
        {
                        unsigned int*weigh=(unsigned int *)malloc(sizeof(unsigned int)*(Gnew->numVertices));
		int k=0;
                if(dirty[G->edgeList[i].head] && !dirty[G->edgeList[i].tail])
                {
		
			unsigned int adj1=G->edgeListPtrs[i];
			unsigned int adj2=G->edgeListPtrs[i+1];
			for(unsigned int jj=adj1;jj<adj2;jj++)
				{	
                        Gnew->edgeList1[j].head=G->edgeList[i].head;
                        Gnew->edgeList1[j].tail=G->edgeList[c[i]].tail;
                        Gnew->edgeList1[j].weight=1;
                        j++;
			Gnew->edgeListPtrs[c[(Gnew->edgeList1[i].head+1)]+1]-=1;
                        Gnew->edgeListPtrs[c[(Gnew->edgeList1[i].tail+1)]+1]-=1;
			Gnew->edgeListPtrs[Gnew->numVertices+cc]+=1;
			}
			cc++;
                }
		if(dirty[G->edgeList[i].tail] && !dirty[G->edgeList[i].head])
		{
			 Gnew->edgeList1[j].head=G->edgeList[c[i]].head;
                        Gnew->edgeList1[j].tail=G->edgeList[i].tail;
                        Gnew->edgeList1[j].weight=weigh[c[i]];
                        j++;
                        Gnew->edgeListPtrs[(Gnew->edgeList1[i].head+1)-(Gnew->numVertices)]-=1;
                        Gnew->edgeListPtrs[(Gnew->edgeList1[i].tail+1)-(Gnew->numVertices)]-=1;
                        Gnew->edgeListPtrs[cc]+=1;
                        cc++;



		}
		if(dirty[G->edgeList[i].tail] && dirty[G->edgeList[i].head])
                {
                         Gnew->edgeList1[j].head=G->edgeList[i].head;
                        Gnew->edgeList1[j].tail=G->edgeList[i].tail;
                        Gnew->edgeList1[j].weight=1;
                        j++;
                        Gnew->edgeListPtrs[(Gnew->edgeList1[i].head+1)-(Gnew->numVertices)]-=1;
                        Gnew->edgeListPtrs[(Gnew->edgeList1[i].tail+1)-(Gnew->numVertices)]-=1;
                        Gnew->edgeListPtrs[cc]+=1;
                        cc++;



                }



        }

if(dirty[i] && (Gnew->edgeList1[i].head!=Gnew->edgeList1[i].tail) ){
                        Gnew->edgeListPtrs[Gnew->edgeList1[i].head+1]-=1;
                        Gnew->edgeListPtrs[Gnew->edgeList1[i].tail+1]-=1;

}*
else if(dirty[i] && (Gnew->edgeList1[i].head==Gnew->edgeList1[i].tail))
                                        Gnew->edgeListPtrs[Gnew->edgeList1[i].head+1]-=1;

}*/

/*for(int i=0;i<Gnew->numVertices+1;i++)
        cout<<Gnew->edgeListPtrs[i]<<" ";
cout<<endl;
for(int i=0;i<Gnew->edgeListPtrs[Gnew->numVertices];i++)
        cout<<Gnew->edgeList[i].head<<" "<<Gnew->edgeList[i].tail<<endl;*/
//Gnew->edgeList=Gnew->edgeList1;
	/*
	thrust::host_vector<int> h_vec((*dev1_community).g.nb_nodes+1);
	int *stat=new int[(*dev1_community).g.nb_nodes+1];
	
	unsigned int* edge1=new unsigned int[(*dev1_community).g.nb_links];
	
	std::copy(((*dev1_community).g.indices).begin(),((*dev1_community).g.indices).end(), h_vec.begin());
	
	int i=0;
	
	for(int i=0;i<h_vec.size();i++)
	{
		stat[i]=h_vec[i];
	}
	
	thrust::host_vector<unsigned int> h_vec1((*dev1_community).g.nb_links);
//	cout<<"links"<<((*dev1_community).g.nb_links)<<endl;
//	cout<<"no of links"<<((*dev1_community).g.links).size()<<endl;

	std::copy(((*dev1_community).g.links).begin(),((*dev1_community).g.links).end(), h_vec1.begin());
	 i=0;
        
//ofstream d("d.txt");
for(int i=0;i<h_vec1.size();i++)
{
	edge1[i]=h_vec1[i];
//	d<<edge1[i]<<" ";
}



	GraphHOST modified_graph;
	modified_graph.nb_nodes = (*dev1_community).g.nb_nodes;
	modified_graph.degrees.resize(modified_graph.nb_nodes+1);
	std::copy(stat, stat + (modified_graph.nb_nodes), modified_graph.degrees.begin());
	modified_graph.nb_links = (*dev1_community).g.nb_links;
	modified_graph.links.resize(modified_graph.nb_links);
	std::copy(edge1, edge1 + modified_graph.nb_links, modified_graph.links.begin());
	Community *dev11_community;
        Community dev12_community(modified_graph, -1, threshold);
        dev11_community=&dev12_community;

	bool *borcheck=(bool*)malloc(NV1*sizeof(bool));
	for(int i=0;i<NV1;i++)
		borcheck[i]=false;
        cout<<(*dev11_community).g.nb_nodes<<" "<<(*dev11_community).g.nb_links<<endl;
		
	movetogpu(Gnew,G1,dirty,dirtycpu,stat,edge1,dev11_community,c,f);
	
	
	Gnew1= movetocpu(dirtygpu,NV1,statIndices,edges,Gnew,opts,G1,C_orig,borcheck);
	cout<<G2->numVertices<<" "<<G2->numEdges<<endl;
	cout<<Gnew1->numVertices<<" "<<Gnew1->numEdges<<endl;
	
	thrust::host_vector<int> h1_vec((*dev1_community).g.nb_nodes+1);
        int *stat1=new int[(*dev11_community).g.nb_nodes+1];
        unsigned int *edge2=new unsigned int[(*dev11_community).g.nb_links];
        std::copy(((*dev11_community).g.indices).begin(),((*dev11_community).g.indices).end(), h1_vec.begin());
         

        for(int i=0;i<h1_vec.size();i++)
        {
                stat1[i]=h1_vec[i];
        }

        thrust::host_vector<unsigned int> h1_vec1((*dev11_community).g.nb_links);
        std::copy(((*dev11_community).g.links).begin(),((*dev11_community).g.links).end(), h1_vec1.begin());
         i=0;

//ofstream d("d.txt");
	for(int i=0;i<h1_vec1.size();i++)
	{
        edge2[i]=h1_vec1[i];
//	d<<edge2[i]<<" ";
	}
	
	
	GraphHOST modified1_graph;
        modified1_graph.nb_nodes = (*dev11_community).g.nb_nodes;
        modified1_graph.degrees.resize(modified1_graph.nb_nodes);
        std::copy(stat, stat + (modified1_graph.nb_nodes), modified1_graph.degrees.begin());
        modified1_graph.nb_links = (*dev11_community).g.nb_links;
        modified1_graph.links.resize(modified1_graph.nb_links);
        std::copy(edge2, edge2 + modified1_graph.nb_links, modified1_graph.links.begin());
	ofstream b("b.txt");
	for(int i=0;i<(*dev11_community).g.nb_links;i++)
		b<<edge2[i]<<" ";	

        Community *devf_community;
        Community dev1f_community(modified1_graph, -1, threshold);
        devf_community=&dev1f_community;

 	cout<<"new gpu"<<(*devf_community).g.nb_nodes<<" "<<(*devf_community).g.nb_links<<endl;
	
	movefinal(Gnew1,C_orig,G2,statIndices,edge2,devf_community, c,borcheck);
	std::cout<<"Everything is done!!!!"<<std::endl;

*/
//free(borcheck);	
//free(statIndices);
//free(edges); 
//free(dirtycpu);
//free(dirtygpu);
// free(G1); 
  return 0;
}//End of main()
