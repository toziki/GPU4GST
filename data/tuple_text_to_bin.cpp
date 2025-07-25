/*
 * Copyright 2016 The George Washington University
 * Written by Hang Liu 
 * Directed by Prof. Howie Huang
 *
 * https://www.seas.gwu.edu/~howie/
 * Contact: iheartgraph@gmail.com
 *
 * 
 * Please cite the following paper:
 * 
 * Hang Liu and H. Howie Huang. 2015. Enterprise: breadth-first graph traversal on GPUs. In Proceedings of the International Conference for High Performance Computing, Networking, Storage and Analysis (SC '15). ACM, New York, NY, USA, Article 68 , 12 pages. DOI: http://dx.doi.org/10.1145/2807591.2807594
 
 *
 * This file is part of Enterprise.
 *
 * Enterprise is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Enterprise is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Enterprise.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include<fstream>
#define INFTY int(1<<30)
using namespace std;

typedef long int vertex_t;
typedef long int index_t;

inline off_t fsize(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

		
int main(int argc, char** argv){
	int fd,i;
	char* ss_head;
	char* ss;
	
	std::cout<<"Input: ./exe tuple_file(text) "
			<<"reverse_the_edge(1 reverse, 0 not reverse) lines_to_skip\n";
	if(argc<4){printf("Wrong input\n");exit(-1);}
	
	size_t file_size = fsize(argv[1]);
	bool is_reverse=(atol(argv[2])==1);	
	long skip_head=atol(argv[3]);
	

	fd=open(argv[1],O_CREAT|O_RDWR,00666 );
	if(fd == -1)
	{
		printf("%s open error\n", argv[1]);
		perror("open");
		exit(-1);
	}

	ss_head = (char*)mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
	assert(ss_head != MAP_FAILED);
	size_t head_offset=0;
	int skip_count = 0;
	while(true)
	{
		if(skip_count == skip_head) break;
		if(head_offset == file_size &&
				skip_count < skip_head)
		{
			std::cout<<"Eorr: skip more lines than the file has\n\n\n";
			exit(-1);
		}

		head_offset++;
		if(ss_head[head_offset]=='\n')
		{
			head_offset++;
			skip_count++;
			if(skip_count == skip_head) break;
		}
	}

	ss = &ss_head[head_offset];
	file_size -= head_offset;

	size_t curr=0;
	size_t next=0;

	//step 1. vert_count,edge_count,
	size_t edge_count=0;
	size_t vert_count;
	vertex_t v_max = 0;
	vertex_t v_min = INFTY;//as infinity
	vertex_t w_max = 0;
	vertex_t w_min = INFTY;//as infinity
	vertex_t a;

	//int reg = 0;
	while(next<file_size){
		char* sss=ss+curr;
		a = atol(sss);
		//std::cout<<a<<"\n";
		//if(reg ++ > 10) break;

		if(v_max<a){
			v_max = a;
		}
		if(v_min>a){
			v_min = a;
		}

		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;//是数字的话不断前进
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;//是空格不断前进
		}
		curr = next;
		
		//one vertex is counted once
		edge_count++;


		char* sss1=ss+curr;
		a = atol(sss1);
		//std::cout<<a<<"\n";
		//if(reg ++ > 10) break;
if(v_max<a){
			v_max = a;
		}
		if(v_min>a){
			v_min = a;
		}
		
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;//是数字的话不断前进
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;//是空格不断前进
		}
		curr = next;
		
		//one vertex is counted once
		edge_count++;

char* sss2=ss+curr;
		a = atol(sss2);
		//std::cout<<a<<"\n";
		//if(reg ++ > 10) break;

		if(w_max<a){
			w_max = a;
		}
		if(w_min>a){
			w_min = a;
		}


		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;//是数字的话不断前进
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;//是空格不断前进
		}
		curr = next;
		
		//one vertex is counted once
		

	}
	
	const index_t line_count=edge_count>>1;//上面是统计出了点数 除以二才是行数
	if(!is_reverse) edge_count >>=1;//如果是无向图那一条边就代表两条了 有向图要除以二
	
	vert_count = v_max - v_min + 1;
	assert(v_min<INFTY);
	cout<<"edge count: "<<edge_count<<endl;
	cout<<"max vertex id: "<<v_max<<endl;
	cout<<"min vertex id: "<<v_min<<endl;
	cout<<"max weight "<<w_max<<endl;
	cout<<"min weight "<<w_min<<endl;
	cout<<"edge count: "<<edge_count<<endl;
	cout<<"vert count: "<<vert_count<<endl;
	
	//step 2. each file size
	char filename[256];
	sprintf(filename,"%s_csr.bin",argv[1]);
	int fd4 = open(filename,O_CREAT|O_RDWR,00666 );
	ftruncate(fd4, edge_count*sizeof(vertex_t));
	vertex_t* adj = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd4,0);
	assert(adj != MAP_FAILED);

	//added by Hang to generate a weight file
	sprintf(filename,"%s_weight.bin",argv[1]);
	int fd6 = open(filename,O_CREAT|O_RDWR,00666 );
	ftruncate(fd6, edge_count*sizeof(vertex_t));
	index_t* weight= (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd6,0);
	assert(weight != MAP_FAILED);
	//-End 

	sprintf(filename,"%s_head.bin",argv[1]);
	int fd5 = open(filename,O_CREAT|O_RDWR,00666 );
	ftruncate(fd5, edge_count*sizeof(vertex_t));
	vertex_t* head = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd5,0);
	assert(head != MAP_FAILED);

	sprintf(filename,"%s_deg.bin",argv[1]);
	int fd2 = open(filename,O_CREAT|O_RDWR,00666 );
	ftruncate(fd2, vert_count*sizeof(index_t));
	index_t* degree = (index_t*)mmap(NULL,vert_count*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd2,0);
	assert(degree != MAP_FAILED);
	
	sprintf(filename,"%s_beg_pos.bin",argv[1]);
	int fd3 = open(filename,O_CREAT|O_RDWR,00666 );
	ftruncate(fd3, (vert_count+1)*sizeof(index_t));
	index_t* begin  = (index_t*)mmap(NULL,(vert_count+1)*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd3,0);
	assert(begin != MAP_FAILED);
		ofstream outputFile;
	outputFile.precision(8);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	string st = filename; 
	outputFile.open("/home/sunyahui/lijiayu/Enterprise-master/tuple_text_to_binary_csr/"+st + ".deg");
	//step 3. write degree
	for(int i=0; i<vert_count;i++){
		degree[i]=0;
	}

	vertex_t index, dest;
	size_t offset =0;
	curr=0;
	next=0;

	printf("Getting degree...\n");
	while(offset<line_count){
		char* sss=ss+curr;
		index = atol(sss)-v_min;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;

		char* sss1=ss+curr;
		dest=atol(sss1)-v_min;

		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;
		degree[index]++;
		if(is_reverse) degree[dest]++;

		char* sss2=ss+curr;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;

//		cout<<index<<" "<<degree[index]<<endl;

		offset++;
	}
//	exit(-1);
	begin[0]=0;
	begin[vert_count]=edge_count;

	printf("Calculate beg_pos ...\n");
	for(size_t i=1; i<vert_count; i++){
		begin[i] = begin[i-1] + degree[i-1];
//		cout<<begin[i]<<" "<<degree[i]<<endl;
		degree [i-1] = 0;
	}
	degree[vert_count-1] = 0;
	//step 4: write adjacent list 
	vertex_t v_id;
	offset =0;
	next = 0;
	curr = 0;
	int cost;
	printf("Constructing CSR...\n");
	while(offset<line_count){
		char* sss=ss+curr;
		index = atol(sss)-v_min;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;

		char* sss1=ss+curr;
		v_id = atol(sss1)-v_min;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;
		char* sss2=ss+curr;
		cost = atol(sss2);

		adj[begin[index]+degree[index]] = v_id;
		if(is_reverse) adj[begin[v_id]+degree[v_id]] = index;
		
		weight[begin[index]+degree[index]] = cost;
		if(is_reverse)
			weight[begin[v_id]+degree[v_id]] = cost;
		//-End
	
		head[begin[index]+degree[index]]= index;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;
		degree[index]++;
		if(is_reverse) degree[v_id]++;

		offset++;
	}
	
	//step 5
	//print output as a test
//	for(size_t i=0; i<vert_count; i++){
	for(size_t i=0; i<(vert_count<8?vert_count:8); i++){
		cout<<i<<" "<<begin[i]<<" "<<begin[i+1]<<" ";
		for(index_t j=begin[i]; j<begin[i+1]&&j<begin[i]+10; j++){
			cout<<adj[j]<<"-"<<weight[j]<<" ";
		}
//		if(degree[i]>0){
			cout<<endl;
//		}
	}

//	for(int i=0; i<edge_count; i++){
//	for(int i=0; i<64; i++){
//		cout<<degree[i]<<endl;
//	}

	for (size_t i = 0; i < vert_count; i++)
	{
		outputFile<<i<<" "<<degree[i]<<endl;
	}
	
	munmap( ss,sizeof(char)*file_size );
	
	//-Added by Hang
	munmap( weight,sizeof(vertex_t)*edge_count );
	//-End

	munmap( adj,sizeof(vertex_t)*edge_count );
	munmap( head,sizeof(vertex_t)*edge_count );
	munmap( begin,sizeof(index_t)*vert_count+1 );
	munmap( degree,sizeof(index_t)*vert_count );
	close(fd2);
	close(fd3);
	close(fd4);
	close(fd5);
	
	//-Added by Hang
	close(fd6);
	//-End
}
