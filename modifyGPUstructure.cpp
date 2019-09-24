/*

    Copyright (C) 2016, University of Bergen

    This file is part of Rundemanen - CUDA C++ parallel program for
    community detection

    Rundemanen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rundemanen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rundemanen.  If not, see <http://www.gnu.org/licenses/>.
    
    */
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

void modifyGPUstructure(Community *dev1_community,unsigned int *statIndices,unsigned int*edges,bool *dirtyg,unsigned int *c,int total,int NV,move1* mo,graph *Gn,unsigned int mid)
{
unsigned int newV1=0;
unsigned int edgec1=0;
for(int i=0;i<total-NV;i++)
{
        if(dirtyg[i]){
        newV1++; 
}

}
cout<<"dirty1"<<" "<<newV1<<endl;
int k,jj;
/*unsigned int *edgeP=(unsigned int*)malloc(sizeof(unsigned int)*(newV1+(*dev1_community).g.nb_nodes));
for(unsigned int i=0;i<(*dev1_community).g.nb_nodes;i++)
        edgeP[i]=statIndices[i];
*/
 k=0;jj=0;
thrust::host_vector<unsigned int> h_vec((*dev1_community).g.nb_nodes);
unsigned int *stat=new unsigned int[(*dev1_community).g.nb_nodes];
cout<<"stat before editting"<<endl;
std::copy(((*dev1_community).g.indices).begin(),((*dev1_community).g.indices).end(), h_vec.begin());
	int i=0;
	for(int i=0;i<h_vec.size();i++)
	{
		stat[i]=h_vec[i+1];
		cout<<stat[i]<<" ";
	}
cout<<endl;
unsigned int* edge1=new unsigned int[(*dev1_community).g.nb_links];
thrust::host_vector<unsigned int> h_vec1((*dev1_community).g.nb_links);
std::copy(((*dev1_community).g.links).begin(),((*dev1_community).g.links).end(), h_vec1.begin());

thrust::host_vector<unsigned int> h_vec2((*dev1_community).g.nb_links);

float * weight1=new float[(*dev1_community).g.nb_links];
std::copy(((*dev1_community).g.weights).begin(),((*dev1_community).g.weights).end(), h_vec2.begin());

for(int i=0;i<h_vec1.size();i++)
{
	edge1[i]=h_vec1[i];
	cout<<edge1[i]<<endl;
}
for(int i=0;i<h_vec1.size();i++)
{
        weight1[i]=h_vec2[i];
}
//return;
//cout<<"okk"<<endl;
//vector<int> edge2;
unsigned int edgestoadd;
unsigned    int *stat1=new unsigned int[(*dev1_community).g.nb_nodes+newV1];
	for(int i=0;i<(*dev1_community).g.nb_nodes;i++){
		if(i!=0)
		stat1[i]=stat[i]-stat[i-1];
		else if(i==0)
		stat1[i]=stat[i];
	cout<<"stat1" << " "<<stat1[i]<<endl;
		}

for(int i=(*dev1_community).g.nb_nodes;i<(*dev1_community).g.nb_nodes+newV1;i++)	
		stat1[i]=0;
thrust::host_vector<float> w((*dev1_community).g.nb_links);
int k1=0;int k2=0;
std::copy((*dev1_community).g.weights.begin(),(*dev1_community).g.weights.end(),w.begin());
//cout<<"weight done"<<endl;

int s=0;
int count=0;
unsigned int adj1,adj2;
//unsigned int* edge2=new unsigned int[(*dev1_community).g.nb_links+edgestoadd];
//float* weight2=new float[(*dev1_community).g.nb_links+edgestoadd];
cout<<"Before removal of dirty vertices"<<endl;
/*for(int i=0;i<(*dev1_community).g.nb_links;i++)
{
        edge2[i]=h_vec1[i];
        weight2[i]=weight1[i];
 
}
for(int i=(*dev1_community).g.nb_links;i<(*dev1_community).g.nb_links+edgestoadd;i++)
{
        edge2[i]=0;
        weight2[i]=0;
}
*/
for(unsigned int i=0;i<total-NV;i++)
{

        if(dirtyg[i]==true)
        {
		if(i!=0){
                 adj1=statIndices[i-1];
                 adj2=statIndices[i];}
		else if(i==0)
			{

			adj1=0;
			adj2=statIndices[i];
			}
		unsigned int *f1=(unsigned int*)malloc((*dev1_community).g.nb_nodes*sizeof(unsigned int));
		for(unsigned int i1=0;i1<(*dev1_community).g.nb_nodes;i1++)
			f1[i1]=0;
                for(unsigned int j=adj1;j<adj2;j++)
                {
			if(f1[c[edges[j]]]==0)
			{	
                        k2++;
			f1[c[edges[j]]]+=1;
			}
			stat1[(*dev1_community).g.nb_nodes+s]+=1;
			stat1[c[edges[j]]]+=1;
			
				}
	//	cout<<"check"<<endl;
	//	if(c[i]!=(*dev1_community).g.nb_nodes-1){	
		
		unsigned int adj3,adj4;
				if(c[i]>0){
				adj3=stat1[c[i]-1];
				 adj4=stat1[c[i]];}
				else if(c[i]==0)
				{
					adj3=0;
					adj4=stat[c[i]];
				}
		for(unsigned int j=adj3;j<adj4;j++)
			{
			//	stat1[(*dev1_community).g.nb_nodes+s]+=1;
                       // stat1[c[edges[j]]]+=1;

				k1++;
			}
/*		else if(flag[edges[j]]>0)
			w.at(k1);
		stat[c[i]]-=1;
		stat[c[edges[i]]]-=1;*/
		
          //      }
               // unsigned int adj3=statIndices[c[i]];
               // unsigned int adj4=statIndices[c[i]+1];

// stat1[(*dev1_community).g.nb_nodes+jj]=k2;
  //    jj++;


	count++;
	//cout<<"c="<<count<<endl;
	s++;
        }
	//s++;
}
cout<<"first phase done"<<endl;

 for(int i=0;i<(*dev1_community).g.nb_nodes;i++){
               
        cout<<"stat12" << " "<<stat1[i]<<endl;
                }
edgestoadd=2*k2;
cout<<"edges to add="<<edgestoadd<<endl;

unsigned int* edge2=new unsigned int[(*dev1_community).g.nb_links+edgestoadd];
float* weight2=new float[(*dev1_community).g.nb_links+edgestoadd];
cout<<"Before removal of dirty vertices"<<endl;
for(int i=0;i<(*dev1_community).g.nb_links;i++)
{
        edge2[i]=h_vec1[i];
	weight2[i]=weight1[i];	
	cout<<edge2[i]<<" "<<weight2[i]<<endl;	
}
for(int i=(*dev1_community).g.nb_links;i<(*dev1_community).g.nb_links+edgestoadd;i++)
{
        edge2[i]=0;
        weight2[i]=0;
}
/*
cout<<"After dirty1 removal graph structure"<<endl;
for(int i=0;i<(*dev1_community).g.nb_links+edgestoadd;i++)
{
  
    cout<<edge2[i]<<" "<<weight2[i]<<endl;
}
*/
 jj=0;

vector<unsigned int> pos;
pos.resize((*dev1_community).g.nb_nodes+newV1+1);
std::fill(pos.begin(), pos.begin()+pos.size(), 0);
GraphHOST modified_graph;
unsigned int gpunodes = (*dev1_community).g.nb_nodes+newV1;
cout<<"initial vertices="<<(*dev1_community).g.nb_nodes<<endl;
unsigned int *flag=(unsigned int *)malloc(sizeof(unsigned int)*(*dev1_community).g.nb_nodes);
jj=0;int ii=0;
//return;
//unsigned int adj1,adj2;
for(unsigned int i=0;i<total-NV;i++)
{
        if(dirtyg[i]==true)
        {
		if(i==0)
		{
		adj1=0;
		adj2=statIndices[i];
		}
		else if(i!=0){
		adj1=statIndices[i-1];
		adj2=statIndices[i];}
	for(unsigned int i1=0;i1<(*dev1_community).g.nb_nodes;i1++)
		flag[i1]=0;
//	unsigned int adj1=statIndices[i];
  //      unsigned int adj2=statIndices[i+1];
                for(unsigned int j=adj1;j<adj2;j++)
                {
//	cout<<"value of ii"<<" "<<ii<<endl;
//cout<<a<<" "<<b<<endl;	
	unsigned int a=ii+(*dev1_community).g.nb_nodes;
	unsigned int b=c[edges[j]];
	//cout<<a<<" "<<b<<endl;
//		cout<<b<<endl;
		if(flag[c[edges[j]]]==0)
		{
		
		edge2[(*dev1_community).g.nb_links+jj]=a;
		weight2[(*dev1_community).g.nb_links+jj]=1;
	//	cout<<a<<" "<<jj<<endl;
		pos.at(a)=jj;
		jj++;
		edge2[(*dev1_community).g.nb_links+jj]=b;
		weight2[(*dev1_community).g.nb_links+jj]=1;
		//cout<<b<<" "<<jj<<endl;
		pos.at(b)=jj;
		
		flag[c[edges[j]]]+=1;
		 jj++;
		}
	else if(flag[c[edges[j]]]>0)
		{
		weight2[(*dev1_community).g.nb_links+pos.at(b)]+=1;
		weight2[(*dev1_community).g.nb_links+pos.at(a)]+=1;
		}
		//	stat1[(*dev1_community).g.nb_nodes+ii]+=1;
                  //      stat1[c[edges[j]]]+=1;


		}

		unsigned int adj3,adj4;
                                if(c[i]>0){
                                adj3=stat1[c[i]-1];
                                 adj4=stat1[c[i]];}
                                else if(c[i]==0)
                                {
                                        adj3=0;
                                        adj4=stat[c[i]];
                                }
                for(unsigned int j=adj3;j<adj4;j++)
                        {
				if(weight2[edges[j]]>=1)
				weight2[edges[j]]-=1;
		if(weight2[i]>=1)
                weight2[i]-=1;
                        }




	


	ii++;
	}


}
//return;
cout<<"total no of edges= "<<(*dev1_community).g.nb_links+edgestoadd<<endl;
cout<<"After dirty1 removal graph structure"<<endl;
for(int i=0;i<(*dev1_community).g.nb_links+edgestoadd;i++)
{
  
    cout<<edge2[i]<<" "<<weight2[i]<<endl;
}


cout<<"second phase done"<<endl;
cout<<"count of incoming vertices"<<" "<<mo->vertex<<endl;
modified_graph.nb_nodes=mo->vertex+gpunodes;
unsigned int*stat2=(unsigned int*)malloc(sizeof(unsigned int)*modified_graph.nb_nodes);
for(unsigned int i=0;i<modified_graph.nb_nodes;i++)
	stat2[i]=0;
modified_graph.degrees.resize(modified_graph.nb_nodes+1);


for(unsigned int i=1;i<gpunodes;i++)
{  
	stat1[i]+=stat1[i-1];
	cout<<stat2[i]<<" ";
	stat2[i]=stat1[i];
}
stat2[0]=stat1[0];
cout<<endl;
//free(stat1);
 s=0;

for(unsigned int i=gpunodes;i<modified_graph.nb_nodes;i++)
{
//	cout<<mo->statIndices[s]<<endl;
	stat2[i]=stat2[i-1]+mo->statIndices[s];
//	cout<<stat2[i]<<endl;
	s++;

}

cout<<"check final statIndices"<<endl;
for(int i=0;i<modified_graph.nb_nodes;i++)
	cout<<stat2[i]<<" ";
cout<<endl;

std::copy(stat2, stat2 + (modified_graph.nb_nodes), modified_graph.degrees.begin());

//free(stat2);
cout<<endl;
modified_graph.nb_links = (*dev1_community).g.nb_links+2*k2+mo->edg;
//modified_graph.nb_links=stat2[modified_graph.nb_nodes-1];
cout<<modified_graph.nb_links<<endl;

modified_graph.links.resize(modified_graph.nb_links);
unsigned int *edge3=(unsigned int *)malloc(sizeof(unsigned int)*modified_graph.nb_links);
for(unsigned int i=0;i<(*dev1_community).g.nb_links+2*k2;i++){
	edge3[i]=edge2[i];
}
//cout<<"edge transfer"<<" "<<mo->edg<<endl;
//free(edge2);
	
int j=0;
if(mo->edg>0){
for(int i=(*dev1_community).g.nb_links+2*k2;i<modified_graph.nb_links;i++)	{	
		edge3[i]=j+gpunodes;
		if(mo->edgesa[j]>NV)
		edge3[i+1]=c[mo->edgesa[j]];
		else
		edge3[i+1]=mo->edgesa[j];			
j++;
}
}



std::copy(edge3, edge3 + modified_graph.nb_links, modified_graph.links.begin());
cout<<"done"<<endl;
/*for(int i=0;i<modified_graph.nb_links+1;i++)
{	cout<<edge2[i]<<" ";
//	cout<<weight2[i]<<" ";
}*/
//cout<<"OK"<<endl;
//free(edge3);
//cout<<"done"<<endl;
modified_graph.weights.resize(modified_graph.nb_links);
unsigned int *weight3=(unsigned int *)malloc(sizeof(unsigned int)*modified_graph.nb_links);
for(unsigned int i=0;i<(*dev1_community).g.nb_links+2*k2;i++){
        weight3[i]=weight2[i];
}
//free(weight2);

j=0;
for(int i=(*dev1_community).g.nb_links+2*k2;i<modified_graph.nb_links;i++)      {
                weight3[i]=mo->weighta[j];
    //            if(mo->edgesa[j]>NV)
      //          edges3[i+1]=c[mo->edgesa[j]];
        //        else
          //      edges3[i+1]=mo->edgesa[j];
j++;
}

std::copy(weight3, weight3 +modified_graph.nb_links, modified_graph.weights.begin());
/*for(int i=0;i<modified_graph.nb_links;i++)
	modified_graph.weights.at(i)=weight2[i];
	cout<<weight2[i]<<endl;
cout<<"OkK"<<endl;*/
//free(weight3);

cout<<"generated graph"<<endl;
for(int i=0;i<modified_graph.nb_nodes;i++)
	cout<<stat2[i]<<" ";
for(int i=0;i<modified_graph.nb_links;i++)
	{
	cout<<edge3[i]<<" "<<weight3[i]<<endl;
	}

Community *dev11_community;
Community dev12_community(modified_graph, -1, 0.0001);
dev11_community=&dev12_community;

double cur_mod = -1.0;
//bull isGauss=true;
bool TEPS = true;
//return;
	bool islastRound = false;
	int szSmallComm = 100000;
cudaStream_t *streams = NULL;
	int n_streams = 8;

//	cudaEvent_t start, stop;
//	cudaEventCreate(&start);
//	cudaEventCreate(&stop);
	bool isGauss=true;
/*cur_mod = (*dev11_community).one_levelGaussSeidel(cur_mod, true,
				szSmallComm ,0.001, isGauss &&((*dev11_community).community_size > szSmallComm),
				streams, n_streams, start, stop);*/
cout<<"modified graph has"<<" "<<(*dev11_community).g.nb_nodes<<" "<<modified_graph.nb_nodes<<endl;
unsigned int*d=(unsigned int*)malloc(sizeof(unsigned int)*(modified_graph.nb_nodes));
bool *dirty1=(bool *)malloc(sizeof(bool)*(modified_graph.nb_nodes));
bool *dirty2=(bool *)malloc(sizeof(bool)*(modified_graph.nb_nodes));

for(int i=0;i<modified_graph.nb_nodes;i++)
{	d[i]=-1;
	dirty1[i]=false;
	dirty2[i]=false;

}
cout<<"cur_mod="<<cur_mod<<endl;
//graph *Gn;unsigned int mid;

int a=gpuonly(modified_graph,d,stat2,edge3,dev11_community,dirty1,dirty2,1,Gn,mid);
free(stat);
free(edge1);
free(weight1);
free(stat2);
//free(edge3);
free(weight3);
pos.clear();
//free(flag);
free(edge2);
free(weight2);
}
