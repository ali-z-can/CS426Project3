//
// Created by alican on 22.04.2019.
//

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include <math.h>
#include <sys/time.h>

void create_histogram(int * hist, int ** img, int num_rows, int num_cols);
double distance(int *a, int *b, int size);
int find_closest(int ***training_set, int num_persons, int num_training, int size, int *test_image);
int main(int argc, char **argv){



    const int h = 200;
    const int w = 180;
    const int people = 18;
    const int imagecount = 20;
    const int size = 255;
    int k,forcounter,forcounter2;
    k = atoi(argv[1]);
    int** arr;
    int *hist;

    int ***trainingset;
    //read images and create their histograms
    char fn[10];

    trainingset = (int***)malloc(people * sizeof(int**));
    for(forcounter = 0; forcounter<people;forcounter++){
        trainingset[forcounter] = (int**)malloc(k * sizeof(int*));

        for(forcounter2 = 0; forcounter2<k;forcounter2++){

            snprintf(fn,10,"%d.%d.txt",forcounter+1,forcounter2+1);
            //fprintf(stderr,"%s\n",fn);

            arr = read_pgm_file(fn,h,w);

            trainingset[forcounter][forcounter2] = (int*)calloc(size, sizeof(int));

            create_histogram(trainingset[forcounter][forcounter2], arr, h, w);

            dealloc_2d_matrix(arr,h,w);



        }

    }


    int result,ecount;
    ecount = 0;
    for(forcounter = 0; forcounter<people;forcounter++){
        for(forcounter2 = k; forcounter2<imagecount;forcounter2++){

            snprintf(fn,10,"%d.%d.txt",forcounter+1,forcounter2+1);
            //fprintf(stderr,"%s\n",fn);
            arr = read_pgm_file(fn,h,w);
            hist = (int*)calloc(size, sizeof(int));
            create_histogram(hist,arr,h,w);
            dealloc_2d_matrix(arr,h,w);
            result = find_closest(trainingset, people, k, size, hist);
            if(result != forcounter+1){
                ecount++;
            }
            fprintf(stderr,"%s %d %d\n",fn,result,forcounter+1);
            free(hist);

        }

    }
    fprintf(stderr,"error count = %d\n",ecount);
    for(forcounter = 0; forcounter<people;forcounter++){
        for(forcounter2 = 0; forcounter2<k;forcounter2++){

            free(trainingset[forcounter][forcounter2]);

        }

        free(trainingset[forcounter]);

    }

    free(trainingset);





}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double distance(int * a, int *b, int size){

    double dist;
    dist = 0;
    int forcounter;

    for(forcounter = 0; forcounter<size; forcounter++){

        if(a[forcounter]+b[forcounter] == 0){

            //do nothing
        }
        else {

            //fprintf(stderr,"a = %d b=%d \n",a[forcounter],b[forcounter] );
            dist += (((double)(a[forcounter] - b[forcounter]) * (double)(a[forcounter] - b[forcounter])) / (double)(a[forcounter] + b[forcounter])) / 2;
            //fprintf(stderr,"dist = %f\n",dist);
        }
    }
    return dist;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void create_histogram(int * hist, int ** img, int num_rows, int num_cols){

    int forcounter,forcounter2,ps;

    for(forcounter = 1; forcounter<num_rows-1;forcounter++){
        ps = 0;
        for(forcounter2 = 1; forcounter2<num_cols-1;forcounter2++){
            //(x,y-1)0 (x+1,y-1)1 (x+1,y)2 (x+1,y+1)3 (x,y+1)4 (x-1,y+1)5 (x-1,y)6 (x-1,y-1)7

            if(forcounter2 == 0 || img[forcounter][forcounter2]<img[forcounter][forcounter2-1]){
                ps += 1;
            }
            if(forcounter2 == 0 || img[forcounter][forcounter2]<img[forcounter+1][forcounter2-1]){
                ps += 2;
            }
            if(forcounter == num_rows || img[forcounter][forcounter2]<img[forcounter+1][forcounter2]){
                ps += 4;

            }
            if(forcounter == num_rows || forcounter2 == num_cols || img[forcounter][forcounter2]<img[forcounter+1][forcounter2+1]){
                ps += 8;

            }
            if(forcounter2 == num_cols || img[forcounter][forcounter2]<img[forcounter][forcounter2+1]){
                ps += 16;

            }
            if(forcounter2 == num_cols || forcounter == 0 || img[forcounter][forcounter2]<img[forcounter-1][forcounter2+1]){
                ps += 32;

            }
            if(forcounter == 0 || img[forcounter][forcounter2]<img[forcounter-1][forcounter2]){
                ps += 64;

            }
            if(forcounter == 0 || forcounter2 == 0 || img[forcounter][forcounter2]<img[forcounter-1][forcounter2-1]){
                ps += 128;

            }


            hist[ps]++;
            ps = 0;

        }


    }


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int find_closest(int ***training_set, int num_persons, int num_training, int size, int *test_image){
    int forCounter,forCounter2;
    double dist,current;
    int person;
    person = 0;
    current = 9999999999999;

    for(forCounter = 0; forCounter<num_persons; forCounter++){

        for(forCounter2 = 0; forCounter2 < num_training; forCounter2++){

            dist = distance(training_set[forCounter][forCounter2],test_image,size);
            //fprintf(stderr,"dist = %f curr = %f\n",dist,current);

            if(dist < current){
                current = dist;
                person = forCounter +1;
            }
        }
    }
    return person;
}



