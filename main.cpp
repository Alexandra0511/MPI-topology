#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <bits/stdc++.h>

#include "pthread.h"

using namespace std;
#define NUM_OF_COORDS 4
#define MIN(a,b) (((a)<(b))?(a):(b))

void printTopology(vector<int> contor, vector<int> children, vector<int> order){
    string top;
    map<int, vector<int>> workeri;
    int idx = 0;
    for(int i = 0; i < 4; i++) {
        vector<int> aux;
        for(int j = 0; j < contor[order[i]] ; j++) {
            aux.push_back(children[idx + j]);
        }
        workeri[order[i]] = aux;
        idx += contor[order[i]];
    }
    for(auto x: workeri)
    {
        cout<<x.first<<":";
        for(int i = 0; i < contor[x.first] - 1; i++){
            cout<<x.second[i]<<",";
        }
        cout<<x.second[contor[x.first]-1]<<" ";
    }
}

int findKey(map<int, int> order, int val) {
    for(auto &it : order) { 
        if(it.second == val) { 
            return(it.first); 
        } 
    } 
    return -1;
}

void sendPartOfVector(int num, vector<int> workers, int size, int numWorkers, vector<int> numbers, int rank) {
    for(int i = 0; i < num; i++){
        vector<int> part;
        int ID = workers[i] - NUM_OF_COORDS;
        int start =  ID * (double) size / numWorkers;
        int end = MIN((ID + 1) * (double)size / numWorkers, size);
        int dim = end - start;
        for(int i = start ; i < end; i++) {
            part.push_back(numbers[i]);
        }
        MPI_Send(&dim, 1, MPI_INT, workers[i], 2, MPI_COMM_WORLD);
        MPI_Send(&part[0], dim, MPI_INT, workers[i], 2, MPI_COMM_WORLD);
        cout<<"M(" <<rank<<","<< workers[i]<<")"<<endl;
    }
}

int main (int argc, char *argv[])
{
    int  numtasks, rank, num;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    vector<int> workers;
    MPI_Status status;
    int numWorkers = numtasks - NUM_OF_COORDS;
    int lastElemOfRing;
    // vector care contine aranjarea worker-ilor in functie de coordonator in 
    // vectorul topologiei finale
    vector<int> places;
    map<int, int> order1 = {{0, 1}, {1, 2}, {2, 3}};
    map<int, int> order2 = {{0, 3}, {3, 2}, {2, 1}};
    map<int, int> ord;
    if (stoi(argv[2]) == 1){
        ord = order2;
        lastElemOfRing = 1;
        places = {0, 3, 2, 1};
    }
    else {
        ord = order1;
        lastElemOfRing = 3;
        places = {0, 1, 2, 3};
    }
  
    // coordonatorii citesc fisierele si isi creaza vectorii de workeri
    if(rank <= 3){
        ifstream myfile;
        myfile.open("cluster" + to_string(rank)+ ".txt");
        string myline;
        getline(myfile, myline);
        num = stoi(myline);
       
        if (myfile.is_open()) {
            for(int i = 0; i<num; i++)
            {
                getline(myfile, myline);
                workers.push_back(stoi(myline));
            }
        }
    }

    // Procesul 0 porneste ciclul
    if (rank == 0) {
        // initializeaza vectorul ce contine numarul copiilor fiecarui cluster
        vector<int> contor ;
        for(int i=0; i<=3; i++){
            contor.push_back(0);
        }
        contor[0] = num;
        
        // trimite vectorul de contor si vectorul de copii urmatorului cluster
        MPI_Send(&contor[0], NUM_OF_COORDS, MPI_INT, ord[rank], 0, MPI_COMM_WORLD);
        MPI_Send(&workers[0], num, MPI_INT, ord[rank], 0, MPI_COMM_WORLD);
        cout<<"M(0," << ord[rank]<<")"<<endl;
        
        // primeste vectorii corespunzatori topologiei finale de la coordonatorul anterior
        vector<int> ct_recv(NUM_OF_COORDS);
        vector<int> recv_children;

        MPI_Recv(&ct_recv[0], NUM_OF_COORDS, MPI_INT, ord[rank], 1, MPI_COMM_WORLD, &status);
        recv_children.resize(numWorkers);
        MPI_Recv(&recv_children[0], numWorkers, MPI_INT, ord[rank], 1, MPI_COMM_WORLD, &status);
        
        // proceseaza vectorii topologiei si face afisarea
        cout<<rank<<" -> ";
        printTopology(ct_recv, recv_children, places);
        cout<<endl;

        // trimite mai departe topologia catre workeri si clusterul urmator
        // informeaza workerii de rankul sau
        for(int i = 0; i<num; i++) {
            MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            MPI_Send(&recv_children[0], numWorkers, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            cout<<"M(0," << workers[i] << ")"<<endl;
        }
    
        MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, ord[rank], 1, MPI_COMM_WORLD);
        MPI_Send(&recv_children[0], numWorkers, MPI_INT, ord[rank], 1, MPI_COMM_WORLD);
        cout<<"M(0,"<<ord[rank]<<")"<<endl;

        // generare vector
        int size = stoi(argv[1]);
        vector<int> numbers;
        for (int i = 0; i < size; i++) {
            numbers.push_back(size-i-1);
        }
        
        // trimit catre workeri partea de vector pe care trebuie sa o prelucreze fiecare
        sendPartOfVector(num, workers, size, numWorkers, numbers, rank);
        
        //trimit vectorul mai departe in topologie
        MPI_Send(&size, 1, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        MPI_Send(&numbers[0], size, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        cout<<"M(0,"<<ord[rank]<<")"<<endl;

        // primesc partile din vector prelucrate de workeri si le reasamblez 
        vector<int> final;
        for(int i = 0; i < num; i++) {
            vector<int> part;
            MPI_Recv(&size, 1, MPI_INT, workers[i], 2, MPI_COMM_WORLD, &status);
            part.resize(size);
            MPI_Recv(&part[0], size, MPI_INT, workers[i], 2, MPI_COMM_WORLD, &status);
            
            for(int j = 0; j < size; j++) {
                final.push_back(part[j]);
            }
        }
        // trimit partea de vector reasamblat mai departe
        size = final.size();
        MPI_Send(&size, 1, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        MPI_Send(&final[0], size, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        cout<<"M(0,"<<ord[rank]<<")"<<endl;

        // primesc vectorul final 
        MPI_Recv(&size, 1, MPI_INT, ord[rank], 3, MPI_COMM_WORLD, &status);
        final.resize(size);
        MPI_Recv(&final[0], size, MPI_INT, ord[rank], 3, MPI_COMM_WORLD, &status);
        
        // vectorul este sortat si afisat
        sort(final.begin(), final.end(), greater<int>());
        cout<<"Rezultat: ";
        for(int i = 0; i < size; i++) {
            cout<<final[i]<<" ";
        }
        cout<<endl;
     

    } else if (rank == lastElemOfRing) {
        // Ultimul proces din inel rezolva topologia
        // primeste topologia creata anterior si o actualizeaza
        vector<int> ct_recv(NUM_OF_COORDS);
        vector<int> recv_children;

        MPI_Recv(&ct_recv[0], NUM_OF_COORDS, MPI_INT, findKey(ord, rank), 0, MPI_COMM_WORLD, &status);
        int newSize = 0;
        for(int i = 0; i<NUM_OF_COORDS; i++) {
            newSize += ct_recv[i];
        }
        recv_children.resize(newSize);
        MPI_Recv(&recv_children[0], newSize, MPI_INT, findKey(ord, rank), 0, MPI_COMM_WORLD, &status);
        
        ct_recv[rank] = num;
        for(int i = 0; i<num; i++){
            recv_children.push_back(workers[i]);
        }
        newSize += num;
        
        // coordonatorul inchide inelul si face afisarea topologiei finale
        cout<<rank<<" -> ";
        printTopology(ct_recv, recv_children, places);
        cout<<endl;

        // trimite topologia finalizata coordonatorului 0 si copiilor sai
        MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, findKey(ord, rank), 1, MPI_COMM_WORLD);
        MPI_Send(&recv_children[0], newSize, MPI_INT, findKey(ord, rank), 1, MPI_COMM_WORLD);
        cout<<"M("<<lastElemOfRing<<","<<findKey(ord, rank)<<")"<<endl;
        for(int i = 0; i<num; i++) {
            MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            MPI_Send(&recv_children[0], newSize, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            cout<<"M("<<lastElemOfRing<<","<< workers[i]<<")"<<endl;
        }

        //task2
        int size, end, quot, rest, start;
        vector<int> numbers;
        MPI_Recv(&size, 1, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        numbers.resize(size);
        MPI_Recv(&numbers[0], size, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        
        // trimit catre workeri partea de vector pe care trebuie sa o prelucreze fiecare
        sendPartOfVector(num, workers, size, numWorkers, numbers, rank);

        // asamblez partile workerilor la vectorul final primit de la coordonatorul anterior
        vector<int> final;
        MPI_Recv(&size, 1, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        final.resize(size);
        MPI_Recv(&final[0], size, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        for(int i = 0; i < num; i++) {
            vector<int> part;
            MPI_Recv(&size, 1, MPI_INT, workers[i], 2, MPI_COMM_WORLD, &status);
            part.resize(size);
            MPI_Recv(&part[0], size, MPI_INT, workers[i], 2, MPI_COMM_WORLD, &status);
            for(int j = 0; j < size; j++) {
                final.push_back(part[j]);
            }
        }

        int dim = final.size();
        MPI_Send(&dim, 1, MPI_INT, findKey(ord, rank), 3, MPI_COMM_WORLD);
        MPI_Send(&final[0], dim, MPI_INT, findKey(ord, rank), 3, MPI_COMM_WORLD);
        cout<<"M("<<lastElemOfRing<<","<<findKey(ord, rank)<<")"<<endl;
        
    } else if (rank < NUM_OF_COORDS){
        // procesele de mijloc actualizeaza vectorii topologiei si trimit mai departe
        vector<int> ct_recv(NUM_OF_COORDS);
        vector<int> recv_children;

        MPI_Recv(&ct_recv[0], NUM_OF_COORDS, MPI_INT, findKey(ord, rank), 0, MPI_COMM_WORLD, &status);
        
        int newSize = 0;
        for(int i = 0; i<NUM_OF_COORDS; i++) {
            newSize += ct_recv[i];
        }
        recv_children.resize(newSize);
        MPI_Recv(&recv_children[0], newSize, MPI_INT, findKey(ord, rank), 0, MPI_COMM_WORLD, &status);
        
        ct_recv[rank] = num;
        for(int i = 0; i<num; i++){
            recv_children.push_back(workers[i]);
        }
        newSize += num;
        MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, ord[rank], 0, MPI_COMM_WORLD);
        MPI_Send(&recv_children[0], newSize, MPI_INT, ord[rank], 0, MPI_COMM_WORLD);
        cout<<"M("<<rank<<","<< ord[rank]<<")"<<endl;

        // primesc vectorii topologiei finale
        MPI_Recv(&ct_recv[0], NUM_OF_COORDS, MPI_INT, ord[rank], 1, MPI_COMM_WORLD, &status);
        recv_children.resize(numWorkers);
        MPI_Recv(&recv_children[0], numWorkers, MPI_INT, ord[rank], 1, MPI_COMM_WORLD, &status);

        // afiseaza topologia
        cout<<rank<<" -> ";
        printTopology(ct_recv, recv_children, places);
        cout<<endl;

        // trimit mai departe topologia catre workeri
        for(int i = 0; i<num; i++) {
            MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            MPI_Send(&recv_children[0], numWorkers, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            cout<<"M("<<rank<<","<< workers[i]<<")"<<endl;
        }
        
        MPI_Send(&ct_recv[0], NUM_OF_COORDS, MPI_INT, findKey(ord, rank), 1, MPI_COMM_WORLD);
        MPI_Send(&recv_children[0], numWorkers, MPI_INT, findKey(ord, rank), 1, MPI_COMM_WORLD);
        cout<<"M("<<rank<<","<< findKey(ord, rank)<<")"<<endl;

        // task2
        int size;
        vector<int> numbers;
        MPI_Recv(&size, 1, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        numbers.resize(size);
        MPI_Recv(&numbers[0], size, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        
        // trimit catre workeri partea de vector pe care trebuie sa o prelucreze fiecare
        sendPartOfVector(num, workers, size, numWorkers, numbers, rank);
        MPI_Send(&size, 1, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        MPI_Send(&numbers[0], size, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        cout<<"M("<<rank<<","<< ord[rank]<<")"<<endl;

        vector<int> final;
        MPI_Recv(&size, 1, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        final.resize(size);
        MPI_Recv(&final[0], size, MPI_INT, findKey(ord, rank), 2, MPI_COMM_WORLD, &status);
        for(int i = 0; i < num; i++) {
            vector<int> part;
            MPI_Recv(&size, 1, MPI_INT, workers[i], 2, MPI_COMM_WORLD, &status);
            part.resize(size);
            MPI_Recv(&part[0], size, MPI_INT, workers[i], 2, MPI_COMM_WORLD, &status);
            for(int j = 0; j < size; j++) {
                final.push_back(part[j]);
            }
        }
        int dim = final.size();
        MPI_Send(&dim, 1, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        MPI_Send(&final[0], dim, MPI_INT, ord[rank], 2, MPI_COMM_WORLD);
        cout<<"M("<<rank<<","<< ord[rank]<<")"<<endl;

        MPI_Recv(&dim, 1, MPI_INT, ord[rank], 3, MPI_COMM_WORLD, &status);
        final.resize(dim);
        MPI_Recv(&final[0], dim, MPI_INT, ord[rank], 3, MPI_COMM_WORLD, &status);
        
        // trimit vectorul cu partile workerilor asamblate inapoi pe calea catre 0
        MPI_Send(&dim, 1, MPI_INT, findKey(ord, rank), 3, MPI_COMM_WORLD);
        MPI_Send(&final[0], dim, MPI_INT, findKey(ord, rank), 3, MPI_COMM_WORLD);
        cout<<"M("<<rank<<","<< findKey(ord, rank)<<")"<<endl;
    } 
    else {
        // workerii primesc topologia de la coordonatorii lor si o afisaza
        int coord;
        vector<int> ct_recv(NUM_OF_COORDS);
        vector<int> recv_children;
        MPI_Recv(&coord, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&ct_recv[0], NUM_OF_COORDS, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);
        recv_children.resize(numWorkers);

        MPI_Recv(&recv_children[0], numWorkers, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);
        cout<<rank<<" -> ";
        printTopology(ct_recv, recv_children, places);
        cout<<endl;

        // task 2
        // primesc partea corespunzatoare din vector 
        int size, start, end;
        vector<int> numbers;
        MPI_Recv(&size, 1, MPI_INT, coord, 2, MPI_COMM_WORLD, &status);
        numbers.resize(size);
        MPI_Recv(&numbers[0], size, MPI_INT, coord, 2, MPI_COMM_WORLD, &status);
        
        // prelucreaza portiunea din vector
        for(int i = 0; i < size; i++) 
        {
            numbers[i] = numbers[i] * 5;
        }
        
        // trimit inapoi catre coordonator partea din vector rezolvata 
        MPI_Send(&size, 1, MPI_INT, coord, 2, MPI_COMM_WORLD);
        MPI_Send(&numbers[0], size, MPI_INT, coord, 2, MPI_COMM_WORLD);
        cout<<"M("<<rank<<","<< coord<<")"<<endl;

    }
   
    MPI_Finalize();

}

