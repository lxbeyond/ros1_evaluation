#include "ros/ros.h"
#include "std_msgs/String.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>		// clock
#include <unistd.h>		// clock
#include <sys/mman.h>		// mlock
#include <sched.h>		// sched

#define EVAL_NUM 120

int i, count = -1, init_num_int;
double  subscribe_time[EVAL_NUM];

struct timespec tp1;		// for clock

FILE *fp;			// for file io

// eval_loop_count is updated when evaluation ends in each data size.
// // 0 means evaluation of 256 bytes, 1 means evaluation of 512 bytes, ... 
int eval_loop_count = 0;

std::string output_filename = "./evaluation/subscribe_time/subscribe_time_256byte.txt";

void chatterCallback(const std_msgs::String::ConstPtr& msg){

  if( count == -1 ){

	// Initialize count
	char init_num_char = *( msg->data.c_str());
	char *init_num_pt = &init_num_char;
	count = atoi(init_num_pt);
	init_num_int = count;	// if init_num_int is not 0, some messages are lost.

	// printf("first recieved number: %d \n\n", count);
	printf("message loss : %d \n", init_num_int);
   
  }
  
  // evaluation
  if( count < EVAL_NUM-1 && 0 <= count){

	if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
	  perror("clock_gettime begin");
	}
	subscribe_time[count] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;

	// printf("%18.9lf\n",subscribe_time[count]);
	// printf("subscribe_time[%2d]:\t%18.9lf\n", count,subscribe_time[count]);

	// printf("I heard: [%s]\n", receiver.data.c_str());
	// printf("I heard: [%c]\n",* ( msg->data.c_str()) );

	// printf("Time Span:\t%ld.%09ld\n", tp1.tv_sec, tp1.tv_nsec);

	count++;

  }else if( count == EVAL_NUM-1 ){
    
	if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
	  perror("clock_gettime begin");
	}
	subscribe_time[count] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;

	// 評価終了後にまとめてpublish_time[]をpublish_time.txtに出力
	if((fp = fopen(output_filename.c_str(), "w")) != NULL){
	
	  // init_numの書き込み
	  if(fprintf(fp, "%d\n",init_num_int ) < 0){
		//書き込みエラー
		printf("error : can't output subscribe_time.txt'");
	  }

	  // subscribe_time[]の書き込み
	  for(i=0; i<EVAL_NUM; i++){
		if(fprintf(fp, "%18.9lf\n", subscribe_time[i]) < 0){
		  //書き込みエラー
		  printf("error : can't output subscribe_time.txt'");
		  break;
		}
	  }

	  printf("output data\n");

	  fclose(fp);

	}else{
	  printf("error : can't output subscribe_time.txt'");
	}

	// 評価の初期化
	count = -1;					// initialize for next data size 
	eval_loop_count++;				// update for next data size
	
	if( eval_loop_count == 1){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_512byte.txt";
	}else if( eval_loop_count == 2){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_1Kbyte.txt";
	}else if( eval_loop_count == 3){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_2Kbyte.txt";
	}else if( eval_loop_count == 4){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_4Kbyte.txt";
	}else if( eval_loop_count == 5){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_8Kbyte.txt";
	}else if( eval_loop_count == 6){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_16Kbyte.txt";
	}else if( eval_loop_count == 7){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_32Kbyte.txt";
	}else if( eval_loop_count == 8){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_64Kbyte.txt";
	}else if( eval_loop_count == 9){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_128Kbyte.txt";
	}else if( eval_loop_count == 10){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_256Kbyte.txt";
	}else if( eval_loop_count == 11){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_512Kbyte.txt";
	}else if( eval_loop_count == 12){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_1Mbyte.txt";
	}else if( eval_loop_count == 13){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_2Mbyte.txt";
	}else if( eval_loop_count == 14){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_4Mbyte.txt";
	}else if( eval_loop_count == 15){
	  // 計測終了
	  count == EVAL_NUM;
	}
	
  }
}

int main(int argc, char **argv)
{
  mlockall(MCL_FUTURE);		// lock all cached memory into RAM and prevent future dynamic memory allocations
  
  usleep(1000);
  sched_param  pri = {94}; 
  if (sched_setscheduler(0, SCHED_FIFO, &pri) == -1) { // set FIFO scheduler
	perror("sched_setattr");
	exit(EXIT_FAILURE);
  }
  
  ros::init(argc, argv, "listener1");
  
  ros::NodeHandle n;

  ros::Subscriber sub = n.subscribe("chatter", 1000, +chatterCallback);
  
  printf("start evaluation\n");

  ros::spin();

  return 0;
}
