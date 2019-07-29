//
// Created by alican on 22.04.2019.
//

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include <math.h>
#include <sys/time.h>
#include <omp.h>
#include <sys/time.h>

void create_histogram(int * hist, int ** img, int num_rows, int num_cols);
double distance(int *a, int *b, int size);
int find_closest(int ***training_set, int num_persons, int num_training, int size, int *test_image);
int main(int argc, char **argv){

    fprintf(stderr,"call\n");
    struct timeval sbegin,send,pbegin,pend,ab,ae;
    const int h = 200;
    const int w = 180;
    const int people = 18;
    const int imagecount = 20;
    const int size = 255;
    int k,forcounter,forcounter2;
    k = atoi(argv[1]);
    int** arr;
    int *hist;
    int pti,sti;
    pti = 0;
    sti = 0;

    int ***trainingset;
    //read images and create their histograms
    char fn[10];

    trainingset = (int***)malloc(people * sizeof(int**));
    gettimeofday(&ab,NULL);

//#pragma omp parallel for default(none),private(forcounter2,forcounter,fn,arr),shared(trainingset,k),schedule(static)
    for(forcounter = 0; forcounter<people;forcounter++){
        trainingset[forcounter] = (int**)malloc(k * sizeof(int*));

        for(forcounter2 = 0; forcounter2<k;forcounter2++){

            gettimeofday(&sbegin,NULL);
            snprintf(fn,10,"%d.%d.txt",forcounter+1,forcounter2+1);
            //fprintf(stderr,"%s\n",fn);

            arr = read_pgm_file(fn,h,w);

            trainingset[forcounter][forcounter2] = (int*)calloc(size, sizeof(int));
            gettimeofday(&send,NULL);
            sti += ((send.tv_sec - sbegin.tv_sec)*1000000L + (send.tv_usec - sbegin.tv_usec));
            gettimeofday(&pbegin,NULL);

            create_histogram(trainingset[forcounter][forcounter2], arr, h, w);

            gettimeofday(&pend,NULL);
            pti += ((pend.tv_sec - pbegin.tv_sec)*1000000L + (pend.tv_usec - pbegin.tv_usec));

            dealloc_2d_matrix(arr,h,w);



        }

    }


    int result,ecount;
    ecount = 0;

//#pragma omp parallel for default(none),private(forcounter2,forcounter,fn,arr,hist,result),shared(trainingset,k,ecount),schedule(static)
    for(forcounter = 0; forcounter<people;forcounter++){
        for(forcounter2 = k; forcounter2<imagecount;forcounter2++){

            gettimeofday(&sbegin,NULL);
            snprintf(fn,10,"%d.%d.txt",forcounter+1,forcounter2+1);
            //fprintf(stderr,"%s\n",fn);
            arr = read_pgm_file(fn,h,w);
            hist = (int*)calloc(size, sizeof(int));
            gettimeofday(&send,NULL);
            sti += ((send.tv_sec - sbegin.tv_sec)*1000000L + (send.tv_usec - sbegin.tv_usec));

            gettimeofday(&pbegin,NULL);

            create_histogram(hist,arr,h,w);




            dealloc_2d_matrix(arr,h,w);
            result = find_closest(trainingset, people, k, size, hist);
            gettimeofday(&pend,NULL);
            pti += ((pend.tv_sec - pbegin.tv_sec)*1000000L + (pend.tv_usec - pbegin.tv_usec));

            gettimeofday(&sbegin,NULL);

            if(result != forcounter+1){
                ecount++;
            }
            printf("%s %d %d\n",fn,result,forcounter+1);
            free(hist);
            gettimeofday(&send,NULL);
            sti += ((send.tv_sec - sbegin.tv_sec)*1000000L + (send.tv_usec - sbegin.tv_usec));



        }

    }

    printf("Accuracy: %d correct answers for %d tests\n",((imagecount-k)*people)-ecount,((imagecount-k)*people));
    printf("Parallel time: %ld\nSequential time: %ld\n",pti,sti);
    //fprintf(stderr,"error count = %d\n",ecount);
    for(forcounter = 0; forcounter<people;forcounter++){
        for(forcounter2 = 0; forcounter2<k;forcounter2++){

            free(trainingset[forcounter][forcounter2]);

        }

        free(trainingset[forcounter]);

    }

    free(trainingset);

    gettimeofday(&ae,NULL);
    fprintf(stderr,"total time %ld\n",((ae.tv_sec - ab.tv_sec)*1000000L + (ae.tv_usec - ab.tv_usec)));


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double distance(int * a, int *b, int size){

    double dist;
    dist = 0;
    int forcounter;

//#pragma omp parallel for default(none), private(forcounter),shared(a,b,dist,size), schedule(static)
    for(forcounter = 0; forcounter<size; forcounter++){

        if(a[forcounter]+b[forcounter] == 0){

            //do nothing
        }
        else {

            //#pragma omp critical (name1)
            {
                //fprintf(stderr,"a = %d b=%d \n",a[forcounter],b[forcounter] );
                dist += (((double) (a[forcounter] - b[forcounter]) * (double) (a[forcounter] - b[forcounter])) /
                         (double) (a[forcounter] + b[forcounter])) / 2;
                //fprintf(stderr,"dist = %f\n",dist);
            }
        }
    }
    return dist;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void create_histogram(int * hist, int ** img, int num_rows, int num_cols){

    int forcounter,forcounter2,ps;

    #pragma omp parallel for default(none),private(forcounter2,forcounter,ps),shared(hist,img,num_rows,num_cols),schedule(static)
    for(forcounter = 1; forcounter<num_rows-1;forcounter++){
        ps = 0;
        //#pragma omp parallel for default(none),private(forcounter2,ps),shared(num_cols,num_rows,img,hist,forcounter),schedule(static)
        for(forcounter2 = 1; forcounter2<num_cols-1;forcounter2++){
            //(x,y-1)0 (x+1,y-1)1 (x+1,y)2 (x+1,y+1)3 (x,y+1)4 (x-1,y+1)5 (x-1,y)6 (x-1,y-1)7
/*
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

*/
            ps = 0;

            ps += 1*(forcounter2 == 0 || img[forcounter][forcounter2]<img[forcounter][forcounter2-1]);
            ps += 2*(forcounter == num_rows || forcounter2 == 0 || img[forcounter][forcounter2]<img[forcounter+1][forcounter2-1]);
            ps += 4*(forcounter == num_rows || img[forcounter][forcounter2]<img[forcounter+1][forcounter2]);
            ps += 8* (forcounter == num_rows || forcounter2 == num_cols || img[forcounter][forcounter2]<img[forcounter+1][forcounter2+1]);
            ps += 16*(forcounter2 == num_cols || img[forcounter][forcounter2]<img[forcounter][forcounter2+1]);
            ps += 32*(forcounter2 == num_cols || forcounter == 0 || img[forcounter][forcounter2]<img[forcounter-1][forcounter2+1]);
            ps += 64*(forcounter == 0 || img[forcounter][forcounter2]<img[forcounter-1][forcounter2]);
            ps += 128*(forcounter == 0 || forcounter2 == 0 || img[forcounter][forcounter2]<img[forcounter-1][forcounter2-1]);


            //if(ps>255)
                //printf("wtfhaveudone %d \n",ps);
            //printf("hello from th %d of %d working for %d\n", omp_get_thread_num(),omp_get_num_threads(),img);
            #pragma omp atomic
            hist[ps]++;

            ps = 0;

        }


    }
    //#pragma omp barrier


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int find_closest(int ***training_set, int num_persons, int num_training, int size, int *test_image){
    int forCounter,forCounter2;
    double dist,current;
    int person;
    person = 0;
    current = 9999999999999;


#pragma omp parallel for default(none),private(forCounter2,forCounter,dist),shared(training_set,num_persons,num_training,person,current,size,test_image),schedule(static)
    for(forCounter = 0; forCounter<num_persons; forCounter++){
        for(forCounter2 = 0; forCounter2 < num_training; forCounter2++){

            dist = distance(training_set[forCounter][forCounter2],test_image,size);
            //fprintf(stderr,"dist = %f curr = %f\n",dist,current);
            //printf("hello from th %d of %d working for %d\n", omp_get_thread_num(),omp_get_num_threads(),test_image);


            #pragma omp critical (name1)
            {
            if(dist < current){

                current = dist;
                person = forCounter +1;
                }
            }
        }
    }
//#pragma omp barrier
    return person;
}



