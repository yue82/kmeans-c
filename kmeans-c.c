#include <stdio.h>
#include <stdlib.h>

/* kmeans-c InputFile OutputFile HSize VSize ClusterNum IterationNum */

typedef struct PointData{
  int x;
  int y;
  int class;
}POINT;

FILE *fpi, *fpo;
char *inputfile;
char *outputfile;

int hsize = 0;
int vsize = 0;
int clnum = 1;
int itnum = 0;

int datanum = 0;
int maxnum = 0;

//class num = 0~k-1
int **means;
int *pnum; //num of points in each class

//class num = 1~k
POINT **points;
int **field;


int main(int argc, char *argv[])
{ 
  int ret;
  int x, y;
  int i, j, n;

  if(argc != 7){
    printf("Please write Command like this.\n ==> ./kmeans-c.o InputFile OutputFile HSize VSize ClusterNum IterationNum'\n");
    return 1;
  }

  /*__________INIT___________*/

  inputfile = argv[1];
  outputfile = argv[2];
  
  hsize = atoi(argv[3]);
  vsize = atoi(argv[4]);
  
  maxnum = hsize*vsize/2;
  
  clnum = atoi(argv[5]);
  itnum = atoi(argv[6]);


  //field init
  if((field = (int**)calloc(vsize, sizeof(int*))) == NULL){
    printf("field init error!!\n");
    return -1;
  }  
  for(i = 0; i < vsize; i++){
    if((field[i] = (int*)calloc(hsize, sizeof(int))) == NULL){
      printf("field init error!!\n");
      return -1;
    }
  }

  //points init
  if((points = (POINT**)malloc(sizeof(POINT*) * maxnum)) == NULL){
    printf("points init error!!\n");
    return -1;
  }  

  for(n = 0; n < maxnum; n++){
    if((points[n] = (POINT*)malloc(sizeof(POINT))) == NULL){
      printf("points init error!!\n");
      return -1;
    }  
  }

  //cluster init
  if((means = (int**)calloc(clnum, sizeof(int*))) == NULL){
    printf("cluster init error!!\n");
    return -1;
  }  
  for(i = 0; i < clnum; i++){
    if((means[i] = (int*)calloc(2, sizeof(int))) == NULL){
      printf("cluster init error!!\n");
      return -1;
    }
  }
  if((pnum = (int*)calloc(clnum, sizeof(int))) == NULL){
    printf("cluster init error!!\n");
    return -1;
  }
  
  // init log output
  if ((fpo = fopen(outputfile, "w")) == NULL) {
    printf("output file open error!!\n");
    return -1;
  }
  fprintf(fpo, "input:%s, output:%s, size:%d*%d, cluster:%d, iterate:%d \n", inputfile, outputfile, hsize, vsize,  clnum, itnum);
  fclose(fpo);

  /*__________READ INPUT___________*/
  //read input
  if ((fpi = fopen(inputfile, "r")) == NULL) {
    printf("input file open error!!\n");
    return -1;
  }  
  while((ret = fscanf(fpi, "%d,%d", &x, &y)) != EOF){
    points[datanum]->x = x;
    points[datanum]->y = y;
    points[datanum]->class = 1;
    //printf("%d:(%d, %d):(%d, %d)\n", datanum, x, y, points[datanum]->x, points[datanum]->y); 
    datanum++;
  }  
  
  fclose(fpi);
  
  //release extra points
  for(n = datanum; n < maxnum; n++){
    free(points[n]);
  } 

  /*______ITERATION________*/

  initMeans();

  renewCluster();
  mapField(); 
  showField("init", 0);
  printf("init done.\n");

  for(n = 0; n < itnum; n++){
    calcCluster();
    renewCluster();

    mapField();
    showField("calc", n+1);    
    printf("%d calc done.\n", n+1);
  }


  /*________FINALIZE________*/
  for(i = 0; i < vsize; i++){
    free(field[i]);
  }
  free(field);
  free(pnum);
  for(n = 0; n < datanum; n++){
    free(points[n]);
  }
  free(points);
  for(n = 0; n < 2; n++){
    free(means[n]);
  }
  free(means);

  printf("finish");

  return 0;
}

////////////////////////////////////////////////////

/*_____calc_____*/

// 2Column
int initMeans(){
  int k;
  int cl = clnum;
  
  if(clnum%2 == 1){
    means[clnum-1][0] = hsize/2;
    means[clnum-1][1] = vsize/2;
    cl -= 1;
  }
  
  for(k = 0; k < cl/2; k++){
    //1st column
    means[k][0] = (2*k+1)*hsize/cl;
    means[k][1] = vsize/4;
    
    //2nd colmun
    means[k + cl/2][0] = (2*k+1)*hsize/cl;
    means[k + cl/2][1] = 3*vsize/4;
  }
  /*
  printf("init means=");
  for(k = 0; k < clnum; k++){
    printf("(%d,%d)/%d ", means[k][0], means[k][1], pnum[k]);
  }
  printf("\n");
  */
}

int renewCluster(){
  int k, n;
  int mindist, mincl;
  int dist;
  for(n = 0; n < datanum; n++){    
    mindist = vsize+hsize;//init
    mincl = -1;
    for(k = 0; k < clnum; k++){ 
      dist = calcMDistance(points[n]->x, points[n]->y, k);
       
      if(dist < mindist){
	mindist = dist;
	mincl = k;
      }
    }
    points[n]->class = mincl + 1;
  }
}

//Manhattan Distance
int calcMDistance(int x, int y, int k){
  int distx, disty;
  distx = means[k][0] - x;
  disty = means[k][1] - y;
  if(distx < 0)
    distx = -distx;
  if(disty < 0)
    disty = -disty;
  return(distx+disty);
}
//calc Cluster means and point num
int calcCluster(){
  int m[clnum][2];
  int pn[clnum];
  int class = -1;
  int k, n;
  
  for(k = 0; k < clnum; k++){
    m[k][0] = 0;
    m[k][1] = 0;
    pn[k] = 0;
  }
  for(n = 0; n < datanum; n++){
    class = points[n]->class - 1;
    m[class][0] += points[n]->x;
    m[class][1] += points[n]->y;
    pn[class]++;
  }

  for(k=0; k < clnum; k++){
    field[ means[k][1] ][ means[k][0] ] = 0;//init means in field
    if(pn[k] == 0){
      means[k][0] = hsize-1;
      means[k][1] = vsize-1;
    }else{
      means[k][0] = m[k][0] / pn[k];
      means[k][1] = m[k][1] / pn[k];
    }
    pnum[k] = pn[k];
  }
  return 0;
}

/*_____show_____*/

int mapField(){
  int k, n;
  for(n = 0; n < datanum; n++){
    field[ points[n]->y ][ points[n]->x ] = points[n]->class;
  }
  for(k = 0; k < clnum; k++){
    field[ means[k][1] ][ means[k][0] ] = -1;//means 
 }
  
  return 1;  
}


int showField(char *msg, int it){
  int i, j, k;
  
  //output
  if ((fpo = fopen(outputfile, "a")) == NULL) {
    printf("output file open error!!\n");
    return -1;
  }
  
  //log
  
  fprintf(fpo, "=====%s===iteration:%d=====\npoints:%d cluster:%d means/pointnum:", msg, it, datanum, clnum);
  for(k = 0; k < clnum; k++){
    fprintf(fpo, "(%d,%d)/%d ", means[k][0], means[k][1], pnum[k]);
    }
  fprintf(fpo, "\n");
  
  //output field
  
  for(i = 0; i < vsize; i++){
    for(j = 0; j < hsize; j++){
      if(field[i][j] == 0)
	fprintf(fpo, "-");
      else if(field[i][j] == -1)
	fprintf(fpo, "@");
      else
	fprintf(fpo, "%d", field[i][j]);
      //fprintf(fpo, " ");
    }
    fprintf(fpo, "\n");
  }
  
  fprintf(fpo, "\n\n");

  fclose(fpo);
  
  return 1;
}
