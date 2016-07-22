// Program Header Information ////////////////////////////////////////
/**
* @file Sim01.c
*
* @brief program for SIM01
* 
* @details Simulation of a simle, one-program OS simulator
*
* @version 1.00
*          C.S. Student (4 March 2016)
*          Initial development of SIM01
*
* @note None
*/
// Program Description/Support /////////////////////////////////////
/*
 This program simulate a simple, one-program OS simulator named Sim01,
 using three state (Enter/Start, Running, Exit). It will accept the 
 meta-data for one program with a potentially unlimited number of 
 meta-data operations, run it, and end the simulation.
*/
// Precompiler Directives //////////////////////////////////////////
//
/////  NONE
//
// Header Files ///////////////////////////////////////////////////
//
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <unistd.h>
   #include <time.h>
   #include <stdbool.h>
//
// Global Constant Definitions ////////////////////////////////////
//
   #define BILLION   1E9 
//
// Class Definitions //////////////////////////////////////////////
//
   struct meta
      {
       // struct that records meta data
       char component;
       char operation[10];
       int cyc_time;
      };

   struct pcb_table
      {
       // struct that records pcb information read from config file
       int processorCycleTime;
       int monitorCycleTime;
       int hardDriveCycleTime;
       int printerCycleTime;
       int keyboardCycleTime;
       char dataFile[15];
       char outputFile[15];
      };

   struct logLine
      {
       // struct that records log to print and write to file
       double time;
       char comment[40];
      };
//
// Free Function Prototypes ///////////////////////////////////////
//
   void readConfig( char* fileName, struct pcb_table* pcb );
   void dataInput( char* fileName, struct meta metaData[], int* numberOfMeta );
   void thread_create( struct meta metaData, struct pcb_table pcb );
   int calcTime( struct meta metaData, struct pcb_table pcb );
   double timeLap( struct timespec startTime, struct timespec endTime );
   void delay( int time );
   void recordLog( struct logLine* currentLog, double time, char memo[40] );
   void printLog( struct logLine currentLog );
   bool checkMeta( struct meta metaData, char comment[40] );
   bool checkLogEnd( char comment[40] );
   void outputToFile( struct logLine currentLog[], struct pcb_table pcb, 
                      int numberOfLog );
//
// Main Function Implementation ///////////////////////////////////
//
   int main( int argc, char* argv[] )
      {
       struct meta metaArray[40] = { 0, 0, 0 };   // meta data array
       struct pcb_table myPCB = { 0, 0, 0, 0, 0, 0, 0 };   // pcb table
       struct logLine myLog[100] = { 0, 0 };   // log array
       bool isThread = false;   // check if the meta a thread
       bool logNeedEnd = false;   // check if the log needs a end log
       int numThreads = 0;   // number of threads
       struct timespec startTime, endTime;   // timer
       double totalTime;   // time range
       int metaIndex = 0;   // index to meta data
       int logIndex = 0;   // index to log
       char logComment[40];   // comment inside log

       // read config file
       readConfig( argv[1], &myPCB );

       // read meta data file
       dataInput( myPCB.dataFile, metaArray, &numThreads );

       // start timer
       clock_gettime( CLOCK_REALTIME, &startTime );

       // loop through the meta data array
       for( metaIndex = 0; metaIndex < numThreads; metaIndex++ )
          {
           // lap timer
           clock_gettime( CLOCK_REALTIME, &endTime );
           totalTime = timeLap( startTime, endTime );

           // check if the meta is a thread
           if( isThread = checkMeta( metaArray[metaIndex], logComment ) )
              {
               // if it is, create a thread
               thread_create( metaArray[metaIndex], myPCB );
              }

           // record this log and print it out
           recordLog( &myLog[logIndex], totalTime, logComment );
           printLog( myLog[logIndex] );
           logIndex ++;

           // lap timer
           clock_gettime( CLOCK_REALTIME, &endTime );
           totalTime = timeLap( startTime, endTime );

           // check if the log needs a end log
           if( logNeedEnd = checkLogEnd(logComment ) )
              {
               // if it needs, record and print
               recordLog( &myLog[logIndex], totalTime, logComment );
               printLog( myLog[logIndex] );
               logIndex ++;
              }
           }   // end of loop

       // output the logs to file
       outputToFile(myLog,myPCB, logIndex);

       return 0;
      }   // end of main

//
// Free Function Implementation ///////////////////////////////////
/**
* @brief Function reads config file 
*
* @details Function reads config file and record to pcb table
*
* @pre char* fileName contains the name of config file
*
* @pre struct* pcb contains the pcb table
*
* @post if the config file doesn't exist, end the program
*
* @post all pcb information recorded in pcb table
*
* @return None
*
*/
   void readConfig( char* fileName, struct pcb_table* pcb )
      {
       FILE* filePtr;   // file pointer
       char line[40];   // string holds each line of file
       int charIndex = 0;   // index of string
       int stringIndex = 0;   // index to pcb string

       // open file and read
       filePtr = fopen( fileName, "r" );

       // if the file doesn't exist
       if( filePtr == NULL )
          {
           printf( "CONFIGURATION FILE NOT FOUND!\n" );
           exit(1);
          }

       // otherwise
       else
          {
           // loop to each line
           while( fgets( line, sizeof(line), filePtr ) )
              {
               stringIndex =0;

               // ingore these lines
               if( strncmp( line, "Start Simulator Configuration File", 15 ) == 0 )
                  continue;
               if( strncmp( line, "Version/Phase: 1.0", 15 ) == 0 )
                  continue;
               if( strncmp( line, "Log: Log to Both", 15 ) == 0 )
                  continue;
               if( strncmp( line, "End Simulator Configuration File", 15 ) == 0 )
                  continue;

               // read and record meta file name
               if( strncmp( line, "File Path: ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                      {
                       if(line[charIndex] == ':') 
                          {
                           do
                              {
                               charIndex ++;
                              } 
                           while( line[charIndex] == ' ' );
                           while( line[charIndex] != ' ' && line[charIndex] != '\n' )
                              {
                               pcb -> dataFile[stringIndex] = line[charIndex];
                               charIndex++;
                               stringIndex++;
                              }
                           pcb -> dataFile[stringIndex] = '\0';
                          }
                      }
                  }

               // read and record processor cycle time
               if( strncmp( line, "Processor cycle time (msec): ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                      {
                       if( line[charIndex] == ':' ) 
                          {
                           do
                              {
                               charIndex ++;
                              }
                           while( line[charIndex] == ' ' );
                           while( line[charIndex] != ' ' && line[charIndex] != '\n' )
                              {
                               pcb -> processorCycleTime = pcb -> processorCycleTime * 10 
                                      + line[charIndex] - '0';
                               charIndex ++;
                              }
                          }
                      }
                  }

               // read and record monitor cycle time
               if( strncmp( line, "Monitor display time (msec): ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                      {
                       if( line[charIndex] == ':' )
                          {
                           do
                              {
                               charIndex ++;
                              }
                           while( line[charIndex] == ' ' );
                           while( line[charIndex] != ' ' && line[charIndex] != '\n')
                              {
                               pcb -> monitorCycleTime = pcb -> monitorCycleTime * 10 
                                      + line[charIndex] - '0';
                               charIndex ++;
                              }
                          }
                      }
                  }

               // read and record hard drive cycle time
               if( strncmp( line, "Hard drive cycle time (msec): ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                      {
                       if( line[charIndex] == ':' ) 
                          {
                           do
                              {
                               charIndex++;
                              }
                           while( line[charIndex] == ' ' );
                           while( line[charIndex] != ' ' && line[charIndex] != '\n' )
                              {
                               pcb -> hardDriveCycleTime = pcb -> hardDriveCycleTime * 10 
                                      + line[charIndex] - '0';
                               charIndex ++;
                              }
                          }
                      }
                  }

               // read and record printer cycle time
               if( strncmp( line, "Printer cycle time (msec): ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                      {
                       if( line[charIndex] == ':' ) 
                          {
                           do
                              {
                               charIndex ++;
                              }
                           while( line[charIndex] == ' ');
                           while( line[charIndex] != ' ' && line[charIndex] != '\n' )
                              {
                               pcb -> printerCycleTime = pcb -> printerCycleTime * 10 
                                      + line[charIndex] - '0';
                               charIndex ++;
                              }
                          }
                      }
                  }

               // read and record keyboard cycle time
               if( strncmp( line, "Keyboard cycle time (msec): ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex++ )
                      {
                       if( line[charIndex] == ':' )
                          {
                           do
                              {
                               charIndex++;
                              }
                           while( line[charIndex] == ' ' );
                           while( line[charIndex] != ' ' && line[charIndex] != '\n' )
                              {
                               pcb -> keyboardCycleTime = pcb -> keyboardCycleTime * 10 
                                      + line[charIndex] - '0';
                               charIndex ++;
                              }
                          }
                      }
                  }

               // read and record output file name
               if( strncmp( line, "Log File Path: ", 10 ) == 0 )
                  {
                   for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                      {
                       if( line[charIndex] == ':' ) 
                          {
                           do
                              {
                               charIndex ++;
                              }
                           while( line[charIndex] == ' ' );
                           while( line[charIndex] != ' ' && line[charIndex] != '\n' )
                              {
                               pcb -> outputFile[stringIndex] = line[charIndex];
                               charIndex ++;
                               stringIndex ++;
                              }
                           pcb -> outputFile[stringIndex] = '\0';
                          }
                      }
                  }
              }   // end of loop
          }

      // close file
      fclose(filePtr);
      }   // end of func

/**
* @brief Function reads meta data file 
*
* @details Function reads meta data and record
*
* @pre char* fileName contains the name of data file
*
* @pre struct* metaData contains the meta data
*
* @pre int* numberOfMeta contains the number of meta data
*
* @post if the data file doesn't exist, end the program
*
* @post all meta data information recorded
*
* @return None
*
*/
   void dataInput( char* fileName, struct meta metaData[], int* numberOfMeta )
      {
       FILE* filePtr;   // file pointer
       char line[100];   // string holds each line of file
       int charIndex;   // index to string
       int stringIndex = 0;   // index to meta string

       // open file
       filePtr = fopen( fileName, "r" );

       // if the file doesn't exist
       if( filePtr == NULL )
          {
           printf( "META DATA FILE NOT FOUND!\n" );
           exit( 1 );
          }

       // otherwise
       else
          {
           // loop to get each line
           while( fgets( line, sizeof( line ), filePtr ) )
              {
               // ignore these lines
               if( strncmp( line, "Start Program Meta-Data Code:", 15 ) == 0 ) 
                  continue;
               if( strncmp( line, "End Program Meta-Data Code.", 15 ) == 0 ) 
                  continue;

               // loop to record each meta data
               for( charIndex = 0; charIndex < sizeof( line ); charIndex ++ )
                  {
                   while( line[charIndex] == ' ' )
                   charIndex ++;
                   if( line[charIndex] == '\n' )
                      break;
                   metaData[stringIndex].component = line[charIndex]; 
                   charIndex ++;
                   if( line[charIndex] == '(' ) 
                      charIndex ++;
                      int k;
                   for( k=0; line[charIndex] != ')' ; charIndex ++, k ++ )
                      metaData[stringIndex].operation[k] = line[charIndex];
                   metaData[stringIndex].operation[k] = '\0';
                   charIndex ++;
                   for( ; line[charIndex] != ';' && line[charIndex] != '.'; charIndex ++)
                   metaData[stringIndex].cyc_time = metaData[stringIndex].cyc_time * 10 
                                                    + line[charIndex] - '0';
                   if( line[charIndex] == ';' )
                      stringIndex ++;
                  }   // end of loop
              }
           *numberOfMeta = stringIndex + 1;   // number of meta incr.
          }   // end of loop

       // close file
       fclose(filePtr);
      }   // end of func

/**
* @brief Function creates a thread
*
* @details Function creates a thread according its cycle time
*
* @pre struct metaData contains the meta data
*
* @pre struct pcb contains the pcb table
*
* @post thread run for its time
*
* @return None
*
*/
   void thread_create( struct meta metaData, struct pcb_table pcb )
      {
       int time = 0;   // init. time

       // calculate the time to run
       time = calcTime(metaData, pcb);

       // delay that long time
       delay(time);
      }   // end of func

/**
* @brief Function checks if the meta is to create a thread
*
* @details Function checks if the meta is to create a thread
*          and write the corresponding log comment
*
* @pre struct metaData contains the meta data
*
* @pre char comment contains the comment of its log
*
* @post the comment that needs to print and output
*
* @return ture if the meta is to create a thread
*
* @return false if the meta is not to create a thread
*
*/
   bool checkMeta( struct meta metaData, char comment[40] )
      {
       char temp[40] = { 0 };   // temp string

       // decide the comment by the component of meta
       if( metaData.component == 'S' )
          {
           if( strcmp( metaData.operation, "start" ) == 0 )
              strcpy( temp, "Simulator program starting" );
           if( strcmp( metaData.operation, "end" ) == 0 )
              strcpy( temp, "Simulator program ending" );
          }
       if( metaData.component == 'A' )
          {
           if( strcmp( metaData.operation, "start" ) == 0)
              strcpy( temp, "OS: preparing process 1" );
           if( strcmp( metaData.operation, "end" ) == 0 )
              strcpy( temp, "OS: removing process 1" );
          }
       if( metaData.component == 'P' || metaData.component == 'I' 
           || metaData.component == 'O' )
          {
           if( metaData.component == 'P' )
              strcpy( temp, "Process 1: start processing action" );
           else
              {
               strcpy( temp, "Process 1: start " );
               strcat( temp, metaData.operation );
               if( metaData.component == 'I' )
                  strcat( temp, " input" );
               if( metaData.component == 'O' )
                  strcat( temp, " ouput" );
              }
          }

       // copy the comment
       strcpy( comment, temp );

       // if the metaData is to create a thread, return true
       // otherwise, return false
       if( metaData.component == 'P' || metaData.component == 'I' 
           || metaData.component == 'O' )
          return true;
       else
          return false;
      }   // end of func

/**
* @brief Function checks if the log needs log end
*
* @details Function checks if the log needs log end
*
* @pre char comment contains the comment of the log
*
* @post if the log doesn't need an end, leave it blank
*
* @post if it needs, gives it a log end
*
* @return true if the log needs a log end
*
*/
   bool checkLogEnd( char comment[40] )
      {
       char* stringIndex;   // index to string
       int newLength = 0;   // length of new string

       // for the log doesn't need an end
       if( strcmp( comment, "Simulator program starting" ) == 0 
           || strcmp( comment, "Simulator program ending" ) == 0 
           || strcmp( comment, "OS: removing process 1" ) == 0 )
          return false;

       // make the end for the log needs
       else if( strcmp( comment, "OS: preparing process 1" ) == 0 )
          strcpy( comment, "OS: starting process 1" );
       else
          {
           stringIndex = strstr( comment, "start" );
           newLength = strlen( comment )-strlen( "start" )+strlen( "end" );
           char temp[newLength+1];
           memcpy( temp, comment, stringIndex - comment );
           memcpy( temp + ( stringIndex - comment ), "end", strlen( "end" ) );
           strcpy( temp + ( stringIndex - comment) + strlen( "end" ), 
                   stringIndex + strlen( "start" ) );
           strcpy( comment, temp );
          }
       return true;
      }   // end of func

/**
* @brief Function delays some time
*
* @details Function delays according to the time
*
* @pre int time contains the time to delay
*
* @post delays for a certain time
*
* @return None
*
*/
   void delay( int time )
      {
       int index_1, index_2;   // indexes

       // loop to create delay
       for( index_1 = 0; index_1 < time; index_1 ++ )
          {
           for( index_2 = 0; index_2 < 400000; index_2 ++ )
              {
               // init. a volatile valuable avoid optimal
               int volatile temp;
               temp = 120 * index_1 * index_2;
               temp = temp + 5;
              }
          }   // end of loop
      }   // end of func

/**
* @brief Function calculates the time
*
* @details Function calculates the time according to cycle time
*
* @pre struct metaData contains meta data
*
* @pre struct pcb contains the pcb table
*
* @post the time that needs to delay created
*
* @return int time that the delay time
*
*/
   int calcTime( struct meta metaData, struct pcb_table pcb )
      {
       int time = 0;   // init. time
       char temp[10] = { 0 };   // string temp
       int numberOfCycle = 0;   // number of cycles
       strcpy( temp, metaData.operation );
       numberOfCycle = metaData.cyc_time;

       // calc the time needs to delay
       if( strcmp( temp, "start" ) == 0 || strcmp( temp, "run" ) == 0 
           || strcmp( temp, "end" ) == 0 )
          time = pcb.processorCycleTime * numberOfCycle;
       if( strcmp( temp, "hard drive" ) == 0 )
          time = pcb.hardDriveCycleTime * numberOfCycle;
       if( strcmp( temp, "keyboard" ) == 0 )
          time = pcb.keyboardCycleTime * numberOfCycle;
       if( strcmp( temp, "monitor" ) == 0 )
          time = pcb.monitorCycleTime * numberOfCycle;
       if( strcmp( temp, "printer" ) == 0 )
          time = pcb.printerCycleTime * numberOfCycle;

       return time;
      }   // end of func

/**
* @brief Function gets the time range it elapses
*
* @details Function returns the real time range
*
* @pre struct startTime contains start time point
*
* @pre struct endTime contains end time point
*
* @post time range calculated and returned
*
* @return double the time range
*
*/
   double timeLap( struct timespec startTime, struct timespec endTime )
      {
       return ( endTime.tv_sec - startTime.tv_sec ) + 
              ( endTime.tv_nsec - startTime.tv_nsec ) / BILLION;
      }   // end of func

/**
* @brief Function records log 
*
* @details Function records log's time and comment
*
* @pre char* memo contains the comment
*
* @pre double contains the time
*
* @pre struct* currentLog contains the log
*
* @post records the log
*
* @return None
*
*/
   void recordLog( struct logLine* currentLog, double time, char memo[40] )
      {
       if( time != 0 )
          currentLog -> time = time;
       if( memo != 0 )
          strcpy( currentLog -> comment, memo );
      }   // end of func

/**
* @brief Function prints each log 
*
* @details Function prints each log to screen
*
* @pre struct currentLog contains the log
*
* @post the log printed on screen
*
* @return None
*
*/
   void printLog( struct logLine currentLog )
      {
       printf( "%f - %s\n", currentLog.time, currentLog.comment );
      }   // end of func

/**
* @brief Function output to file
*
* @details Function output logs to file
*
* @pre struct currentLog contains the log
*
* @pre int numberOfLog contains the number of logs
*
* @pre struct pcb contains the pcb table
*
* @post output logs to file
*
* @return None
*
*/
   void outputToFile( struct logLine currentLog[], struct pcb_table pcb, 
                      int numberOfLog )
      {
       FILE *filePtr;   // file pointer
       int index =0;   // index

       // open file and write
       filePtr = fopen( pcb.outputFile, "w" );

       // output each log
       while( index < numberOfLog )
          {
           fprintf( filePtr, "%f", currentLog[index].time );
           fputs( " - ", filePtr );
           fprintf( filePtr, "%s", currentLog[index].comment );
           fputs( "\n", filePtr );
           index ++;
          }

       // close file
       fclose( filePtr );
      }   // end of func



